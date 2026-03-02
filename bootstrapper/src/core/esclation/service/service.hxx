
namespace service
{
    
    bool load_driver_privilage( bool enabled ) {
        TOKEN_PRIVILEGES privilege{};
        privilege.PrivilegeCount = 1;

        if ( !LookupPrivilegeValueA(
            nullptr,
            oxorany( "SeLoadDriverPrivilege" ),
            &privilege.Privileges[ 0 ].Luid ) ) {
            logging::print( oxorany( "[c_service] load_driver_privilage: LookupPrivilegeValue failed: 0x%x" ), GetLastError( ) );
            return false;
        }

        HANDLE token;
        if ( !OpenProcessToken(
            GetCurrentProcess( ),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &token ) ) {
            logging::print( oxorany( "[c_service] load_driver_privilage: OpenProcessToken failed: 0x%x" ), GetLastError( ) );
            return false;
        }

        privilege.Privileges[ 0 ].Attributes = enabled ? SE_PRIVILEGE_ENABLED : 0;

        auto result = AdjustTokenPrivileges(
            token,
            FALSE,
            &privilege,
            sizeof( privilege ),
            nullptr,
            nullptr
        );

        if ( !result || GetLastError( ) == ERROR_NOT_ALL_ASSIGNED ) {
            result = AdjustTokenPrivileges(
                token,
                TRUE,
                &privilege,
                sizeof( privilege ),
                nullptr,
                nullptr
            );
        }

        NtClose( token );

        if ( !result || GetLastError( ) == ERROR_NOT_ALL_ASSIGNED ) {
            logging::print( oxorany( "[c_service] load_driver_privilage: AdjustTokenPrivileges failed: 0x%x" ), GetLastError( ) );
            return false;
        }

        TOKEN_PRIVILEGES verify_privilege{ };
        DWORD length;
        if ( GetTokenInformation( token, TokenPrivileges, &verify_privilege, sizeof( verify_privilege ), &length ) ) {
            logging::print( oxorany( "[c_service] load_driver_privilage: Privilege state after adjustment: 0x%x" ),
                            verify_privilege.Privileges[ 0 ].Attributes );
        }

        return true;
    }

    
    bool create_service( const wchar_t* service_name, const wchar_t* registry_path ) {
        if ( !service_name || !registry_path ) {
            logging::print( oxorany( "[c_service] create_service: Invalid service name or registry path" ) );
            return false;
        }

        HKEY service_key = nullptr;
        auto result = RegOpenKeyA(
            HKEY_LOCAL_MACHINE,
            oxorany( "system\\CurrentControlSet\\Services" ),
            &service_key
        );

        if ( result != ERROR_SUCCESS ) {
            logging::print( oxorany( "[c_service] create_service: Could not open services key: %d" ), result );
            return false;
        }

        HKEY service_config_key = nullptr;
        result = RegCreateKeyW(
            service_key,
            service_name,
            &service_config_key
        );

        if ( result != ERROR_SUCCESS ) {
            RegCloseKey( service_key );
            logging::print( oxorany( "[c_service] create_service: Could not create service key: %d" ), result );
            return false;
        }

        const auto path_len = ( crt::str_len( registry_path ) + 1 ) << 1;
        result = RegSetValueExW(
            service_config_key,
            oxorany( L"ImagePath" ),
            0,
            REG_EXPAND_SZ,
            reinterpret_cast< const std::uint8_t* >( registry_path ),
            path_len
        );

        if ( result != ERROR_SUCCESS ) {
            RegCloseKey( service_config_key );
            RegCloseKey( service_key );
            logging::print( oxorany( "[c_service] create_service: Could not set ImagePath: %d" ), result );
            return false;
        }

        const DWORD type = 1;
        result = RegSetValueExA(
            service_config_key,
            oxorany( "Type" ),
            0,
            REG_DWORD,
            reinterpret_cast< const std::uint8_t* >( &type ),
            sizeof( type )
        );

        RegCloseKey( service_config_key );
        RegCloseKey( service_key );

        if ( result != ERROR_SUCCESS ) {
            logging::print( oxorany( "[c_service] create_service: Could not set Type: %d" ), result );
            return false;
        }

        return true;
    }

    
    UNICODE_STRING build_driver_path( const wchar_t* service_name ) {
        if ( !service_name || service_name[ 0 ] == L'\0' ) {
            return {};
        }

        static wchar_t registry_path[ 80 ]{};
        crt::mem_zero( registry_path, sizeof( registry_path ) );
        crt::str_cat( registry_path, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" );
        crt::str_cat( registry_path, service_name );

        UNICODE_STRING driver_path{};
        driver_path.Length = USHORT( crt::str_len( registry_path ) << 1 );
        driver_path.Buffer = registry_path;
        driver_path.MaximumLength = driver_path.Length + 2;

        return driver_path;
    }

    
    bool uninstall_service( const wchar_t* service_name ) {
        if ( !service_name || service_name[ 0 ] == L'\0' ) {
            logging::print( oxorany( "[c_service] uninstall_service: Invalid service name" ) );
            return false;
        }

        wchar_t registry_path[ MAX_PATH ]{};
        const wchar_t* prefix = oxorany( L"\\??\\" );
        crt::str_cat( registry_path, prefix );

        auto file_path = &registry_path[ 4 ];
        if ( GetTempPathW( MAX_PATH - 4, file_path ) == 0 ) {
            logging::print( oxorany( "[c_service] uninstall_service: Could not get temp path" ) );
            return false;
        }

        crt::str_cat( file_path, service_name );
        crt::str_cat( file_path, oxorany( L".log" ) );

        using nt_unload_driver_t = NTSTATUS( __stdcall* )( PUNICODE_STRING );
        auto* nt_unload_drv = reinterpret_cast< nt_unload_driver_t >(
            GetProcAddress(
                GetModuleHandleA( oxorany( "ntdll.dll" ) ),
                oxorany( "NtUnloadDriver" )
            )
            );

        if ( !nt_unload_drv ) {
            logging::print( oxorany( "[c_service] uninstall_service: Could not get NtUnloadDriver" ) );
            return false;
        }

        auto driver_nt_name = build_driver_path( service_name );
        if ( !driver_nt_name.Buffer ) {
            logging::print( oxorany( "[c_service] uninstall_service: Could not build driver path" ) );
            return false;
        }

        const auto status = nt_unload_drv( &driver_nt_name );
        if ( status == 0xc0000034 ) {

        }
        else if ( status != 0 ) {
            return false;
        }

        HKEY services_key = nullptr;
        auto result = RegOpenKeyA(
            HKEY_LOCAL_MACHINE,
            oxorany( "SYSTEM\\CurrentControlSet\\Services" ),
            &services_key
        );

        if ( result != ERROR_SUCCESS ) {
            logging::print( oxorany( "[c_service] uninstall_service: Could not open services key: %d" ), result );
            return false;
        }

        HKEY service_key = nullptr;
        wchar_t image_path[ MAX_PATH ] = { 0 };
        bool have_image_path = false;

        result = RegOpenKeyW( services_key, service_name, &service_key );
        if ( result == ERROR_SUCCESS ) {
            DWORD data_type = REG_NONE;
            DWORD data_size = sizeof( image_path );
            result = RegQueryValueExW( service_key, L"ImagePath", nullptr, &data_type, reinterpret_cast< LPBYTE >( image_path ), &data_size );
            if ( result == ERROR_SUCCESS && ( data_type == REG_SZ || data_type == REG_EXPAND_SZ ) ) {
                have_image_path = true;
                if ( data_type == REG_EXPAND_SZ ) {
                    wchar_t expanded_path[ MAX_PATH ];
                    if ( ExpandEnvironmentStringsW( image_path, expanded_path, MAX_PATH ) != 0 ) {
                        wcscpy_s( image_path, expanded_path );
                    }
                }
            }
            RegCloseKey( service_key );
        }

        result = RegDeleteKeyW( services_key, service_name );
        RegCloseKey( services_key );

        if ( result != ERROR_SUCCESS ) {
            logging::print( oxorany( "[c_service] uninstall_service: Could not delete service key: %d" ), result );
            return false;
        }

        return true;
    }

    bool uninstall_services( ) {
        HKEY services_key = nullptr;
        if ( RegOpenKeyA(
            HKEY_LOCAL_MACHINE,
            oxorany( "SYSTEM\\CurrentControlSet\\Services" ),
            &services_key ) != ERROR_SUCCESS ) {
            logging::print( oxorany( "[c_service] uninstall_services: Could not open services key" ) );
            return false;
        }

        DWORD index = 0;
        wchar_t service_name[ MAX_PATH ];

        while ( !RegEnumKeyW( services_key, index, service_name, MAX_PATH ) ) {
            HKEY service_key = nullptr;
            if ( !RegOpenKeyW( services_key, service_name, &service_key ) ) {
                wchar_t image_path[ MAX_PATH ];
                DWORD path_size = sizeof( image_path );
                DWORD type = REG_EXPAND_SZ;

                if ( RegQueryValueExW( service_key, oxorany( L"ImagePath" ), nullptr, &type,
                                       reinterpret_cast< LPBYTE >( image_path ), &path_size ) == ERROR_SUCCESS ) {
                    if ( wcsstr( image_path, oxorany( L"\\??\\" ) ) != nullptr &&
                         wcsstr( image_path, oxorany( L".log" ) ) != nullptr ) {
                        uninstall_service( service_name );
                    }
                }
                RegCloseKey( service_key );
            }
            index++;
        }

        RegCloseKey( services_key );
        return true;
    }

    
    bool install_service( const wchar_t* service_name ) {
        logging::print( oxorany( "[c_service] install_service: Installing service: %ws" ), service_name );

        if ( !service_name || service_name[ 0 ] == L'\0' ) {
            logging::print( oxorany( "[c_service] install_service: Invalid service name" ) );
            return false;
        }

        wchar_t registry_path[ MAX_PATH ]{};
        const wchar_t* prefix = oxorany( L"\\??\\" );
        crt::str_cat( registry_path, prefix );

        auto file_path = &registry_path[ 4 ];
        if ( GetTempPathW( MAX_PATH - 4, file_path ) == 0 ) {
            logging::print( oxorany( "[c_service] install_service: Could not get temp path" ) );
            return false;
        }

        crt::str_cat( file_path, service_name );
        crt::str_cat( file_path, oxorany( L".log" ) );

        if ( !create_service( service_name, registry_path ) ) {
            logging::print( oxorany( "[c_service] install_service: Could not register kernel driver" ) );
            return false;
        }

        if ( !MoveFileExW( file_path, nullptr, MOVEFILE_DELAY_UNTIL_REBOOT ) ) {
            logging::print( oxorany( "[c_service] install_service: Could not schedule file deletion" ) );
            return false;
        }

        HANDLE file_handle = CreateFileW(
            file_path,
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_TEMPORARY,
            nullptr
        );

        if ( file_handle == INVALID_HANDLE_VALUE ) {
            logging::print( oxorany( "[c_service] install_service: Could not create file" ) );
            return false;
        }

        DWORD bytes_written = 0;
        if ( !WriteFile(
            file_handle,
            driver_bytes,
            sizeof( driver_bytes ),
            &bytes_written,
            nullptr
        ) || bytes_written != sizeof( driver_bytes ) ) {
            CloseHandle( file_handle );
            logging::print( oxorany( "[c_service] install_service: Could not write driver bytes" ) );
            return false;
        }

        CloseHandle( file_handle );

        auto driver_nt_name = build_driver_path( service_name );
        if ( !driver_nt_name.Buffer ) {
            logging::print( oxorany( "[c_service] install_service: Could not build driver path" ) );
            return false;
        }

        auto* nt_load_drv = reinterpret_cast< nt_load_driver_t >(
            GetProcAddress(
                GetModuleHandleA( oxorany( "ntdll.dll" ) ),
                oxorany( "NtLoadDriver" ) )
            );

        if ( !nt_load_drv ) {
            logging::print( oxorany( "[c_service] install_service: Could not get NtLoadDriver" ) );
            return false;
        }

        auto status = nt_load_drv( &driver_nt_name );
        if ( status == 0xc0000035 ) {
            logging::print( oxorany( "[c_service] install_service: Found existing service, starting service uninstallation.\n" ) );
            uninstall_services( );
            logging::print( oxorany( "[c_service] install_service: Please restart the application." ) );
            return false;
        }

        if ( status != 0 ) {
            logging::print( oxorany( "[c_service] install_service: NtLoadDriver failed: %x" ), status );
            return false;
        }

        logging::print( oxorany( "[c_service] install_service: Successfully installed service" ) );
        return true;
    }
}