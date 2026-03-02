#pragma once

namespace pdb {
    class ComInitializer {
    public:
        ComInitializer( ) {
            HRESULT hr = CoInitialize( NULL );
            initialized = SUCCEEDED( hr );
        }
        ~ComInitializer( ) {
            if ( initialized ) {
                CoUninitialize( );
            }
        }
    private:
        bool initialized;
    };

    static ComInitializer g_com_init;

    struct pdb_info {
        DWORD signature;
        GUID guid;
        DWORD age;
        char pdb_file_name[ 1 ];
    };

    class c_pdb {
    public:
        c_pdb( const char* module_name ) : m_module_name( module_name ) {
            m_pe_path = get_pe_path( module_name );
        }

        ~c_pdb( ) {
            cleanup( );
        }

        bool load( ) {
            m_pdb_path = download_pdb( m_pe_path );
            if ( m_pdb_path.empty( ) ) {
                logging::print( oxorany( "[c_pdb] load: Failed to download PDB" ) );
                return false;
            }

            logging::print( oxorany( "[c_pdb] load: PDB downloaded" ) );

            CComPtr<IDiaDataSource> pSource;
            HRESULT hr = CoCreateInstance( CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER,
                IID_IDiaDataSource, ( void** )&pSource );

            if ( FAILED( hr ) ) {
                const wchar_t* dia_versions[ ] = { L"msdia140.dll", L"msdia120.dll", L"msdia110.dll" };

                for ( const auto& dll : dia_versions ) {
                    hr = NoRegCoCreate( dll, CLSID_DiaSource, IID_IDiaDataSource,
                        ( void** )&pSource );
                    if ( SUCCEEDED( hr ) ) {
                        logging::print( oxorany( "[c_pdb] load: Using %S" ), dll );
                        break;
                    }
                }

                if ( FAILED( hr ) ) {
                    logging::print( oxorany( "[c_pdb] load: Failed to create DIA data source: 0x%08x" ), hr );
                    return false;
                }
            }

            std::wstring wide_path( m_pdb_path.begin( ), m_pdb_path.end( ) );
            hr = pSource->loadDataFromPdb( wide_path.c_str( ) );
            if ( FAILED( hr ) ) {
                logging::print( oxorany( "[c_pdb] load: Failed to load PDB data: 0x%08x" ), hr );
                return false;
            }

            CComPtr<IDiaSession> pSession;
            hr = pSource->openSession( &pSession );
            if ( FAILED( hr ) ) {
                logging::print( oxorany( "[c_pdb] load: Failed to open DIA session: 0x%08x" ), hr );
                return false;
            }

            CComPtr<IDiaSymbol> pGlobal;
            hr = pSession->get_globalScope( &pGlobal );
            if ( FAILED( hr ) ) {
                logging::print( oxorany( "[c_pdb] load: Failed to get global scope: 0x%08x" ), hr );
                return false;
            }

            m_pSource = pSource.Detach( );
            m_pSession = pSession.Detach( );
            m_pGlobal = pGlobal.Detach( );

            if ( !enumerate_symbols( ) ) {
                logging::print( oxorany( "[c_pdb] load: Failed to enumerate symbols" ) );
                cleanup( );
                return false;
            }

            logging::print( oxorany( "[c_pdb] load: Loaded %zu symbols from %s\n" ),
                m_symbols.size( ), m_module_name.c_str( ) );

            return true;
        }

        std::uint64_t get_symbol_address( const char* symbol_name ) {
            for ( const auto& symbol : m_symbols ) {
                if ( symbol.first == symbol_name ) {
                    return symbol.second;
                }
            }
            return 0;
        }

        std::uint64_t get_symbol_rva( const char* symbol_name ) {
            return get_symbol_address( symbol_name );
        }

    private:
        std::string m_module_name;
        std::string m_pe_path;
        std::string m_pdb_path;
        std::vector<std::pair<std::string, std::uint64_t>> m_symbols;

        IDiaDataSource* m_pSource = nullptr;
        IDiaSession* m_pSession = nullptr;
        IDiaSymbol* m_pGlobal = nullptr;

        std::string get_pe_path( const char* module_name ) {
			return std::string( std::getenv( "systemroot" ) ) + "\\System32\\" + module_name;
        }

        std::string download_pdb( const std::string& pe_path ) {
            if ( pe_path.empty( ) ) {
                logging::print( oxorany( "[c_pdb] download_pdb: PE path is empty" ) );
                return "";
            }

            char sz_download_dir[ MAX_PATH ] = { 0 };
            if ( !GetCurrentDirectoryA( sizeof( sz_download_dir ), sz_download_dir ) ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Failed to get current directory" ) );
                return "";
            }

            std::string download_path = sz_download_dir;
            if ( download_path[ download_path.size( ) - 1 ] != '\\' ) {
                download_path += "\\";
            }

            std::ifstream file( pe_path, std::ios::binary | std::ios::ate );
            if ( !file.is_open( ) ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Failed to open PE file" ) );
                return "";
            }

            auto size = file.tellg( );
            if ( size <= 0 ) {
                logging::print( oxorany( "[c_pdb] download_pdb: PE file is empty" ) );
                return "";
            }

            file.seekg( 0, std::ios::beg );
            std::vector<char> buffer( size );
            if ( !file.read( buffer.data( ), size ) ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Failed to read PE file" ) );
                return "";
            }

            if ( buffer.size( ) < sizeof( IMAGE_DOS_HEADER ) ) {
                logging::print( oxorany( "[c_pdb] download_pdb: File too small" ) );
                return "";
            }

            auto p_dos = reinterpret_cast< IMAGE_DOS_HEADER* >( buffer.data( ) );
            if ( p_dos->e_magic != IMAGE_DOS_SIGNATURE ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Invalid DOS signature" ) );
                return "";
            }

            if ( buffer.size( ) < p_dos->e_lfanew + sizeof( IMAGE_NT_HEADERS ) ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Invalid PE structure" ) );
                return "";
            }

            auto p_nt = reinterpret_cast< IMAGE_NT_HEADERS* >( buffer.data( ) + p_dos->e_lfanew );
            if ( p_nt->Signature != IMAGE_NT_SIGNATURE ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Invalid NT signature" ) );
                return "";
            }

            bool is_x86 = ( p_nt->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 );
            auto p_opt32 = reinterpret_cast< IMAGE_OPTIONAL_HEADER32* >( &p_nt->OptionalHeader );
            auto p_opt64 = reinterpret_cast< IMAGE_OPTIONAL_HEADER64* >( &p_nt->OptionalHeader );
            auto image_size = is_x86 ? p_opt32->SizeOfImage : p_opt64->SizeOfImage;

            auto image_buffer = std::make_unique<BYTE[ ]>( image_size );
            if ( !image_buffer ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Failed to allocate memory" ) );
                return "";
            }

            auto headers_size = is_x86 ? p_opt32->SizeOfHeaders : p_opt64->SizeOfHeaders;
            std::memcpy( image_buffer.get( ), buffer.data( ), headers_size );

            auto p_section = IMAGE_FIRST_SECTION( p_nt );
            for ( UINT i = 0; i < p_nt->FileHeader.NumberOfSections; ++i, ++p_section ) {
                if ( p_section->SizeOfRawData ) {
                    std::memcpy( image_buffer.get( ) + p_section->VirtualAddress,
                        buffer.data( ) + p_section->PointerToRawData,
                        p_section->SizeOfRawData );
                }
            }

            IMAGE_DATA_DIRECTORY* p_data_dir = is_x86 ?
                &p_opt32->DataDirectory[ IMAGE_DIRECTORY_ENTRY_DEBUG ] :
                &p_opt64->DataDirectory[ IMAGE_DIRECTORY_ENTRY_DEBUG ];

            if ( !p_data_dir->Size ) {
                logging::print( oxorany( "[c_pdb] download_pdb: No debug directory found" ) );
                return "";
            }

            auto p_debug_dir = reinterpret_cast< IMAGE_DEBUG_DIRECTORY* >(
                image_buffer.get( ) + p_data_dir->VirtualAddress );

            if ( p_debug_dir->Type != IMAGE_DEBUG_TYPE_CODEVIEW ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Not CodeView debug type" ) );
                return "";
            }

            auto pdb_info_ptr = reinterpret_cast< pdb_info* >(
                image_buffer.get( ) + p_debug_dir->AddressOfRawData );

            if ( pdb_info_ptr->signature != 0x53445352 ) { // 'RSDS'
                logging::print( oxorany( "[c_pdb] download_pdb: Invalid PDB signature" ) );
                return "";
            }

            wchar_t w_guid[ 100 ] = { 0 };
            if ( !StringFromGUID2( pdb_info_ptr->guid, w_guid, 100 ) ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Failed to convert GUID" ) );
                return "";
            }

            char a_guid[ 100 ] = { 0 };
            size_t l_guid = 0;
            wcstombs_s( &l_guid, a_guid, w_guid, sizeof( a_guid ) );

            char guid_filtered[ 256 ] = { 0 };
            for ( size_t i = 0; i < l_guid; ++i ) {
                if ( ( a_guid[ i ] >= '0' && a_guid[ i ] <= '9' ) ||
                    ( a_guid[ i ] >= 'A' && a_guid[ i ] <= 'F' ) ||
                    ( a_guid[ i ] >= 'a' && a_guid[ i ] <= 'f' ) ) {
                    guid_filtered[ strlen( guid_filtered ) ] = a_guid[ i ];
                }
            }

            char age[ 16 ] = { 0 };
            sprintf_s( age, "%X", pdb_info_ptr->age );  // Use %X for uppercase hex instead of %d

            std::string url = "https://msdl.microsoft.com/download/symbols/";
            url += pdb_info_ptr->pdb_file_name;
            url += "/";
            url += guid_filtered;
            url += age;
            url += "/";
            url += pdb_info_ptr->pdb_file_name;

            std::string pdb_path = download_path + md5_hash( buffer.data( ), ( ULONG )size ) + ".pdb";
            DeleteFileA( pdb_path.c_str( ) );

            HRESULT hr = URLDownloadToFileA( NULL, url.c_str( ), pdb_path.c_str( ), NULL, NULL );
            if ( FAILED( hr ) ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Download failed: 0x%08x" ), hr );
                return "";
            }

            std::ifstream verify( pdb_path, std::ios::binary | std::ios::ate );
            if ( !verify.is_open( ) || verify.tellg( ) < 1024 ) {
                logging::print( oxorany( "[c_pdb] download_pdb: Downloaded PDB is invalid or too small" ) );
                DeleteFileA( pdb_path.c_str( ) );
                return "";
            }
            verify.close( );

            return pdb_path;
        }

        bool enumerate_symbols( ) {
            if ( !m_pGlobal ) {
                return false;
            }

            CComPtr<IDiaEnumSymbols> pEnumSymbols;
            HRESULT hr = m_pGlobal->findChildren( SymTagNull, NULL, nsNone, &pEnumSymbols );

            if ( FAILED( hr ) ) {
                logging::print( oxorany( "[c_pdb] enumerate_symbols: Failed to enumerate: 0x%08x" ), hr );
                return false;
            }

            CComPtr<IDiaSymbol> pSymbol;
            ULONG celt = 0;
            while ( SUCCEEDED( pEnumSymbols->Next( 1, &pSymbol, &celt ) ) && celt == 1 ) {
                BSTR name;
                if ( SUCCEEDED( pSymbol->get_name( &name ) ) ) {
                    DWORD rva = 0;
                    if ( SUCCEEDED( pSymbol->get_relativeVirtualAddress( &rva ) ) ) {
                        char symbol_name[ 1024 ] = { 0 };
                        size_t converted = 0;
                        wcstombs_s( &converted, symbol_name, name, sizeof( symbol_name ) - 1 );

                        if ( converted > 0 && symbol_name[ 0 ] != '\0' ) {
                            m_symbols.emplace_back( symbol_name, rva );
                        }
                    }
                    SysFreeString( name );
                }
                pSymbol.Release( );
            }

            return m_symbols.size( ) > 0;
        }

        void cleanup( ) {
            if ( m_pGlobal ) {
                m_pGlobal->Release( );
                m_pGlobal = nullptr;
            }
            if ( m_pSession ) {
                m_pSession->Release( );
                m_pSession = nullptr;
            }
            if ( m_pSource ) {
                m_pSource->Release( );
                m_pSource = nullptr;
            }

            if ( !m_pdb_path.empty( ) ) {
                DeleteFileA( m_pdb_path.c_str( ) );
            }
        }

        std::string md5_hash( const char* data, ULONG length ) {
            char hash[ 64 ] = { 0 };
            sprintf_s( hash, "%08x%08x%08x%08x",
                ( UINT )data, length, GetTickCount( ), GetCurrentProcessId( ) );
            return std::string( hash );
        }
    };
}