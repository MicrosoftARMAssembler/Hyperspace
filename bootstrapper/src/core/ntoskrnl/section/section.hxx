
namespace section {
    class c_section {
    public:
        c_section( ) { }
        ~c_section( ) { }
        c_section( std::shared_ptr< utility::c_module > kernel_module ) : m_kernel_module( kernel_module ){ }

        std::uint64_t find_unused_space( std::size_t size ) {
            std::uint64_t section_base = 0;
            std::uint32_t section_size = 0;
            if ( !find_discardable_section( &section_base, &section_size ) ) {
                return 0;
            }

            return find_unused_space( section_base, section_size, size );
        }

    private:
        std::shared_ptr< utility::c_module > m_kernel_module;

        bool find_discardable_section( std::uint64_t* section_base, std::uint32_t* section_size ) {
            auto dos_header_pa = g_paging->translate_linear( m_kernel_module->m_module_base );
            if ( !dos_header_pa )
                return false;

            dos_header_t dos_header{ };
            if ( !g_paging->m_read_physical( dos_header_pa,
                &dos_header, sizeof( dos_header_t ) )
                ) return false;

            if ( !dos_header.is_valid( ) )
                return false;

            auto nt_header_pa = g_paging->translate_linear(
                m_kernel_module->m_module_base + dos_header.m_lfanew );
            if ( !nt_header_pa )
                return false;

            nt_headers_t nt_headers{ };
            if ( !g_paging->m_read_physical( nt_header_pa,
                &nt_headers, sizeof( nt_headers_t ) )
                ) return false;

            if ( !nt_headers.is_valid( ) )
                return false;

            auto section_headers_offset = m_kernel_module->m_module_base + dos_header.m_lfanew +
                offsetof( nt_headers_t, m_magic );
            section_headers_offset += nt_headers.m_size_of_optional_header;

            for ( int i = 0; i < nt_headers.m_number_of_sections; i++ ) {
                auto section_header_pa = g_paging->translate_linear(
                    section_headers_offset + i * sizeof( section_header_t ) );
                if ( !section_header_pa )
                    return false;

                section_header_t section_header{ };
                if ( !g_paging->m_read_physical(
                    section_header_pa,
                    &section_header, sizeof( section_header_t ) )
                    ) return false;

                if ( !strncmp( reinterpret_cast< const char* >( section_header.m_name ), oxorany( ".text" ), 5 ) ) {
                    *section_base = m_kernel_module->m_module_base + section_header.m_virtual_address;
                    *section_size = section_header.m_size_of_raw_data;
                    return true;
                }
            }

            logging::print( oxorany( "[c_section] find_unused_space: Could not find discardable section" ) );
            return false;
        }

        std::uint64_t find_unused_space( std::uint64_t section_base, uint32_t section_size, size_t shell_size ) {
            auto section_pa = g_paging->translate_linear( section_base );
            if ( !section_pa )
                return 0;
            
            auto section_data = std::make_unique< std::uint8_t[ ] >( section_size );
            if ( !g_paging->m_read_physical( section_pa,
                section_data.get( ), section_size )
                ) return 0;

            uint32_t free_space_offsets[ 128 ];
            auto free_space_count = 0;

            for ( auto i = 0; i <= section_size - shell_size; ) {
                auto found_space = true;
                auto largest_fit = 0;

                for ( auto j = 0; j < shell_size; ++j ) {
                    if ( section_data[ i + j ] != 0x00 ) {
                        found_space = false;
                        break;
                    }
                }

                if ( found_space ) {
                    largest_fit = shell_size;

                    for ( auto j = shell_size; i + j < section_size; ++j ) {
                        if ( section_data[ i + j ] != 0x00 ) {
                            break;
                        }
                        largest_fit = j + 1;
                    }

                    if ( free_space_count < 128 ) {
                        free_space_offsets[ free_space_count++ ] = i;
                    }
                    else {
                        break;
                    }

                    i += largest_fit;
                }
                else {
                    ++i;
                }
            }

            if ( free_space_count == 0 ) {
                logging::print( oxorany( "[c_section] find_unused_space: Could not find free space" ) );
                return 0;
            }

            auto random_index = static_cast< uint32_t >( rand( ) % free_space_count );
            auto chosen_offset = free_space_offsets[ random_index ];
            return section_base + chosen_offset;
        }
    };
}