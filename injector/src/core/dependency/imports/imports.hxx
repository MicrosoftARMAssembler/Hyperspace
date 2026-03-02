#pragma once

namespace dependency {
	std::wstring get_module_path( std::string module_name, std::wstring library_name ) {
		auto module_string = utility::ansi_to_wstring( module_name );
		auto module_path = utility::to_lower( std::move( module_string ) );
		auto file_name = utility::strip_path( module_path );

		if ( file_name.find( oxorany( L"ext-ms-" ) ) == 0 )
			file_name.erase( 0, 4 );

		auto resolved_name = g_apiset->resolve_api_set( file_name, library_name );
		if ( resolved_name != file_name ) {
			wchar_t system_path[ MAX_PATH ] = { 0 };
			if ( GetSystemDirectoryW( system_path, MAX_PATH ) ) {
				auto full_path = std::wstring( system_path ) + oxorany( L"\\" ) + resolved_name;
				if ( utility::file_exists( full_path ) ) {
					return full_path;
				}
			}
		}

		HKEY handle = nullptr;
		auto result = RegOpenKeyW( HKEY_LOCAL_MACHINE,
			oxorany( L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs" ),
			&handle );

		if ( result == ERROR_SUCCESS && handle ) {
			for ( DWORD idx = 0; idx < 0x1000; idx++ ) {
				wchar_t value_name[ MAX_PATH ] = { 0 };
				wchar_t value_data[ MAX_PATH ] = { 0 };
				DWORD name_size = MAX_PATH;
				DWORD data_size = MAX_PATH * sizeof( wchar_t );
				DWORD dwType = 0;

				result = RegEnumValueW( handle, idx, value_name, &name_size,
					nullptr, &dwType,
					reinterpret_cast< LPBYTE >( value_data ), &data_size );

				if ( result != ERROR_SUCCESS )
					break;

				if ( _wcsicmp( value_data, file_name.c_str( ) ) == 0 ) {
					wchar_t sys_path[ MAX_PATH ] = { 0 };
					if ( GetSystemDirectoryW( sys_path, MAX_PATH ) ) {
						RegCloseKey( handle );
						return std::wstring( sys_path ) + oxorany( L"\\" ) + value_data;
					}
				}
			}
			RegCloseKey( handle );
		}

		auto exe_directory = utility::get_exe_directory( );
		if ( !exe_directory.empty( ) ) {
			exe_directory += oxorany( L"\\" ) + file_name;
			if ( utility::file_exists( exe_directory ) ) {
				return exe_directory;
			}
		}

		auto process_directory = utility::get_process_directory( hyperspace::target::m_process_id );
		if ( !process_directory.empty( ) ) {
			auto process_path = process_directory + oxorany( L"\\" ) + file_name;
			if ( utility::file_exists( process_path ) ) {
				return process_path;
			}
		}

		wchar_t tmp_path[ MAX_PATH * 16 ] = { 0 };
		if ( GetSystemDirectoryW( tmp_path, MAX_PATH ) ) {
			auto sys_path = std::wstring( tmp_path ) + oxorany( L"\\" ) + file_name;
			if ( utility::file_exists( sys_path ) ) {
				return sys_path;
			}
		}

		if ( GetCurrentDirectoryW( MAX_PATH, tmp_path ) ) {
			auto current_path = std::wstring( tmp_path ) + oxorany( L"\\" ) + file_name;
			if ( utility::file_exists( current_path ) ) {
				return current_path;
			}
		}

		if ( GetEnvironmentVariableW( oxorany( L"PATH" ), tmp_path, MAX_PATH * 16 ) ) {
			wchar_t* context = nullptr;
			for ( wchar_t* directory = wcstok_s( tmp_path, oxorany( L";" ), &context );
				directory != nullptr;
				directory = wcstok_s( nullptr, oxorany( L";" ), &context ) ) {
				auto path_entry = std::wstring( directory ) + oxorany( L"\\" ) + file_name;
				if ( utility::file_exists( path_entry ) ) {
					return path_entry;
				}
			}
		}

		if ( GetSystemDirectoryW( tmp_path, MAX_PATH ) ) {
			auto sys_path = utility::wstring_to_ansi( tmp_path );
			auto file_narrow = utility::wstring_to_ansi( file_name );
			auto full_path = sys_path + "\\" + file_narrow;

			auto maps = pdb::get_module_mappings( full_path.c_str( ) );

			std::string in_lower = module_name;
			std::transform( in_lower.begin( ), in_lower.end( ), in_lower.begin( ), ::tolower );

			for ( const auto& m : maps ) {
				std::string original_lower = m.original_name;
				std::transform(
					original_lower.begin( ),
					original_lower.end( ),
					original_lower.begin( ),
					::tolower
				);

				if ( original_lower == in_lower ) {
					auto wide_mapped = utility::ansi_to_wstring( m.mapped_name );
					if ( wide_mapped.find( L".dll" ) == std::wstring::npos )
						wide_mapped += L".dll";

					return wide_mapped;
				}
			}
		}

		return std::wstring( );
	}
}