#pragma once

namespace dependency {
    class c_dependency {
    public:
        c_dependency( const std::string& file_path ) {
            m_dependency_name = utility::ansi_to_wstring( file_path );

            std::ifstream file( file_path, std::ios::binary | std::ios::ate );
            if ( !file.is_open( ) ) {
                logging::print( oxorany( "[c_dependency] initialize: Failed to open file: %s" ), file_path.c_str( ) );
                return;
            }

            const auto file_size = file.tellg( );
            m_dependency.resize( static_cast< size_t >( file_size ) );

            file.seekg( 0, std::ios::beg );
            if ( !file.read( reinterpret_cast< char* >( m_dependency.data( ) ), file_size ) ) {
                logging::print( oxorany( "[c_dependency] initialize: Failed to read file: %s" ), file_path.c_str( ) );
                m_dependency.clear( );
                return;
            }

            this->m_dos_header = reinterpret_cast
                < dos_header_t* >( m_dependency.data( ) );
            if ( !m_dos_header->is_valid( ) ) {
                logging::print( oxorany( "[c_dependency] initialize: Invalid DOS signature in: %s" ), file_path.c_str( ) );
                m_dependency.clear( );
                return;
            }

            this->m_nt_headers = reinterpret_cast
                < nt_headers_t* >( m_dependency.data( ) + m_dos_header->m_lfanew );
            if ( !m_nt_headers->is_valid( ) ) {
                logging::print( oxorany( "[c_dependency] initialize: Invalid NT headers in: %s" ), file_path.c_str( ) );
                m_dependency.clear( );
                return;
            }

            this->m_section_header = reinterpret_cast< section_header_t* >( reinterpret_cast
                < std::uintptr_t >( m_nt_headers ) + m_nt_headers->m_size_of_optional_header + 0x18 );
            return;
        }

        bool is_dll( ) const {
            if ( !m_nt_headers )
                return false;

            return ( m_nt_headers->m_characteristics & 0x2000 );
        }

        std::uint32_t size_of_image( ) const {
            return m_nt_headers->m_size_of_image;
        }

        bool map_relocs( std::uint64_t new_image_base ) {
            struct reloc_entry {
                std::uint32_t m_to_rva;
                std::uint32_t m_size;
                struct {
                    std::int16_t m_offset : 0xc;
                    std::int16_t m_type : 0x4;
                } m_item[ 0x1 ];
                std::int8_t m_pad0[ 0x2 ];
            };

            auto delta_offset{ new_image_base - m_nt_headers->m_image_base };
            if ( !delta_offset )
                return false;

            auto reloc_ent{ reinterpret_cast< reloc_entry* >( rva_va( m_nt_headers->m_base_relocation_table.m_virtual_address ) ) };
            auto reloc_end{ reinterpret_cast< std::uintptr_t >( reloc_ent ) + m_nt_headers->m_base_relocation_table.m_size };
            if ( !reloc_ent )
                return false;

            while ( reinterpret_cast< std::uintptr_t >( reloc_ent ) < reloc_end && reloc_ent->m_size ) {
                auto records_count{ ( reloc_ent->m_size - sizeof( std::int8_t* ) ) >> 0x1 };
                for ( std::size_t i{}; i < records_count; i++ ) {
                    auto fix_type{ reloc_ent->m_item[ i ].m_type };
                    auto shift_delta{ reloc_ent->m_item[ i ].m_offset % 0x1000 };

                    if ( fix_type == 0x3 || fix_type == 0xa ) {
                        auto fix_va{ rva_va( reloc_ent->m_to_rva ) };
                        if ( !fix_va )
                            fix_va = reinterpret_cast< std::int8_t* >( m_dos_header );
                        *reinterpret_cast< std::uint64_t* >( fix_va + shift_delta ) += delta_offset;
                    }
                }

                reloc_ent = ( reloc_entry* )( ( std::uint8_t* )reloc_ent + reloc_ent->m_size );
            }

            return true;
        }

        bool map_imports( ) {
            if ( !m_nt_headers->m_import_table.m_virtual_address )
                return true;

            auto import_desc = reinterpret_cast< import_descriptor_t* >(
                rva_va( m_nt_headers->m_import_table.m_virtual_address ) );

            if ( !import_desc ) {
                logging::print( oxorany( "[c_dependency] map_imports: Failed to resolve import table RVA" ) );
                return false;
            }

            for ( auto descriptor = import_desc; descriptor->m_name; descriptor++ ) {
                auto module_name = reinterpret_cast< char* >( rva_va( descriptor->m_name ) );
                if ( !module_name ) {
                    logging::print( oxorany( "[c_dependency] map_imports: Failed to resolve module name RVA" ) );
                    return false;
                }

                auto module_lib = LoadLibraryA( module_name );
                if ( !module_lib ) {
                    logging::print( oxorany( "[c_dependency] map_imports: Failed to load library: %s" ), module_name );
                    return false;
                }

                auto module_path = get_module_path( module_name, m_dependency_name );
                if ( module_path.empty( ) ) {
                    FreeLibrary( module_lib );
                    return false;
                }

                auto file_name = utility::strip_path( module_path );
                auto module_base = hyperspace::target::get_process_module( file_name.c_str( ) );
                if ( !module_base ) {
                    logging::print( oxorany( "[c_dependency] map_imports: Could not find module in target: %ws" ), file_name.c_str( ) );
                    FreeLibrary( module_lib );
                    return false;
                }

                auto thunk = reinterpret_cast< image_thunk_data_t* >( rva_va( descriptor->m_first_thunk ) );
                if ( !thunk ) {
                    logging::print( oxorany( "[c_dependency] map_imports: Failed to resolve first thunk RVA" ) );
                    FreeLibrary( module_lib );
                    return false;
                }

                auto original_thunk_rva = descriptor->m_original_first_thunk ?
                    descriptor->m_original_first_thunk : descriptor->m_first_thunk;
                auto original_thunk = reinterpret_cast< image_thunk_data_t* >( rva_va( original_thunk_rva ) );

                if ( !original_thunk ) {
                    logging::print( oxorany( "[c_dependency] map_imports: Failed to resolve original thunk RVA" ) );
                    FreeLibrary( module_lib );
                    return false;
                }

                for ( auto current_thunk = thunk; original_thunk->m_u1.m_address_of_data; original_thunk++, current_thunk++ ) {
                    std::uint64_t function = 0;

                    if ( original_thunk->m_u1.m_ordinal & IMAGE_ORDINAL_FLAG64 ) {
                        auto ordinal = ( std::uint16_t )( original_thunk->m_u1.m_ordinal & 0xFFFF );
                        function = reinterpret_cast< std::uint64_t >( GetProcAddress( module_lib, ( LPCSTR )ordinal ) );
                    }
                    else {
                        auto import_name = reinterpret_cast< image_import_name_t* >(
                            rva_va( original_thunk->m_u1.m_address_of_data ) );

                        if ( !import_name ) {
                            logging::print( oxorany( "[c_dependency] map_imports: Failed to resolve import name RVA" ) );
                            FreeLibrary( module_lib );
                            return false;
                        }

                        function = reinterpret_cast< std::uint64_t >( GetProcAddress( module_lib, import_name->m_name ) );
                    }

                    if ( !function ) {
                        logging::print( oxorany( "[c_dependency] map_imports: Failed to resolve import in: %s" ), module_name );
                        FreeLibrary( module_lib );
                        return false;
                    }

                    auto offset = function - reinterpret_cast< std::uint64_t >( module_lib );
                    current_thunk->m_u1.m_function = module_base + offset;
                }

                FreeLibrary( module_lib );
            }

            return true;
        }

        bool map_sections( uint64_t new_image_base ) {
            auto section = m_section_header;
            for ( auto idx = 0; idx < m_nt_headers->m_number_of_sections; idx++, section++ ) {
                auto dst = new_image_base + section->m_virtual_address;
                auto src = reinterpret_cast< std::uint64_t >
                    ( m_dependency.data( ) + section->m_pointer_to_raw_data );

                auto size = section->m_virtual_size;
                if ( !hyperspace::write_virt( dst, reinterpret_cast< void* >( src ), size ) )
                    return false;
            }

            return true;
        }

        bool erase_discarded_sec( uint64_t mapped_image_base ) {
            auto section = m_section_header;

            for ( auto idx = 0; idx < m_nt_headers->m_number_of_sections; idx++, section++ ) {
                if ( section->m_characteristics & 0x02000000 ) {
                    auto size = section->m_virtual_size;
                    if ( !size )
                        continue;

                    auto dst = mapped_image_base + section->m_virtual_address;

                    std::vector<uint8_t> zero_buffer( size, 0 );
                    if ( !hyperspace::write_virt( dst, zero_buffer.data( ), size ) ) {
                        logging::print( oxorany( "[c_dependency] erase_discarded_sec: Could not erase section at RVA: 0x%X" ),
                            section->m_virtual_address );
                        return false;
                    }

                    logging::print( oxorany( "[c_dependency] erase_discarded_sec: Erased discarded section at RVA: 0x%X, Size: 0x%X" ),
                        section->m_virtual_address, size );
                }
            }

            return true;
        }

        std::uint64_t get_export( const char* export_name ) {
            if ( !m_nt_headers->m_export_table.m_virtual_address )
                return 0;

            auto export_dir = reinterpret_cast< export_directory_t* >(
                rva_va( m_nt_headers->m_export_table.m_virtual_address ) );

            if ( !export_dir )
                return 0;

            auto functions = reinterpret_cast< std::uint32_t* >(
                rva_va( export_dir->m_address_of_functions ) );
            auto names = reinterpret_cast< std::uint32_t* >(
                rva_va( export_dir->m_address_of_names ) );
            auto ordinals = reinterpret_cast< std::uint16_t* >(
                rva_va( export_dir->m_address_of_names_ordinals ) );

            if ( !functions || !names || !ordinals )
                return 0;

            for ( auto idx = 0; idx < export_dir->m_number_of_names; idx++ ) {
                auto name = reinterpret_cast< const char* >( rva_va( names[ idx ] ) );
                if ( !name )
                    continue;

                if ( !std::strcmp( name, export_name ) ) {
                    auto ordinal = ordinals[ idx ];
                    if ( ordinal >= export_dir->m_number_of_functions )
                        return 0;

                    return functions[ ordinal ];
                }
            }

            return 0;
        }

    private:
        std::wstring m_dependency_name{ };
        dos_header_t* m_dos_header{ nullptr };
        nt_headers_t* m_nt_headers{ nullptr };
        section_header_t* m_section_header{ nullptr };
        std::vector<uint8_t> m_dependency{ };

        std::int8_t* rva_va( const std::ptrdiff_t rva ) {
            for ( auto p_section{ m_section_header }; p_section < m_section_header + m_nt_headers->m_number_of_sections; p_section++ )
                if ( rva >= p_section->m_virtual_address && rva < p_section->m_virtual_address + p_section->m_virtual_size )
                    return ( std::int8_t* )m_dos_header + p_section->m_pointer_to_raw_data + ( rva - p_section->m_virtual_address );
            return {};
        }
    };
}