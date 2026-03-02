#pragma once
#define in_range(x, a, b) (x >= a && x <= b) 
#define get_bits(x) (in_range(x, '0', '9') ? (x - '0') : ((x - 'A') + 0xA))
#define get_byte(x) ((BYTE)(get_bits(x[0]) << 4 | get_bits(x[1])))
#define putchar(c) putc((c),stdout)

namespace utility {
	std::uint64_t get_module( const std::wstring& module_name ) {
		const auto* peb = reinterpret_cast< peb_t* >( __readgsqword( 0x60 ) );
		if ( peb == nullptr ) {
			return 0;
		}

		auto first_module =
			reinterpret_cast< kldr_data_table_entry_t* >( peb->m_ldr->m_module_list_memory_order.m_flink );
		auto current_module = first_module;

		do {
			auto entry = reinterpret_cast< kldr_data_table_entry_t* >(
				reinterpret_cast< std::uint64_t >( current_module ) - sizeof( list_entry_t ) );

			if ( ( ( entry->m_base_dll_name.m_buffer != nullptr ) && !_wcsicmp( entry->m_base_dll_name.m_buffer, module_name.c_str( ) ) ) ) {
				const auto module_base = reinterpret_cast< std::uintptr_t >( entry->m_dll_base );

				return module_base;
			}

			current_module =
				reinterpret_cast< kldr_data_table_entry_t* >( reinterpret_cast< list_entry_t* >( current_module )->m_flink );
		} while ( current_module != first_module );

		return 0;
	}

	bool adjust_privilege( std::uint32_t privilege ) {
		auto ntdll = GetModuleHandleA( "ntdll.dll" );
		if ( !ntdll )
			return false;

		typedef NTSTATUS( __stdcall* rtl_adjust_privilege_fn )( DWORD, BOOL, INT, PBOOL );
		auto rtl_adjust_privilege = reinterpret_cast< rtl_adjust_privilege_fn >( GetProcAddress( ntdll, "RtlAdjustPrivilege" ) );

		BOOL was_enabled;
		auto status = rtl_adjust_privilege( privilege, TRUE, FALSE, &was_enabled );
		if ( status ) {
			logging::print( "[system] adjust_privilege: Could not acquire privilege" );
			return false;
		}

		return true;
	}

	void open_file( const std::string& file, std::vector<uint8_t>& data ) {
		std::ifstream fstr( file, std::ios::binary );
		fstr.unsetf( std::ios::skipws );
		fstr.seekg( 0, std::ios::end );

		const auto file_size = fstr.tellg( );

		fstr.seekg( NULL, std::ios::beg );
		data.reserve( static_cast< uint32_t >( file_size ) );
		data.insert( data.begin( ), std::istream_iterator<uint8_t>( fstr ), std::istream_iterator<uint8_t>( ) );
	}

	void gen_rnd_str( wchar_t* random_str ) {
		auto length = ( crt::rand( ) % 12 ) + 8;
		for ( auto i = 0ull; i < length; i++ ) {
			switch ( crt::rand( ) % 3 ) {
			case 0:
				random_str[ i ] = static_cast< wchar_t > ( 'A' + crt::rand( ) % 26 );
				break;

			case 1:
				random_str[ i ] = static_cast< wchar_t > ( 'a' + crt::rand( ) % 26 );
				break;

			case 2:
				random_str[ i ] = static_cast< wchar_t > ( '0' + crt::rand( ) % 10 );
				break;
			}
		}

		random_str[ length ] = 0;
	}

	const wchar_t* find_driver_path( ) {
		wchar_t* cmd = GetCommandLineW( );
		int cmd_len = crt::str_len( cmd ) - 1;
		if ( cmd_len == -1 )
			return nullptr;

		if ( cmd[ cmd_len-- ] == L'\"' ) {
			cmd[ cmd_len + 1 ] = 0;
			for ( ; cmd_len >= 0; cmd_len-- ) {
				if ( cmd[ cmd_len ] == L'\"' ) {
					cmd[ cmd_len++ ] = 0;
					return &cmd[ cmd_len ];
				}
			}
		}
		else {
			for ( ; cmd_len >= 0; cmd_len-- ) {
				if ( cmd[ cmd_len ] == L' ' ) {
					return &cmd[ ++cmd_len ];
				}
			}
		}

		return nullptr;
	}
}