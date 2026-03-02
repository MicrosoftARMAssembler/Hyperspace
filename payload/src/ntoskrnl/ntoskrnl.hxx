#pragma once

struct pdb_symbols_t {
    std::uint64_t m_mi_copy_on_write;
    std::uint64_t m_mm_set_page_protection;
    std::uint64_t m_mi_is_page_on_bad_list;
    std::uint64_t m_mi_make_page_bad;
    std::uint64_t m_rtlp_debug_print_callback_list;
    std::uint64_t m_psp_load_image_notify_routine;
    std::uint64_t m_ki_is_process_termination_requested;
    std::uint64_t m_ps_query_thread_start_address;
    std::uint64_t m_mi_lock_page_table_page;
    std::uint64_t m_mm_initialize_process_address_space;
    std::uint64_t m_ke_get_prcb;
    std::uint64_t m_ki_nmi_callback_list_head;
    std::uint64_t m_po_idle;
    std::uint64_t m_etwp_max_pmc_counter;
    std::uint64_t m_etwp_debugger_data;
    std::uint64_t m_mi_obtain_section_for_driver;
    std::uint64_t m_mm_allocate_independent_pages;
    std::uint64_t m_mm_free_independent_pages;
    std::uint64_t m_ki_dynamic_trace_mask;
    std::uint64_t m_ps_enum_process_threads;
    std::uint64_t m_ps_integrity_check_enabled;
    std::uint64_t m_exp_license_watch_init_worker;
    std::uint64_t m_ki_filter_fiber_context;
    std::uint64_t m_ki_sw_interrupt_dispatch;
    std::uint64_t m_fs_rtl_uninitialize_small_mcb;
    std::uint64_t m_ki_dispatch_exception;
    std::uint64_t m_ki_expand_kernel_stack_and_callout;
    std::uint64_t m_fs_rtl_mdl_read_complete_dev_ex;
    std::uint64_t m_ki_initialize_boot_structures;
    std::uint64_t m_halp_stall_counter;
    std::uint64_t m_kd_trap;
    std::uint64_t m_ke_freeze_execution;
    std::uint64_t m_ke_stall_execution_processor;
    std::uint64_t m_kdp_report;
    std::uint64_t m_kdp_trap;
    std::uint64_t m_kd_enter_debugger;
    std::uint64_t m_nt_global_flag;
    std::uint64_t m_kdp_debug_routine_select;
    std::uint64_t m_kd_debugger_lock;
    std::uint64_t m_po_all_proc_intr_disabled;
    std::uint64_t m_kd_ignore_um_exceptions;
};

namespace nt {
    extern "C" std::uint64_t get_nt_base( );
    pdb_symbols_t m_pdb_symbols;
	std::uint64_t m_module_base;
    eprocess_t* m_eprocess;

    bool get_section( const char* section_name, std::uint64_t* exec_base, std::uint64_t* exec_size ) {
        auto dos_header{ reinterpret_cast< dos_header_t* > ( m_module_base ) };
        auto nt_headers{ reinterpret_cast< nt_headers_t* > ( m_module_base + dos_header->m_lfanew ) };
        if ( !dos_header->is_valid( )
            || !nt_headers->is_valid( ) )
            return false;

        auto section_header = reinterpret_cast< section_header_t* >(
            reinterpret_cast< std::uintptr_t >( nt_headers ) +
            nt_headers->m_size_of_optional_header + 0x18 );

        for ( int i = 0; i < nt_headers->m_number_of_sections; i++ ) {
            auto current_section_base = reinterpret_cast< std::uint64_t >( dos_header ) + section_header[ i ].m_virtual_address;
            if ( !std::strcmp( section_header[ i ].m_name, section_name ) ) {
                *exec_base = current_section_base;
                *exec_size = section_header[ i ].m_size_of_raw_data;
                break;
            }
        }

        return *exec_base && *exec_size;
    }

    bool next_exec_section( std::uint64_t* exec_base, std::uint64_t* exec_size ) {
        auto dos_header{ reinterpret_cast< dos_header_t* > ( m_module_base ) };
        auto nt_headers{ reinterpret_cast< nt_headers_t* > ( m_module_base + dos_header->m_lfanew ) };
        if ( !dos_header->is_valid( )
            || !nt_headers->is_valid( ) )
            return false;

        auto section_header = reinterpret_cast< section_header_t* >(
            reinterpret_cast< std::uintptr_t >( nt_headers ) +
            nt_headers->m_size_of_optional_header + 0x18 );

        for ( int i = 0; i < nt_headers->m_number_of_sections; i++ ) {
            auto current_section_base = reinterpret_cast< std::uint64_t >( dos_header ) + section_header[ i ].m_virtual_address;
            if ( section_header[ i ].m_characteristics & 0x20000000 ) {
                *exec_base = current_section_base;
                *exec_size = section_header[ i ].m_size_of_raw_data;
                break;
            }
        }

        return *exec_base && *exec_size;
    }

    std::uint64_t get_export ( std::uint64_t module_base, const char* export_name ) {
        auto dos_header{ reinterpret_cast< dos_header_t* > ( module_base ) };
        auto nt_headers{ reinterpret_cast< nt_headers_t* > ( module_base + dos_header->m_lfanew ) };
        if ( !dos_header->is_valid( )
            || !nt_headers->is_valid( ) )
            return {};

        auto exp_dir{ nt_headers->m_export_table.as_rva< export_directory_t* >( module_base ) };
        if ( !exp_dir->m_address_of_functions
            || !exp_dir->m_address_of_names
            || !exp_dir->m_address_of_names_ordinals )
            return {};

        auto name{ reinterpret_cast< std::int32_t* > ( module_base + exp_dir->m_address_of_names ) };
        auto func{ reinterpret_cast< std::int32_t* > ( module_base + exp_dir->m_address_of_functions ) };
        auto ords{ reinterpret_cast< std::int16_t* > ( module_base + exp_dir->m_address_of_names_ordinals ) };

        for ( std::int32_t i{}; i < exp_dir->m_number_of_names; i++ ) {
            auto cur_name{ module_base + name[ i ] };
            auto cur_func{ module_base + func[ ords[ i ] ] };
            if ( !cur_name
                || !cur_func )
                continue;

            if ( std::strcmp( export_name, reinterpret_cast< char* > ( cur_name ) ) == 0 )
                return cur_func;
        }
        return {};
    }

    const char* get_export_name( std::uint64_t func_addr ) {
        auto dos_header{ reinterpret_cast< dos_header_t* > ( m_module_base ) };
        auto nt_headers{ reinterpret_cast< nt_headers_t* > ( m_module_base + dos_header->m_lfanew ) };
        if ( !dos_header->is_valid( )
            || !nt_headers->is_valid( ) )
            return {};

        auto exp_dir{ nt_headers->m_export_table.as_rva< export_directory_t* >( m_module_base ) };
        if ( !exp_dir->m_address_of_functions
            || !exp_dir->m_address_of_names
            || !exp_dir->m_address_of_names_ordinals )
            return {};

        auto name{ reinterpret_cast< std::int32_t* > ( m_module_base + exp_dir->m_address_of_names ) };
        auto func{ reinterpret_cast< std::int32_t* > ( m_module_base + exp_dir->m_address_of_functions ) };
        auto ords{ reinterpret_cast< std::int16_t* > ( m_module_base + exp_dir->m_address_of_names_ordinals ) };

        for ( std::int32_t i{}; i < exp_dir->m_number_of_names; i++ ) {
            auto cur_func{ m_module_base + func[ ords[ i ] ] };
            if ( cur_func != func_addr )
                continue;

            return reinterpret_cast< const char* > ( m_module_base + name[ i ] );
        }
        return {};
    }

    std::uintptr_t find_pattern( std::uintptr_t base, std::size_t size, const char* pattern, const char* mask ) {
        const auto pattern_length = std::strlen( mask );
        if ( pattern_length == 0 || size < pattern_length )
            return 0;

        for ( std::size_t i = 0; i <= size - pattern_length; ++i ) {
            bool found = true;

            for ( std::size_t j = 0; j < pattern_length; ++j ) {
                if ( mask[ j ] == 'x' &&
                    *reinterpret_cast< const unsigned char* >( base + i + j ) != static_cast< unsigned char >( pattern[ j ] ) ) {
                    found = false;
                    break;
                }
            }

            if ( found )
                return base + i;
        }

        return 0;
    }

    std::uintptr_t find_signature( std::uintptr_t base, std::size_t size, const std::uint8_t* signature, const char* mask ) {
        const auto sig_length = std::strlen( mask );

        if ( sig_length == 0 || size < sig_length )
            return 0;

        for ( std::size_t i = 0; i <= size - sig_length; ++i ) {
            bool found = true;

            for ( std::size_t j = 0; j < sig_length; ++j ) {
                if ( mask[ j ] == 'x' &&
                    *reinterpret_cast< const std::uint8_t* >( base + i + j ) != signature[ j ] ) {
                    found = false;
                    break;
                }
            }

            if ( found )
                return base + i;
        }

        return 0;
    }

    std::uintptr_t find_ida_pattern( std::uintptr_t base, std::size_t size, const char* ida_pattern ) {
        std::uint8_t pattern[ 256 ];
        char mask[ 256 ];
        std::size_t pattern_size = 0;

        const char* ptr = ida_pattern;
        while ( *ptr ) {
            if ( *ptr == ' ' ) {
                ptr++;
                continue;
            }

            if ( *ptr == '?' ) {
                mask[ pattern_size ] = '?';
                pattern[ pattern_size++ ] = 0;
                ptr++;

                if ( *ptr == '?' ) ptr++;
            }
            else {
                char byte_str[ 3 ] = { ptr[ 0 ], ptr[ 1 ], 0 };
                pattern[ pattern_size ] = static_cast< std::uint8_t >( std::strtoul( byte_str, nullptr, 16 ) );
                mask[ pattern_size++ ] = 'x';
                ptr += 2;
            }

            if ( *ptr == ' ' ) ptr++;
        }

        mask[ pattern_size ] = 0;

        for ( std::size_t i = 0; i < pattern_size; i++ ) {
            if ( mask[ i ] == '?' ) mask[ i ] = '?';
            else mask[ i ] = 'x';
        }

        return find_signature( base, size, pattern, mask );
    }

    std::uintptr_t scan_text_section( const char* pattern, const char* mask ) {
        std::uint64_t text_base = 0;
        std::uint64_t text_size = 0;

        if ( !next_exec_section( &text_base, &text_size ) )
            return 0;

        return find_pattern( text_base, text_size, pattern, mask );
    }

    std::uintptr_t scan_ida_pattern( const char* ida_pattern ) {
        std::uint64_t text_base = 0;
        std::uint64_t text_size = 0;

        if ( !next_exec_section( &text_base, &text_size ) )
            return 0;

        return find_ida_pattern( text_base, text_size, ida_pattern );
    }
}