namespace pg {
    struct original_bytes_t {
        std::uint8_t m_ki_expand_kernel_stack[ 6 ];
        std::uint8_t m_fs_rtl_mdl_read_complete[ 1 ];
        std::uint8_t m_ki_sw_interrupt_dispatch[ 3 ];
        std::uint8_t m_exp_license_watch_init[ 3 ];
        std::uint8_t m_fs_rtl_uninitialize_small_mcb[ 3 ];
        std::uint8_t m_ki_filter_fiber_context[ 3 ];
		bool m_initialized{ false };
    } m_original_code;

    auto m_patch_code = [ ] ( std::uint64_t target_va, const std::uint8_t* bytes, std::size_t size ) -> bool {
        std::uint64_t target_pa = 0;
        if ( !paging::translate_linear( target_va, &target_pa ) ) {
            return false;
        }

        if ( !paging::dpm::write_physical( target_pa, const_cast< std::uint8_t* >( bytes ), size ) ) {
            return false;
        }

        __invlpg( reinterpret_cast< void* >( target_va ) );
        return true;
        };

    bool capture_instructions( std::uint64_t target_va, std::uint8_t* buffer, std::size_t size ) {
        std::uint64_t target_pa = 0;
        if ( !paging::translate_linear( target_va, &target_pa ) ) {
            return false;
        }

        if ( !paging::dpm::read_physical( target_pa, buffer, size ) ) {
            return false;
        }

        return true;
    }

    bool disable_integrity( ) {
        nt::dbg_print( oxorany( "[PG] patching integrity routines\n" ) );

        auto ps_integrity_enabled = reinterpret_cast< int* >( nt::m_pdb_symbols.m_ps_integrity_check_enabled );
        if ( ps_integrity_enabled )
            *ps_integrity_enabled = 0;

        std::uint8_t ki_sw_interrupt_dispatch[ ] = { 0x8B, 0x83 };
        std::uint8_t fiber_callout[ ] = { 0x8B, 0x87 };
        std::uint8_t fiber_callback[ ] = { 0x41, 0x8B, 0x81 };

        auto search_start = module::get_module_base( oxorany( L"ntoskrnl.exe" ) );
        auto module_size = module::get_module_size( oxorany( L"ntoskrnl.exe" ) );

        std::uint64_t current_offset = 0;
        while ( current_offset < module_size ) {
            auto pattern = nt::find_ida_pattern( 
                search_start + current_offset,
                module_size - current_offset,
                oxorany( "0F 22 C0" ) );
            if ( !pattern ) break;

            std::uint8_t instruction_bytes[ 16 ] = { 0 };
            if ( !capture_instructions( pattern, instruction_bytes, sizeof( instruction_bytes ) ) ) {
                current_offset = ( pattern - search_start ) + 3;
                continue;
            }

            hde64s hde = { 0 };
            hde64_disasm( instruction_bytes, &hde );
            auto next_instruction = &instruction_bytes[ hde.len ];

            bool is_target_instruction = false;
            const char* instruction_name = nullptr;
            if ( !memcmp( next_instruction, ki_sw_interrupt_dispatch, sizeof( ki_sw_interrupt_dispatch ) ) ) {
                is_target_instruction = true;
                instruction_name = oxorany( "ki_sw_interrupt_dispatch" );
            }

            if ( !memcmp( next_instruction, fiber_callback, sizeof( fiber_callback ) ) ) {
                is_target_instruction = true;
                instruction_name = oxorany( "fiber_callback" );
            }

            if ( !memcmp( next_instruction, fiber_callout, sizeof( fiber_callout ) ) ) {
                is_target_instruction = true;
                instruction_name = oxorany( "fiber_callout" );
            }

            if ( is_target_instruction ) {
                nt::dbg_print( oxorany( "[PG] found routine: %s at 0x%llx\n" ), instruction_name, pattern );

                std::uint8_t patch_zero[ ] = { 0x33, 0xC0, 0xC3 };
                if ( !m_patch_code( pattern, patch_zero, sizeof( patch_zero ) ) ) {
                    nt::dbg_print( oxorany( "[PG] could not patch at 0x%llx\n" ), pattern );
                }
                else {
                    nt::dbg_print( oxorany( "[PG] successfully patched %s at 0x%llx\n" ), instruction_name, pattern );
                }
            }

            current_offset = ( pattern - search_start ) + 3;
        }

        auto fs_rtl_uninitialize_small_mcb = nt::m_pdb_symbols.m_fs_rtl_uninitialize_small_mcb;
        if ( !fs_rtl_uninitialize_small_mcb )
            return false;

        std::uint8_t patch_ret[ ] = { 0xC3 };
        if ( !m_patch_code( fs_rtl_uninitialize_small_mcb, patch_ret, sizeof( patch_ret ) ) ) {
            nt::dbg_print( oxorany( "[PG] could not patch mcb routine at 0x%llx\n" ), fs_rtl_uninitialize_small_mcb );
            return false;
        }

        auto ki_filter_fiber_context = nt::m_pdb_symbols.m_ki_filter_fiber_context;
        if ( !ki_filter_fiber_context )
            return false;

        std::uint8_t patch_zero[ ] = { 0x33, 0xC0, 0xC3 };
        if ( !m_patch_code( ki_filter_fiber_context, patch_zero, sizeof( patch_zero ) ) ) {
            nt::dbg_print( oxorany( "[PG] could not patch filter fiber at 0x%llx\n" ), ki_filter_fiber_context );
            return false;
        }

        auto fs_rtl_mdl_read_complete_dev_ex = nt::m_pdb_symbols.m_fs_rtl_mdl_read_complete_dev_ex;
        if ( !fs_rtl_mdl_read_complete_dev_ex )
            return false;

        if ( !m_patch_code( fs_rtl_mdl_read_complete_dev_ex, patch_zero, sizeof( patch_zero ) ) ) {
            nt::dbg_print( oxorany( "[PG] could not patch mdl routine at 0x%llx\n" ), fs_rtl_mdl_read_complete_dev_ex );
            return false;
        }

        nt::dbg_print( oxorany( "[PG] patched integrity routines\n" ) );
        return true;
    }
}