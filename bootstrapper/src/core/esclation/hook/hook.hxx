#pragma once

namespace hook {
    class c_hook {
    public:
        bool setup( ) {
            auto ke_query_auxiliary_counter_frequency = g_ntoskrnl->get_export( oxorany( "KeQueryAuxiliaryCounterFrequency" ) );
            if ( !ke_query_auxiliary_counter_frequency )
                return false;

            this->m_hook_address = g_ntoskrnl->rva( ke_query_auxiliary_counter_frequency + 4, 3, 7 );
            if ( !m_hook_address )
                return false;

            auto hook_physical = g_paging->translate_linear( m_hook_address );
            if ( !hook_physical )
                return false;

            if ( !g_driver->read_physical_memory( hook_physical, &m_original_fn, 8 ) )
                return false;

            auto callback_door = std::make_unique< compiler::shellcode_t >( );
            if ( !g_compiler->compile_callback( callback_door.get( ) ) ) {
                logging::print( oxorany( "[c_hook] create: Could not compile shellcode" ) );
                return false;
            }

            this->m_shellcode_size = callback_door->m_size;
            this->m_compiled_shellcode = callback_door->m_byte_code;
            if ( m_compiled_shellcode.empty( ) )
                return false;

            logging::print( oxorany( "[c_hook] create: Compiled shellcode" ) );

            auto text_section = std::make_unique< section::c_section >( g_ntoskrnl );
            auto allocation_base = text_section->find_unused_space( m_shellcode_size );
            if ( !allocation_base ) {
                return false;
            }

            logging::print( oxorany( "[c_hook] create: Allocated shellcode at 0x%llx" ), allocation_base );

            auto allocation_pa = g_paging->translate_linear( allocation_base );
            if ( !allocation_pa )
                return false;

            this->m_mapped_code = g_driver->map_physical_memory( allocation_pa, m_shellcode_size );
            if ( !m_mapped_code )
                return false;

            logging::print( oxorany( "[c_hook] create: Mapped shellcode at: 0x%llx\n" ), m_mapped_code );

            if ( !g_driver->write_physical_memory( hook_physical, &allocation_base, 8 ) ) {
                logging::print( oxorany( "[c_hook] create: Could not write executable code" ) );
                return false;
            }

            this->m_original_code.resize( m_shellcode_size );
            return true;
        }

        bool unhook( ) const {
            rw::write_virt( m_hook_address, m_original_fn );
            return true;
        }

        template<typename ret_t = std::uint64_t, typename fn_t, typename a1_t = void*, typename a2_t = void*, typename a3_t = void*, typename a4_t = void*, typename a5_t = void*, typename a6_t = void*, typename a7_t = void*, typename a8_t = void*, typename a9_t = void*, typename a10_t = void*>
        ret_t call_kernel( fn_t func_ptr, a1_t arg1 = a1_t{}, a2_t arg2 = a2_t{}, a3_t arg3 = a3_t{}, a4_t arg4 = a4_t{}, a5_t arg5 = a5_t{}, a6_t arg6 = a6_t{}, a7_t arg7 = a7_t{}, a8_t arg8 = a8_t{}, a9_t arg9 = a9_t{}, a10_t arg10 = a10_t{} ) {
            struct call_struct_t { void* func; a1_t arg1; a2_t arg2; a3_t arg3; a4_t arg4; a5_t arg5; a6_t arg6; a7_t arg7; a8_t arg8; a9_t arg9; a10_t arg10; };
            call_struct_t call_context{ reinterpret_cast< void* >( func_ptr ), arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 };

            if ( !m_mapped_code || m_compiled_shellcode.empty( ) ) {
                logging::print( oxorany( "[c_hook] create: Kerenl hook has not been initialized." ) );
                if constexpr ( std::is_void_v<ret_t> ) {
                    return;
                }
                else {
                    return ret_t{};
                }
            }

            memcpy( this->m_original_code.data( ), reinterpret_cast< void* >( m_mapped_code ), m_shellcode_size );
            memcpy( reinterpret_cast< void* >( m_mapped_code ), this->m_compiled_shellcode.data( ), m_shellcode_size );

            using nt_query_counter_t = void* ( __fastcall* )( void*, std::uint64_t, std::uint64_t, call_struct_t* );
            auto* nt_query_counter = reinterpret_cast< nt_query_counter_t >(
                GetProcAddress(
                    GetModuleHandleA( "ntdll.dll" ),
                    "NtQueryAuxiliaryCounterFrequency" )
                );

            if ( !nt_query_counter ) {
                if constexpr ( std::is_void_v<ret_t> ) {
                    return;
                }
                else {
                    return ret_t{};
                }
            }

            std::uint64_t dummy = 0;
            auto result = nt_query_counter( &dummy, 0, 0, &call_context );

            memcpy( reinterpret_cast< void* >( m_mapped_code ), this->m_original_code.data( ), m_shellcode_size );

            if constexpr ( std::is_void_v<ret_t> ) {
                return;
            }
            else {
                return reinterpret_cast< ret_t >( result );
            }
        }

    private:
        std::uint64_t m_hook_address = 0;
        std::uint64_t m_original_fn = 0;
        std::uint64_t m_discardable_code = 0;
        std::uint64_t m_mapped_code = 0;
        std::vector<std::uint8_t> m_original_code;

        std::vector<std::uint8_t> m_compiled_shellcode;
        std::size_t m_shellcode_size = 0;
    };
}