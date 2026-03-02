#pragma once

namespace utility {
    std::wstring to_lower( std::wstring str ) {
        std::transform( str.begin( ), str.end( ), str.begin( ), ::towlower );
        return str;
    }

    std::wstring ansi_to_wstring( const std::string& input, DWORD locale = 0 ) {
        if ( input.empty( ) ) return std::wstring( );

        int size_needed = MultiByteToWideChar( locale, 0, input.c_str( ), ( int )input.length( ), nullptr, 0 );
        std::wstring result( size_needed, 0 );
        MultiByteToWideChar( locale, 0, input.c_str( ), ( int )input.length( ), &result[ 0 ], size_needed );
        return result;
    }

    std::string wstring_to_ansi( const std::wstring& input, DWORD locale = 0 ) {
        if ( input.empty( ) ) return std::string( );

        int size_needed = WideCharToMultiByte( locale, 0, input.c_str( ), ( int )input.length( ), nullptr, 0, nullptr, nullptr );
        std::string result( size_needed, 0 );
        WideCharToMultiByte( locale, 0, input.c_str( ), ( int )input.length( ), &result[ 0 ], size_needed, nullptr, nullptr );
        return result;
    }

    std::wstring get_exe_directory( ) {
        wchar_t path[ MAX_PATH ];
        GetModuleFileNameW( nullptr, path, MAX_PATH );
        std::wstring exe_path( path );
        return exe_path.substr( 0, exe_path.find_last_of( L"\\" ) );
    }

    std::wstring strip_path( const std::wstring& path ) {
        size_t pos = path.find_last_of( L"\\/" );
        if ( pos != std::wstring::npos )
            return path.substr( pos + 1 );
        return path;
    }

    bool file_exists( const std::wstring& path ) {
        auto attrib = GetFileAttributesW( path.c_str( ) );
        return ( attrib != INVALID_FILE_ATTRIBUTES && !( attrib & FILE_ATTRIBUTE_DIRECTORY ) );
    }

    std::wstring get_process_directory( DWORD pid ) {
        HANDLE snapshot;
        MODULEENTRY32W mod = { sizeof( MODULEENTRY32W ), 0 };
        std::wstring path = L"";

        if ( ( snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pid ) ) &&
            Module32FirstW( snapshot, &mod ) != FALSE
            ) {
            path = mod.szExePath;
            path = path.substr( 0, path.rfind( L"\\" ) );
        }

        return path;
    }
}