#pragma once 

namespace module {
    std::uint64_t allocate_between_modules( std::uint32_t size, bool extend_module = false ) {
        const auto page_mask = paging::page_4kb_size - 1;
        const auto aligned_size = ( size + page_mask ) & ~page_mask;
        const auto page_count = aligned_size >> paging::page_shift;

        auto ps_loaded_module_list = reinterpret_cast< list_entry_t* >(
            nt::get_export( nt::m_module_base, oxorany( "PsLoadedModuleList" ) )
            );
        if ( !ps_loaded_module_list )
            return 0;

        auto iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
            ps_loaded_module_list->m_flink
            );

        std::uint64_t allocation_base = 0;
        kldr_data_table_entry_t* prev_ldr_entry = nullptr;

        while ( reinterpret_cast< list_entry_t* >( iter_ldr_entry ) != ps_loaded_module_list ) {
            auto module_start = reinterpret_cast< std::uint64_t >( iter_ldr_entry->m_dll_base );

            if ( prev_ldr_entry ) {
                auto prev_module_end = reinterpret_cast< std::uint64_t >( prev_ldr_entry->m_dll_base ) +
                    prev_ldr_entry->m_size_of_image;
                auto gap_start = ( prev_module_end + 0xFFF ) & ~0xFFF;

                if ( gap_start < module_start ) {
                    auto gap_size = module_start - gap_start;

                    if ( gap_size >= aligned_size ) {
                        allocation_base = gap_start;

                        if ( extend_module ) {
                            auto new_module_end = allocation_base + aligned_size;
                            auto new_size = new_module_end -
                                reinterpret_cast< std::uint64_t >( prev_ldr_entry->m_dll_base );
                            new_size = ( new_size + 0xFFF ) & ~0xFFF;

                            prev_ldr_entry->m_size_of_image = static_cast< std::uint32_t >( new_size );
                        }

                        break;
                    }
                }
            }

            prev_ldr_entry = iter_ldr_entry;
            iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
                iter_ldr_entry->m_in_load_order_links.m_flink
                );
        }

        if ( !allocation_base ) {
            nt::dbg_print( oxorany( "[c_module] allocate_between_modules: Could not find gap between modules\n" ) );
            return 0;
        }

        std::uint64_t orig_cr3;
        paging::attach_to_process( paging::m_system_cr3, &orig_cr3 );

        if ( !paging::map_pte_entry( allocation_base, size ) ) {
            nt::dbg_print( oxorany( "[c_module] allocate_between_modules: Could not map page tables\n" ) );
            return 0;
        }

        paging::attach_to_process( orig_cr3 );
        return allocation_base;
    }

    std::uint64_t find_unused_space( std::uint64_t module_base, size_t space_size ) {
        auto dos_header = reinterpret_cast< dos_header_t* >( module_base );
        auto nt_headers = reinterpret_cast< nt_headers_t* >( module_base + dos_header->m_lfanew );
        if ( !dos_header->is_valid( ) || !nt_headers->is_valid( ) ) {
            return 0;
        }

        auto section_header = reinterpret_cast< section_header_t* >(
            reinterpret_cast< std::uintptr_t >( nt_headers ) +
            nt_headers->m_size_of_optional_header + 0x18
            );

        for ( auto idx = 0; idx < nt_headers->m_number_of_sections; idx++ ) {
            if ( std::strcmp( section_header[ idx ].m_name, ".data" ) )
                continue;

            if ( section_header[ idx ].m_virtual_address == 0 || section_header[ idx ].m_virtual_size == 0 )
                continue;

            if ( section_header[ idx ].m_virtual_address + section_header[ idx ].m_virtual_size > nt_headers->m_size_of_image )
                continue;

            auto section_base = module_base + section_header[ idx ].m_virtual_address;
            auto section_size = section_header[ idx ].m_virtual_size;

            auto unused_space = mmu::find_unused_space(
                reinterpret_cast < void* >( section_base ),
                section_size,
                space_size
            );

            if ( !unused_space )
                continue;

            std::uint64_t orig_cr3;
            paging::attach_to_process( paging::m_system_cr3, &orig_cr3 );
            if ( !paging::spoof_pte_range( unused_space, space_size ) ) {
                paging::attach_to_process( orig_cr3 );
                return 0;
            }

            paging::attach_to_process( orig_cr3 );
            return unused_space;
        }

        return 0;
    }

    std::uint64_t copy_on_allocation( std::uint64_t module_base, std::size_t space_size ) {
        auto dos_header = reinterpret_cast< dos_header_t* >( module_base );
        if ( !dos_header->is_valid( ) )
            return 0;

        auto nt_headers = reinterpret_cast< nt_headers_t* >( module_base + dos_header->m_lfanew );
        if ( !nt_headers->is_valid( ) )
            return 0;

        auto section_header = reinterpret_cast< section_header_t* >(
            reinterpret_cast< std::uintptr_t >( nt_headers ) +
            nt_headers->m_size_of_optional_header + 0x18 );

        for ( auto idx = 0; idx < nt_headers->m_number_of_sections; idx++ ) {
            if ( ( section_header[ idx ].m_characteristics & 0x20000000 ) )
                continue;

            auto section_base = module_base + section_header[ idx ].m_virtual_address;
            auto section_size = section_header[ idx ].m_virtual_size;

            auto target_address = mmu::find_unused_space(
                reinterpret_cast < void* >( section_base ),
                section_size,
                space_size
            );

            if ( !target_address )
                continue;

            paging::pt_entries_t pt_entries;
            if ( !paging::hyperspace_entries( pt_entries, target_address ) )
                continue;

            if ( pt_entries.m_pdpte.hard.page_size ) {
                nt::dbg_print( oxorany( "[COW] copy_on_allocation: splitting 1gb page to 4kb\n" ) );

                if ( !paging::split_1gb_to_4kb( target_address ) ) {
                    nt::dbg_print( oxorany( "[COW] copy_on_allocation: could not split 1gb page to 4kb\n" ) );
                    continue;
                }
            }

            if ( pt_entries.m_pde.hard.page_size ) {
                nt::dbg_print( oxorany( "[COW] copy_on_allocation: splitting 2mb page to 4kb\n" ) );

                if ( !paging::split_2mb_to_4kb( target_address ) ) {
                    nt::dbg_print( oxorany( "[COW] copy_on_allocation: could not split 2mb page to 4kb\n" ) );
                    continue;
                }
            }

            auto pte_address = paging::get_pte_address( target_address );
            if ( !pte_address )
                continue;

            //auto result = nt::mi_copy_on_write(
            //    target_address,
            //    reinterpret_cast< std::uint64_t* >( pte_address )
            //);

            //if ( result ) {
            //    nt::dbg_print( oxorany( "[COW] Could not enable CopyOnWrite for %llx\n" ), target_address );
            //    return 0;
            //}

            if ( !paging::protect_pages( target_address, space_size ) ) {
                nt::dbg_print( oxorany( "[COW] copy_on_allocation: could not spoof read write\n" ) );
                return 0;
            }

            return target_address;
        }

        return 0;
    }

    bool is_inside_module( std::uint64_t address, std::uint64_t* module_base = nullptr ) {
        auto ps_loaded_module_list = reinterpret_cast< list_entry_t* >(
            nt::get_export( nt::m_module_base, oxorany( "PsLoadedModuleList" ) )
            );
        if ( !ps_loaded_module_list )
            return false;

        auto iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
            ps_loaded_module_list->m_flink
            );

        while ( reinterpret_cast< list_entry_t* >( iter_ldr_entry ) != ps_loaded_module_list ) {
            auto size_of_image = iter_ldr_entry->m_size_of_image;
            auto dll_base = reinterpret_cast< std::uint64_t >( iter_ldr_entry->m_dll_base );
            if ( address >= dll_base &&
                 address < ( dll_base + size_of_image ) ) {
                if ( module_base ) *module_base = dll_base;
                return true;
            }

            iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
                iter_ldr_entry->m_in_load_order_links.m_flink
                );
        }

        return false;
    }

    std::uint64_t get_module_base( const wchar_t* module_name, std::uint64_t* module_size = nullptr ) {
        unicode_string_t module_name_string{ };
        nt::rtl_init_unicode_string( &module_name_string, module_name );

        auto ps_loaded_module_list = reinterpret_cast< list_entry_t* >(
            nt::get_export( nt::m_module_base, oxorany( "PsLoadedModuleList" ) )
            );
        if ( !ps_loaded_module_list )
            return false;

        auto iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
            ps_loaded_module_list->m_flink
            );

        while ( reinterpret_cast< list_entry_t* >( iter_ldr_entry ) != ps_loaded_module_list ) {
            if ( !nt::rtl_compare_unicode_string( &iter_ldr_entry->m_base_dll_name, &module_name_string, true ) ) {
                if ( module_size )
                    *module_size = iter_ldr_entry->m_size_of_image;
                return reinterpret_cast< std::uintptr_t >( iter_ldr_entry->m_dll_base );
            }

            iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
                iter_ldr_entry->m_in_load_order_links.m_flink
                );
        }

        return 0;
    }

    std::uint64_t get_module_size( const wchar_t* module_name ) {
        unicode_string_t module_name_string{ };
        nt::rtl_init_unicode_string( &module_name_string, module_name );

        auto ps_loaded_module_list = reinterpret_cast< list_entry_t* >(
            nt::get_export( nt::m_module_base, oxorany( "PsLoadedModuleList" ) )
            );
        if ( !ps_loaded_module_list )
            return false;

        auto iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
            ps_loaded_module_list->m_flink
            );

        while ( reinterpret_cast< list_entry_t* >( iter_ldr_entry ) != ps_loaded_module_list ) {
            if ( !nt::rtl_compare_unicode_string( &iter_ldr_entry->m_base_dll_name, &module_name_string, true ) ) {
                return iter_ldr_entry->m_size_of_image;
            }

            iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
                iter_ldr_entry->m_in_load_order_links.m_flink
                );
        }

        return 0;
    }

    std::uint64_t get_module_section( const wchar_t* module_name ) {
        unicode_string_t module_name_string{ };
        nt::rtl_init_unicode_string( &module_name_string, module_name );

        auto ps_loaded_module_list = reinterpret_cast< list_entry_t* >(
            nt::get_export( nt::m_module_base, oxorany( "PsLoadedModuleList" ) )
            );
        if ( !ps_loaded_module_list )
            return false;

        auto iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
            ps_loaded_module_list->m_flink
            );

        while ( reinterpret_cast< list_entry_t* >( iter_ldr_entry ) != ps_loaded_module_list ) {
            if ( !nt::rtl_compare_unicode_string( &iter_ldr_entry->m_base_dll_name, &module_name_string, true ) ) {
                std::uint64_t driver_section = 0;
                nt::mi_obtain_section_for_driver( &iter_ldr_entry->m_full_dll_name, &iter_ldr_entry->m_base_dll_name, &driver_section );
                if ( !driver_section )
                    return 0;

                return driver_section;
            }

            iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
                iter_ldr_entry->m_in_load_order_links.m_flink
                );
        }

        return 0;
    }
}