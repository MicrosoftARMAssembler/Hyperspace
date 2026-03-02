#pragma once

namespace kernel {
	class c_module {
	public:
		std::uint64_t m_module_base;
		char m_module_path[ 64 ]{ };

		c_module( ) { }
		~c_module( ) { }

		c_module( std::uint64_t module_base, char* module_path ) : m_module_base( module_base ) {
			if ( module_path ) {
				strcpy( m_module_path, module_path );
			}
		}

		std::uintptr_t get_export( const char* export_name ) const {
			auto symbol_address = pdb::get_symbol_address( m_module_path, export_name );
			if ( !symbol_address ) {
				return 0;
			}

			return symbol_address + m_module_base;
		}

		std::uint64_t rva( std::uint64_t instruction, int size, int offset ) {
			auto h_module = LoadLibraryExA( m_module_path, nullptr, DONT_RESOLVE_DLL_REFERENCES );
			if ( !h_module ) {
				return 0;
			}

			const auto module_base = reinterpret_cast< std::uint64_t >( h_module );
			const auto local_instruction = module_base + ( instruction - m_module_base );

			const auto p_displacement = reinterpret_cast< int32_t* >( local_instruction + size );
			const auto local_target = ( local_instruction + offset ) + *p_displacement;

			const auto target_address = m_module_base + ( local_target - module_base );

			FreeLibrary( h_module );
			return target_address;
		}

		std::uint64_t find_pattern( const char* pattern ) {
			auto h_module = LoadLibraryExA( m_module_path, nullptr, DONT_RESOLVE_DLL_REFERENCES );
			if ( !h_module ) {
				return 0;
			}

			auto module_start = reinterpret_cast< std::uint8_t* >( h_module );
			if ( !module_start ) {
				return 0;
			}

			auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( module_start );
			auto nt_header = reinterpret_cast< PIMAGE_NT_HEADERS >( module_start + dos_header->e_lfanew );
			auto module_end = reinterpret_cast< std::uint8_t* >( module_start + nt_header->OptionalHeader.SizeOfImage - 0x1000 );

			std::uint8_t* result = 0;
			auto curr_pattern = pattern;
			for ( ; module_start < module_end; ++module_start ) {
				bool skip_byte = ( *curr_pattern == '\?' );
				if ( skip_byte || *module_start == get_byte( curr_pattern ) ) {
					if ( !result ) result = module_start;
					skip_byte ? curr_pattern += 2 : curr_pattern += 3;
					if ( curr_pattern[ -1 ] == 0 ) {
						break;
					}
				}
				else if ( result ) {
					module_start = result;
					result = nullptr;
					curr_pattern = pattern;
				}
			}

			const auto module_base = reinterpret_cast< std::uintptr_t >( h_module );
			auto address = m_module_base + (
				reinterpret_cast< std::uintptr_t >( result ) - module_base
				);

			FreeLibrary( h_module );
			return address;
		}

		std::uint64_t find_signature( const std::uint8_t* signature, const char* mask ) {
			auto h_module = LoadLibraryExA( m_module_path, nullptr, DONT_RESOLVE_DLL_REFERENCES );
			if ( !h_module ) {
				return 0;
			}

			auto module_start = reinterpret_cast< std::uint8_t* >( h_module );
			if ( !module_start ) {
				return 0;
			}

			auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( module_start );
			auto nt_header = reinterpret_cast< PIMAGE_NT_HEADERS >( module_start + dos_header->e_lfanew );
			auto module_end = reinterpret_cast< std::uint8_t* >( module_start + nt_header->OptionalHeader.SizeOfImage - 0x1000 );

			std::uint64_t found_address = 0;
			const auto mask_length = strlen( mask );

			for ( auto i = module_start; i < module_end; ++i ) {
				bool found = true;
				for ( auto j = 0; j < mask_length; ++j ) {
					if ( mask[ j ] == 'x' && i[ j ] != signature[ j ] ) {
						found = false;
						break;
					}
				}

				if ( found ) {
					found_address = m_module_base + ( reinterpret_cast< std::uintptr_t >( i ) - reinterpret_cast< std::uintptr_t >( h_module ) );
					break;
				}
			}

			FreeLibrary( h_module );
			return found_address;
		}

		std::uint64_t find_signature( std::uint64_t start_address, std::uint64_t scan_size, const std::uint8_t* signature, const char* mask ) {
			if ( !signature || !mask || scan_size == 0 ) {
				return 0;
			}

			auto h_module = LoadLibraryExA( m_module_path, nullptr, DONT_RESOLVE_DLL_REFERENCES );
			if ( !h_module ) {
				return 0;
			}

			const auto module_base = reinterpret_cast< std::uint64_t >( h_module );
			const auto local_start = module_base + ( start_address - m_module_base );
			const auto local_end = local_start + scan_size;

			auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( h_module );
			auto nt_header = reinterpret_cast< PIMAGE_NT_HEADERS >( h_module + dos_header->e_lfanew );
			const auto module_end = reinterpret_cast< std::uint64_t >( h_module + nt_header->OptionalHeader.SizeOfImage );

			if ( local_start < reinterpret_cast< std::uint64_t >( h_module ) || local_end > module_end ) {
				FreeLibrary( h_module );
				return 0;
			}

			std::uint64_t found_address = 0;
			const auto mask_length = strlen( mask );

			for ( auto i = local_start; i < local_end - mask_length; ++i ) {
				bool found = true;
				for ( auto j = 0; j < mask_length; ++j ) {
					if ( mask[ j ] == 'x' && reinterpret_cast< std::uint8_t* >( i )[ j ] != signature[ j ] ) {
						found = false;
						break;
					}
				}

				if ( found ) {
					found_address = m_module_base + ( i - module_base );
					break;
				}
			}

			FreeLibrary( h_module );
			return found_address;
		}
	};

	std::unique_ptr< c_module > get_kernel_module( const char* module_name ) {
		char full_path[ 64 ]{ };

		unsigned long buffer_size = 0;
		auto status = NtQuerySystemInformation(
			static_cast< SYSTEM_INFORMATION_CLASS >( 11 ),
			nullptr,
			0,
			&buffer_size
		);

		if ( status != 0xC0000004L )
			return nullptr;

		auto buffer = std::make_unique< std::uint8_t[ ] >( buffer_size );
		status = NtQuerySystemInformation(
			static_cast< SYSTEM_INFORMATION_CLASS >( 11 ),
			buffer.get( ),
			buffer_size,
			&buffer_size
		);

		if ( !NT_SUCCESS( status ) ) {
			return nullptr;
		}

		std::uint64_t module_base = 0;
		const auto modules = reinterpret_cast< rtl_process_modules_t* >( buffer.get( ) );
		for ( auto idx = 0u; idx < modules->m_count; ++idx ) {
			const auto current_module_name = std::string(
				reinterpret_cast< char* >( modules->m_modules[ idx ].m_full_path ) +
				modules->m_modules[ idx ].m_offset_to_file_name
			);

			if ( !_stricmp( current_module_name.c_str( ), module_name ) ) {
				GetWindowsDirectoryA( full_path, 64 );
				strcat( full_path, "\\" );
				strcat( full_path, ( char* )&modules->m_modules[ idx ].m_full_path[ 12 ] );

				module_base = reinterpret_cast< std::uint64_t >( modules->m_modules[ idx ].m_image_base );
				break;
			}
		}

		return std::make_unique< c_module >( module_base, full_path );
	}
}