#pragma once

namespace kep {
    struct kep_entry_t {
        char          m_function_name[ 64 ];
        std::uint64_t m_original_address;
        std::uint64_t m_target_address;
        std::uint64_t m_hook_handler;
        bool          m_active;
    };

    constexpr std::uint32_t max_entries = 512;
    kep_entry_t m_kep_entries[ max_entries ] = {};
    long volatile m_kep_in_progress = 0;

    constexpr std::uint8_t kep_shellcode[ ]{
       0x50,
       0x48, 0xB8, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00,
       0x48, 0x87, 0x04, 0x24,
       0xC3
    };

    std::int32_t find_entry( const char* function_name ) {
        for ( auto idx = 0; idx < max_entries; idx++ ) {
            if ( m_kep_entries[ idx ].m_active &&
                 std::strcmp( m_kep_entries[ idx ].m_function_name, function_name ) == 0 )
                return idx;
        }
        return -1;
    }

    std::int32_t get_next_slot( ) {
        for ( auto idx = 0; idx < max_entries; idx++ ) {
            if ( !m_kep_entries[ idx ].m_active )
                return idx;
        }
        return -1;
    }

    std::int32_t find_entry( void* hook_handler ) {
        for ( auto idx = 0; idx < max_entries; idx++ ) {
            if ( m_kep_entries[ idx ].m_active &&
                 m_kep_entries[ idx ].m_hook_handler == reinterpret_cast< std::uint64_t >( hook_handler ) )
                return idx;
        }
        return -1;
    }

    std::uint64_t find_unused_target( ) {
        for ( auto idx = 0; idx < max_entries; idx++ ) {
            if ( !m_kep_entries[ idx ].m_active &&
                 m_kep_entries[ idx ].m_target_address ) {
                return m_kep_entries[ idx ].m_target_address;
            }
        }
        return 0;
    }

    bool is_hooks_active( ) {
        for ( auto idx = 0; idx < max_entries; idx++ ) {
            if ( m_kep_entries[ idx ].m_active ) {
                return true;
            }
        }
        return false;
    }

    void acquire_lock( ) {
        _mm_mfence( );
        while ( _InterlockedCompareExchange( &m_kep_in_progress, 1, 0 ) ) {
            _mm_pause( );
        }
        _mm_mfence( );
    }

    void release_lock( ) {
        _mm_mfence( );
        _InterlockedExchange( &m_kep_in_progress, 0 );
        _mm_pause( );
    }

    bool mdl_write( void* target, const void* src, size_t size ) {
        auto mdl = nt::io_allocate_mdl( target, size, false, false, nullptr );
        if ( !mdl )
            return false;

        __try {
            nt::mm_probe_and_lock_pages( mdl, 0, 1 ); // KernelMode, IoReadAccess
        }
        __except ( EXCEPTION_EXECUTE_HANDLER ) {
            nt::io_free_mdl( mdl );
            return false;
        }

        auto mapped = nt::mm_map_locked_pages_specify_cache(
            mdl, 0, 4, nullptr, false, 16 ); // KernelMode, MmNonCached, NormalPagePriority

        if ( !mapped ) {
            nt::mm_unlock_pages( mdl );
            nt::io_free_mdl( mdl );
            return false;
        }

        auto status = nt::mm_protect_mdl_system_address( mdl, 0x04 ); // PAGE_READWRITE
        if ( status != nt_status_t::success ) {
            nt::mm_unmap_locked_pages( mapped, mdl );
            nt::mm_unlock_pages( mdl );
            nt::io_free_mdl( mdl );
            return false;
        }

        std::memcpy( mapped, src, size );
        _mm_mfence( );

        nt::mm_unmap_locked_pages( mapped, mdl );
        nt::mm_unlock_pages( mdl );
        nt::io_free_mdl( mdl );
        return true;
    }

    template<typename org_fn>
    bool create_hook(
        std::uint64_t module_base,
        const char* function_name,
        void* hook_handler,
        org_fn* original_fn = nullptr
    ) {
        acquire_lock( );

        const auto kep_entry = find_entry( hook_handler );
        if ( kep_entry != -1 ) {
            nt::dbg_print(
                oxorany( "[KEP] create_hook fail: already active handler=0x%llx slot=%d\n" ),
                reinterpret_cast< std::uint64_t >( hook_handler ), kep_entry );
            release_lock( );
            return false;
        }

        const auto dos_header = reinterpret_cast< dos_header_t* >( module_base );
        const auto nt_headers = reinterpret_cast< nt_headers_t* >( module_base + dos_header->m_lfanew );
        if ( !dos_header->is_valid( ) || !nt_headers->is_valid( ) ) {
            nt::dbg_print(
                oxorany( "[KEP] create_hook fail: bad headers fn=%s module_base=0x%llx\n" ),
                function_name, module_base );
            release_lock( );
            return false;
        }

        const auto exp_dir = nt_headers->m_export_table.as_rva<export_directory_t*>( module_base );
        if ( !exp_dir || !exp_dir->m_number_of_names ) {
            nt::dbg_print(
                oxorany( "[KEP] create_hook fail: no exports fn=%s module_base=0x%llx\n" ),
                function_name, module_base );
            release_lock( );
            return false;
        }

        auto functions = reinterpret_cast< std::uint32_t* >( module_base + exp_dir->m_address_of_functions );
        auto names = reinterpret_cast< std::uint32_t* >( module_base + exp_dir->m_address_of_names );
        auto ordinals = reinterpret_cast< std::uint16_t* >( module_base + exp_dir->m_address_of_names_ordinals );

        for ( auto idx = 0; idx < exp_dir->m_number_of_names; ++idx ) {
            const auto export_name = reinterpret_cast< const char* >( module_base + names[ idx ] );
            if ( std::strcmp( function_name, export_name ) )
                continue;

            const auto ordinal = ordinals[ idx ];
            const auto function_rva = functions[ ordinal ];
            const auto function_addr = module_base + function_rva;

            auto target_address = find_unused_target( );
            if ( !target_address ) {
                target_address = module::copy_on_allocation( module_base, sizeof( kep_shellcode ) );
                if ( !target_address ) {
                    nt::dbg_print(
                        oxorany( "[KEP] create_hook fail: no cave fn=%s module_base=0x%llx\n" ),
                        function_name, module_base );
                    release_lock( );
                    return false;
                }
            }/*

            std::uint64_t section_size;
            std::uint64_t section_base;
            if ( !nt::get_section( oxorany( ".data" ), &section_base, &section_size ) ) {
                nt::dbg_print(
                    oxorany( "[KEP] create_hook fail: no section fn=%s module_base=0x%llx\n" ),
                    function_name, module_base );
                release_lock( );
                return false;
            }

            auto target_address = mmu::find_unused_space(
                reinterpret_cast< void* >( section_base ),
                section_size,
                sizeof( kep_shellcode ) );
            if ( !target_address ) {
                nt::dbg_print(
                    oxorany( "[KEP] create_hook fail: no cave fn=%s module_base=0x%llx\n" ),
                    function_name, module_base );
                release_lock( );
                return false;
            }

            std::uint64_t target_module;
            if ( !module::is_inside_module( target_address, &target_module ) || target_module != nt::m_module_base ) {
                nt::dbg_print(
                    oxorany( "[KEP] create_hook fail: outside of module bounds hook=0x%llx module_base=0x%llx\n" ),
                    target_address, target_module );
                release_lock( );
                return false;
            }

            if ( !paging::spoof_pte_range( target_address, sizeof( kep_shellcode ) ) ) {
                return false;
            }*/

            if ( !nt::mm_is_address_valid( reinterpret_cast< void* >( target_address ) ) ) {
                nt::dbg_print(
                    oxorany( "[KEP] create_hook fail: invalid cave fn=%s target=0x%llx\n" ),
                    function_name, target_address );
                release_lock( );
                return false;
            }

            nt::dbg_print(
                oxorany( "[KEP] selected target: target=0x%llx module_base=0x%llx function_addr=0x%llx\n" ),
                target_address, module_base, function_addr );

            if ( original_fn )
                *original_fn = reinterpret_cast< org_fn >( function_addr );

            std::uint8_t shellcode[ sizeof( kep_shellcode ) ];
            std::memcpy( shellcode, kep_shellcode, sizeof( shellcode ) );
            *reinterpret_cast< std::uint64_t* >( shellcode + 3 ) =
                reinterpret_cast< std::uint64_t >( hook_handler );

            std::memcpy(
                reinterpret_cast< void* >( target_address ),
                shellcode,
                sizeof( shellcode )
            );

            auto hook_rva = static_cast< std::uint32_t >( target_address - module_base );

            _mm_mfence( );
            auto new_rva = static_cast< uint32_t >( target_address - module_base );
            if ( !mdl_write( &functions[ ordinal ], &new_rva, sizeof( new_rva ) ) ) {
                nt::dbg_print( "[KEP] create_hook fail: mdl write failed fn=%s\n", function_name );
                release_lock( );
                return false;
            }

            _mm_mfence( );

            auto next_entry = get_next_slot( );
            strncpy( m_kep_entries[ next_entry ].m_function_name, function_name, sizeof( m_kep_entries[ next_entry ].m_function_name ) - 1 );
            m_kep_entries[ next_entry ].m_function_name[ sizeof( m_kep_entries[ next_entry ].m_function_name ) - 1 ] = '\0';
            m_kep_entries[ next_entry ].m_original_address = function_addr;
            m_kep_entries[ next_entry ].m_target_address = target_address;
            m_kep_entries[ next_entry ].m_hook_handler = reinterpret_cast< std::uint64_t >( hook_handler );
            m_kep_entries[ next_entry ].m_active = true;

            nt::dbg_print(
                oxorany( "[KEP] hook installed: hook_rva=0x%x target=0x%llx orig=0x%llx slot=%d\n" ),
                hook_rva, target_address, function_addr, next_entry );

            release_lock( );
            return true;
        }

        release_lock( );
        return false;
    }

    bool remove_hook(
        std::uint64_t module_base,
        const char* function_name
    ) {
        acquire_lock( );

        const auto kep_index = find_entry( function_name );
        if ( kep_index == -1 ) {
            nt::dbg_print( oxorany( "[KEP] remove_hook fail: not found fn=%s\n" ), function_name );
            release_lock( );
            return false;
        }

        const auto dos_header = reinterpret_cast< dos_header_t* >( module_base );
        const auto nt_headers = reinterpret_cast< nt_headers_t* >( module_base + dos_header->m_lfanew );
        if ( !dos_header->is_valid( ) || !nt_headers->is_valid( ) ) {
            nt::dbg_print( oxorany( "[KEP] remove_hook fail: bad headers fn=%s\n" ), function_name );
            release_lock( );
            return false;
        }

        const auto exp_dir = nt_headers->m_export_table.as_rva<export_directory_t*>( module_base );
        if ( !exp_dir || exp_dir->m_number_of_names == 0 ) {
            nt::dbg_print( oxorany( "[KEP] remove_hook fail: no exports fn=%s\n" ), function_name );
            release_lock( );
            return false;
        }

        auto functions = reinterpret_cast< std::uint32_t* >( module_base + exp_dir->m_address_of_functions );
        auto names = reinterpret_cast< std::uint32_t* >( module_base + exp_dir->m_address_of_names );
        auto ordinals = reinterpret_cast< std::uint16_t* >( module_base + exp_dir->m_address_of_names_ordinals );

        auto& kep_entry = m_kep_entries[ kep_index ];
        const auto original_fn = kep_entry.m_original_address;
        const auto original_rva = static_cast< std::uint32_t >( original_fn - module_base );

        for ( auto idx = 0; idx < exp_dir->m_number_of_names; ++idx ) {
            const auto export_name = reinterpret_cast< const char* >( module_base + names[ idx ] );
            if ( std::strcmp( function_name, export_name ) )
                continue;

            const auto ordinal = ordinals[ idx ];

            _mm_mfence( );
            _InterlockedExchange(
                reinterpret_cast< volatile long* >( &functions[ ordinal ] ),
                static_cast< long >( original_rva )
            );
            _mm_mfence( );

            nt::dbg_print( oxorany( "[KEP] hook removed: 0x%llx slot=%d\n" ),
                           kep_entry.m_hook_handler, kep_index );

            kep_entry.m_active = false;
            kep_entry.m_original_address = 0;
            kep_entry.m_hook_handler = 0;
            kep_entry.m_function_name[ 0 ] = '\0';

            release_lock( );
            return true;
        }

        nt::dbg_print( oxorany( "[KEP] remove_hook fail: function not found fn=%s\n" ), function_name );
        release_lock( );
        return false;
    }
}