#pragma once

namespace module {
    std::uint64_t emunerate_module_gaps( std::function< bool( std::uint64_t, std::size_t ) > callback ) {
        auto ps_loaded_module_list = nt::ps_loaded_module_list( );
        if ( !ps_loaded_module_list )
            return 0;

        list_entry_t loaded_module_list;
        nt::memcpy( &loaded_module_list, ps_loaded_module_list, sizeof( list_entry_t ) );
        auto iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >( loaded_module_list.m_flink );
        
        kldr_data_table_entry_t prev_ldr_entry{ };
        while ( reinterpret_cast< std::uint64_t >( iter_ldr_entry ) !=
            reinterpret_cast< std::uint64_t >( ps_loaded_module_list ) ) {
            if ( !nt::is_address_valid( iter_ldr_entry ) ) {
                break;
            }

            kldr_data_table_entry_t ldr_entry;
            nt::memcpy( &ldr_entry, iter_ldr_entry, sizeof( kldr_data_table_entry_t ) );

            wchar_t module_name[ 256 ] = { 0 };
            if ( ldr_entry.m_base_dll_name.m_buffer && ldr_entry.m_base_dll_name.m_length > 0 ) {
                std::size_t copy_size = min( ldr_entry.m_base_dll_name.m_length, sizeof( module_name ) - sizeof( wchar_t ) );
                nt::memcpy( module_name, ldr_entry.m_base_dll_name.m_buffer, copy_size );

                logging::print( oxorany( "[module] module name: %ls" ), module_name );
            }
            else {
                logging::print( oxorany( "[module] module name: <invalid>" ) );
            }

            auto module_start = reinterpret_cast< std::uint64_t >( ldr_entry.m_dll_base );

            if ( prev_ldr_entry.m_dll_base ) {
                auto prev_module_end = reinterpret_cast< std::uint64_t >( prev_ldr_entry.m_dll_base ) +
                    prev_ldr_entry.m_size_of_image;
                auto gap_start = ( prev_module_end + 0xFFF ) & ~0xFFF;

                if ( gap_start < module_start ) {
                    auto gap_size = module_start - gap_start;

                    if ( callback( gap_start, gap_size ) ) {
                        return gap_start;
                    }
                }
            }

            prev_ldr_entry = ldr_entry;
            iter_ldr_entry = reinterpret_cast< kldr_data_table_entry_t* >(
                ldr_entry.m_in_load_order_links.m_flink
                );
        }

        return 0;
    }
}