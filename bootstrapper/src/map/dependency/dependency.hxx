#pragma once

namespace map {
	class c_dependency {
	public:
		c_dependency() { }
		~c_dependency() { }
		c_dependency( const std::string& file_path ) : m_dependency_path( file_path ) { }

		bool load( ) {
			std::ifstream dependency( m_dependency_path, std::ios::binary | std::ios::ate );
			if ( !dependency.is_open( ) ) {
				logging::print( oxorany( "[c_dependency] load: Could not open file" ) );
				return false;
			}

			logging::print( oxorany( "[c_dependency] load: Mapping dependency [%s]" ), m_dependency_path.c_str( ) );

			auto dependency_size = dependency.tellg( );
			if ( dependency_size <= 0 ) {
				logging::print( oxorany( "[c_dependency] load: Invalid file size" ) );
				return false;
			}

			m_dependency_image.resize( dependency_size );

			dependency.seekg( 0, std::ios::beg );
			if ( !dependency.read( reinterpret_cast< char* >( m_dependency_image.data( ) ), dependency_size ) ) {
				logging::print( oxorany( "[c_dependency] load: Could not access file contents" ) );
				m_dependency_image.clear( );
				return false;
			}

			this->m_dos_header = reinterpret_cast< dos_header_t* >( m_dependency_image.data( ) );
			if ( !m_dos_header->is_valid( ) ) {
				logging::print( oxorany( "[c_dependency] load: Could not get DOS header" ) );
				m_dependency_image.clear( );
				return false;
			}

			this->m_nt_headers = reinterpret_cast< nt_headers_t* >( m_dependency_image.data( ) + m_dos_header->m_lfanew );
			if ( !m_nt_headers->is_valid( ) ) {
				logging::print( oxorany( "[c_dependency] load: Could not get NT header" ) );
				m_dependency_image.clear( );
				return false;
			}

			{
				auto nt_base = reinterpret_cast< std::uint8_t* >( m_dependency_image.data( ) + m_dos_header->m_lfanew );
				auto opt_base = nt_base + offsetof( nt_headers_t, m_magic );
				this->m_section_header = reinterpret_cast< section_header_t* >( opt_base + m_nt_headers->m_size_of_optional_header );
			}
			if ( !m_section_header ) {
				logging::print( oxorany( "[c_dependency] load: Could not get section header" ) );
				m_dependency_image.clear( );
				return false;
			}

			return true;
		}

		std::uint64_t get_entry_point( ) {
			auto entry_rva = m_nt_headers->m_address_of_entry_point;
			for ( auto idx = 0ull; idx < get_section_count( ); idx++ ) {
				auto section = get_section( idx );
				if ( entry_rva >= section->m_virtual_address &&
					entry_rva < ( section->m_virtual_address + section->m_virtual_size ) ) {
					logging::print( oxorany( "[c_dependency] get_entry_point: Found entry point in section: %s" ), section->m_name );
					break;
				}
			}

			return entry_rva;
		}

		template <typename t>
		t rva_to_va( std::uint64_t rva ) {
			if ( rva < static_cast< std::uint64_t >( m_nt_headers->m_size_of_headers ) )
				return reinterpret_cast< t >( m_dependency_image.data( ) + rva );

			auto first_section = m_section_header;
			for ( auto section = first_section; section < first_section + m_nt_headers->m_number_of_sections; section++ ) {
				const std::uint32_t va = static_cast< std::uint32_t >( section->m_virtual_address );
				const std::uint32_t virtual_size = static_cast< std::uint32_t >( section->m_virtual_size );
				if ( rva >= va && rva < static_cast< std::uint64_t >( va ) + virtual_size ) {
					const std::uint32_t raw = static_cast< std::uint32_t >( section->m_pointer_to_raw_data );
					const std::uint32_t raw_size = static_cast< std::uint32_t >( section->m_size_of_raw_data );
					const std::uint32_t delta = static_cast< std::uint32_t >( rva - va );
					if ( delta < raw_size )
						return reinterpret_cast< t >( m_dependency_image.data( ) + raw + delta );
					return reinterpret_cast< t >( nullptr );
				}
			}
			return reinterpret_cast< t >( nullptr );
		}

		auto get_section( std::uint32_t idx )  {
			return std::make_shared< section_header_t > ( m_section_header[ idx ] );
		}

		std::vector< std::uint8_t > get_raw_image( ) {
			return m_dependency_image;
		}

		std::uint64_t get_image_base( ) {
			return m_nt_headers->m_image_base;
		}

		std::uint32_t get_size( ) {
			return m_nt_headers->m_size_of_image;
		}

		std::uint32_t get_header_size( ) {
			return m_nt_headers->m_size_of_headers;
		}

		bool is_reloc_stripped( ) const {
			return ( m_nt_headers->m_characteristics & IMAGE_FILE_RELOCS_STRIPPED );
		}

		auto get_reloc_directory( ) const {
			return &m_nt_headers->m_base_relocation_table;
		}

		auto get_import_table( ) const {
			return &m_nt_headers->m_import_table;
		}

		auto get_delay_import_descriptor( ) const {
			return &m_nt_headers->m_delay_import_descriptor;
		}

		std::uint32_t get_section_count( ) {
			return m_nt_headers->m_number_of_sections;
		}

		std::wstring get_file_name( ) const {
			if ( m_dependency_path.empty( ) )
				return { };

			const auto last_sep = m_dependency_path.find_last_of( "\\/" );
			const std::string name = last_sep == std::string::npos ? m_dependency_path : m_dependency_path.substr( last_sep + 1 );
			return std::wstring( name.begin( ), name.end( ) );
		}

	private:
		std::string m_dependency_path{ };
		std::vector< std::uint8_t > m_dependency_image{ };
		nt_headers_t* m_nt_headers = nullptr;
		dos_header_t* m_dos_header = nullptr;
		section_header_t* m_section_header = nullptr;
	};
}