#pragma once

namespace etw {
    namespace handler {
        bool is_process_name( const char* target_process ) {
            auto process = nt::ps_get_current_process( );
            if ( !nt::mm_is_address_valid( process ) )
                return false;

            auto image_file_name = process::get_image_file_name( process );
            return std::strstr_lowercase( image_file_name, target_process );
        }

        nt_status_t( *nt_allocate_uuids_org )( ularge_integer_t*, unsigned long*, unsigned long*, char* ) = nullptr;
        nt_status_t nt_allocate_uuids_hk( ularge_integer_t* time, unsigned long* range, unsigned long* sequence, char* seed ) {
            if ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) {
                nt::dbg_print( oxorany( "[ETW] blocked mac address\n" ) );
                return nt_status_t::unsuccessful;
            }

            return nt_allocate_uuids_org( time, range, sequence, seed );
        }

        nt_status_t( __stdcall* nt_query_system_information_org )( system_information_class_t, void*, unsigned long, unsigned long* ) = nullptr;
        nt_status_t __stdcall nt_query_system_information_hk( system_information_class_t system_information_class, void* system_information, unsigned long system_information_length, unsigned long* return_length ) {
            switch ( system_information_class ) {
                case 0xA5: {
                    nt::dbg_print( oxorany( "[ETW] SystemIsolatedUserModeInformation called | length: %lu | buffer: %p\n" ),
                        system_information_length, system_information );

                    auto status = nt_query_system_information_org(
                        system_information_class,
                        system_information,
                        system_information_length,
                        return_length
                    );

                    nt::dbg_print( oxorany( "[ETW] original returned: 0x%08X\n" ), status );

                    if ( status == nt_status_t::success && system_information_length == 0x10 && system_information ) {
                        auto info = reinterpret_cast< std::uint8_t* >( system_information );

                        nt::dbg_print( oxorany( "[ETW] pre-patch  | [0x00]=0x%02X [0x01]=0x%02X [0x02]=0x%02X [0x03]=0x%02X [0x04]=0x%02X [0x05]=0x%02X [0x06]=0x%02X\n" ),
                            info[ 0x00 ], info[ 0x01 ], info[ 0x02 ], info[ 0x03 ], info[ 0x04 ], info[ 0x05 ], info[ 0x06 ] );

                        //info[ 0x02 ] = 1; // StrongIsolationEnabled
                        //info[ 0x04 ] = 1; // HvciEnabled

                        nt::dbg_print( oxorany( "[ETW] post-patch | [0x00]=0x%02X [0x01]=0x%02X [0x02]=0x%02X [0x03]=0x%02X [0x04]=0x%02X [0x05]=0x%02X [0x06]=0x%02X\n" ),
                            info[ 0x00 ], info[ 0x01 ], info[ 0x02 ], info[ 0x03 ], info[ 0x04 ], info[ 0x05 ], info[ 0x06 ] );
                    }
                    else {
                        nt::dbg_print( oxorany( "[ETW] skipped patch | status=%s length_match=%s buffer=%s\n" ),
                            status == nt_status_t::success ? oxorany( "ok" ) : oxorany( "fail" ),
                            system_information_length == 0x10 ? oxorany( "yes" ) : oxorany( "no" ),
                            system_information ? oxorany( "valid" ) : oxorany( "null" ) );
                    }

                    return status;
                }

                case 0x67: {
                    nt::dbg_print( oxorany( "[ETW] SystemCodeIntegrityInformation called | length: %lu | buffer: %p\n" ),
                        system_information_length, system_information );

                    auto status = nt_query_system_information_org(
                        system_information_class,
                        system_information,
                        system_information_length,
                        return_length
                    );

                    nt::dbg_print( oxorany( "[ETW] original returned: 0x%08X\n" ), status );

                    if ( status == nt_status_t::success && system_information && system_information_length >= 0x08 ) {
                        auto info = reinterpret_cast< std::uint8_t* >( system_information );
                        auto options = reinterpret_cast< std::uint32_t* >( reinterpret_cast< std::uint64_t >( system_information ) + 0x04 );

                        nt::dbg_print( oxorany( "[ETW] pre-patch  | Length=0x%08X CodeIntegrityOptions=0x%08X\n" ),
                            *reinterpret_cast< std::uint32_t* >( info ), *options );

                        *options = ( *options | 0x1401u );

                        nt::dbg_print( oxorany( "[ETW] post-patch | Length=0x%08X CodeIntegrityOptions=0x%08X\n" ),
                            *reinterpret_cast< std::uint32_t* >( info ), *options );
                    }
                    else {
                        nt::dbg_print( oxorany( "[ETW] skipped patch | status=%s length_ok=%s buffer=%s\n" ),
                            status == nt_status_t::success ? oxorany( "ok" ) : oxorany( "fail" ),
                            system_information_length >= 0x08 ? oxorany( "yes" ) : oxorany( "no" ),
                            system_information ? oxorany( "valid" ) : oxorany( "null" ) );
                    }

                    return status;
                }
            }

            return nt_query_system_information_org(
                system_information_class,
                system_information,
                system_information_length,
                return_length
            );
        }

        nt_status_t( __stdcall* nt_device_io_control_file_org )( void*, void*, void*, void*, io_status_block_t*, unsigned long, void*, unsigned long, void*, unsigned long ) = nullptr;
        nt_status_t __stdcall nt_device_io_control_file_hk(
            void* file_handle,
            void* event,
            void* apc_routine,
            void* apc_context,
            io_status_block_t* io_status_block,
            unsigned long io_control_code,
            void* input_buffer,
            unsigned long input_buffer_length,
            void* output_buffer,
            unsigned long output_buffer_length
        ) {
            if ( is_process_name( oxorany( "FortniteClient" ) ) ||
                is_process_name( oxorany( "FortniteLauncher" ) ) ||
                is_process_name( oxorany( "RustClient" ) ) ) {
                file_object_t* file_object = nullptr;
                auto result = nt::ob_reference_object_by_handle(
                    file_handle,
                    0,
                    nt::io_file_object_type( ),
                    0,
                    reinterpret_cast< void** >( &file_object ),
                    nullptr
                );

                if ( !result && file_object ) {
                    __try {
                        auto device_object = file_object->m_device_object;
                        if ( device_object && nt::mm_is_address_valid( device_object ) ) {
                            unsigned long return_length = 0;
                            object_name_information_t* device_name = nullptr;
                            nt::ob_query_name_string( device_object, device_name, 0, &return_length );

                            if ( return_length ) {
                                device_name = reinterpret_cast< object_name_information_t* >( mmu::alloc_page( return_length ) );
                                result = nt::ob_query_name_string( device_object, device_name, return_length, &return_length );

                                if ( !result ) {
                                    unicode_string_t target_device;
                                    nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\Harddisk0\\DR0" ) );
                                    if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                        nt::dbg_print( oxorany( "[ETW] Blocked dr0 access\n" ) );
                                        return nt_status_t::unsuccessful;
                                    }

                                    nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\NTPNP_PCI0002" ) );
                                    if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                        nt::dbg_print( oxorany( "[ETW] blocked pci access\n" ) );
                                        return nt_status_t::unsuccessful;
                                    }

                                    nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\NvAdminDevice" ) );
                                    if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                        nt::dbg_print( oxorany( "[ETW] blocked nividia access\n" ) );
                                        return nt_status_t::unsuccessful;
                                    }

                                    nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\USBPDO-0" ) );
                                    if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                        nt::dbg_print( oxorany( "[ETW] blocked usbpdo0 access\n" ) );
                                        return nt_status_t::unsuccessful;
                                    }

                                    nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\USBPDO-2" ) );
                                    if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                        nt::dbg_print( oxorany( "[ETW] blocked usbpdo2 access\n" ) );
                                        return nt_status_t::unsuccessful;
                                    }

                                    //                          nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\MountPointManager" ) );
                                    //                          if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                                                  //nt::dbg_print( oxorany( "[ETW] Blocked MountPointManager access\n" ) );
                                    //                              return nt_status_t::unsuccessful;
                                    //                          }

                                                              nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\Harddisk" ) );
                                                              if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                                                  nt::dbg_print( oxorany( "[ETW] blocked harddisk access\n" ) );
                                                                  return nt_status_t::unsuccessful;
                                                              }

                                    //                          nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\DeviceApi" ) );
                                    //                          if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                                                  //nt::dbg_print( oxorany( "[ETW] Blocked DeviceApi access\n" ) );
                                    //                              return nt_status_t::unsuccessful;
                                    //                          }

                                    nt::rtl_init_unicode_string( &target_device, oxorany( L"\\Device\\EasyAntiCheat_EOS" ) );
                                    if ( !nt::rtl_compare_unicode_string( &device_name->name, &target_device, true ) ) {
                                        //nt::dbg_print( oxorany( "[ETW] eac device: ioctl=0x%x input=0x%x output=0x%x\n" ),
                                        //    io_control_code, input_buffer_length, output_buffer_length );

                                        /*
                                        [ETW] eac device: ioctl=0x22e023 input=0x2c output=0x2c
                                        [ETW] eac device: ioctl=0x226013 input=0x0 output=0x4
                                        [ETW] eac device: ioctl=0x22e017 input=0x330 output=0x330
                                        [ETW] eac device: ioctl=0x22e067 input=0x4 output=0x250
                                        [ETW] eac device: ioctl=0x22e04f input=0x14 output=0x14
                                        [ETW] eac device: ioctl=0x22605f input=0x0 output=0x1000
                                        [ETW] eac device: ioctl=0x22e06b input=0x0 output=0x80010
                                        [ETW] eac device: ioctl=0x22e067 input=0x4 output=0x250
                                        [ETW] eac device: ioctl=0x22e00f input=0x10 output=0x10
                                        [ETW] eac device: ioctl=0x22e057 input=0x0 output=0x4
                                        */

                                        if ( io_control_code == 0x22605f ) {
                                            nt::dbg_print( oxorany( "[ETW] blocked EAC ioctl 0x22605f\n" ) );
                                            return nt_status_t::unsuccessful;
                                        }

                                        if ( io_control_code == 0x22e067 ) {
                                            nt::dbg_print( oxorany( "[ETW] blocked EAC ioctl 0x22e067\n" ) );
                                            return nt_status_t::unsuccessful;
                                        }

                                        if ( io_control_code == 0x22e00f ) {
                                            if ( output_buffer && input_buffer && input_buffer_length ) {
                                                nt::dbg_print( oxorany( "[ETW] blocked EAC ioctl 0x22e00f\n" ) );
												std::memcpy( output_buffer, input_buffer, input_buffer_length );
                                                return nt_status_t::success;
											}
                                        }

                                        if ( io_control_code == 0x22e04f ) {
                                            if ( output_buffer && input_buffer && input_buffer_length ) {
                                                nt::dbg_print( oxorany( "[ETW] blocked EAC ioctl 0x22e04f\n" ) );
                                                std::memcpy( output_buffer, input_buffer, input_buffer_length );
                                                return nt_status_t::success;
                                            }
                                        }

                                        //if ( io_control_code == 0x22e023 ) {
                                        //    if ( output_buffer && input_buffer && input_buffer_length ) {
                                        //        nt::dbg_print( oxorany( "[ETW] blocked EAC ioctl 0x22e023\n" ) );
                                        //        std::memcpy( output_buffer, input_buffer, input_buffer_length );
                                        //        return nt_status_t::success;
                                        //    }
                                        //}
                                    }

                                    //return status;
                                //}

                                //nt::dbg_print( oxorany( "[ETW] Device name: %wZ\n" ), &device_name->name );
                                }

                                //mmu::free_page( reinterpret_cast< std::uint64_t >( name_info ), return_length );
                            }
                        }
                    }
                    __except ( 1 ) { }

                    nt::ob_dereference_object( file_object );
                }
            }

            return nt_device_io_control_file_org(
                file_handle,
                event,
                apc_routine,
                apc_context,
                io_status_block,
                io_control_code,
                input_buffer,
                input_buffer_length,
                output_buffer,
                output_buffer_length
            );
        }

        nt_status_t( *nt_query_license_value_org )(
            unicode_string_t*,
            std::uint32_t*,
            void*,
            std::uint32_t,
            std::uint32_t*
            ) = nullptr;

        nt_status_t nt_query_license_value_hk(
            unicode_string_t* value_name,
            std::uint32_t* type,
            void* data,
            std::uint32_t data_size,
            std::uint32_t* result_size
        ) {
            auto result = nt_query_license_value_org(
                value_name,
                type,
                data,
                data_size,
                result_size
            );

            if ( !result && ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) ) {

                if ( value_name && nt::mm_is_address_valid( value_name ) ) {
                    if ( value_name->m_buffer && nt::mm_is_address_valid( value_name->m_buffer ) ) {
                        return nt_status_t::unsuccessful;
                    }
                }
            }

            return result;
        }

        nt_status_t( *nt_query_volume_information_file_org )(
            void*,
            io_status_block_t*,
            void*,
            std::uint32_t,
            fs_information_class_t
            ) = nullptr;

        volatile std::uint64_t m_last_volume_time = 0;
        nt_status_t nt_query_volume_information_file_hk(
            void* file_handle,
            io_status_block_t* io_status_block,
            void* fs_information,
            std::uint32_t length,
            fs_information_class_t fs_information_class
        ) {
            auto result = nt_query_volume_information_file_org(
                file_handle,
                io_status_block,
                fs_information,
                length,
                fs_information_class
            );

            if ( is_process_name( oxorany( "FortniteLaunch" ) ) || !m_last_volume_time ) {
                m_last_volume_time = nt::ke_query_system_time_precise( );
            }

            if ( !result && ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) ) {

                switch ( fs_information_class ) {
                    case fs_information_class_t::fs_volume_information:
                    {
                        auto vol_info = reinterpret_cast< file_fs_volume_information_t* >( fs_information );
                        if ( vol_info && length >= sizeof( file_fs_volume_information_t ) ) {
                            auto original_serial = vol_info->m_volume_serial_number;
                            auto tick_count = nt::ke_query_tick_count( );
                            auto seed = static_cast< unsigned long >( tick_count.m_low_part );
                            seed = nt::rtl_random_ex( &seed );

                            auto time_now = nt::ke_query_system_time_precise( );
                            auto last_reset = m_last_volume_time;
                            const auto one_minute = 1 * 60 * 10000000;

                            if ( last_reset != 0 && ( time_now - last_reset ) >= one_minute ) {
                                vol_info->m_volume_serial_number = ( seed & 0xFFFF0000 ) | ( ( seed * 1103515245 + 12345 ) & 0xFFFF );

                                nt::dbg_print(
                                    oxorany( "[ETW] blocked volume serial: 0x%08X -> 0x%08X\n" ),
                                    original_serial, vol_info->m_volume_serial_number
                                );
                            }
                        }
                    } break;
                }
            }

            return result;
        }

        volatile std::uint64_t m_last_sid_time = 0;
        nt_status_t( *nt_query_information_token_org )( void*, std::uint8_t, void*, unsigned long, unsigned long* ) = nullptr;
        nt_status_t nt_query_information_token_hk(
            void* token_handle,
            std::uint8_t token_info_class,
            void* token_info,
            unsigned long token_info_length,
            unsigned long* return_length
        ) {
            auto result = nt_query_information_token_org(
                token_handle,
                token_info_class,
                token_info,
                token_info_length,
                return_length
            );

            if ( is_process_name( oxorany( "FortniteLaunch" ) ) || !m_last_sid_time ) {
                m_last_sid_time = nt::ke_query_system_time_precise( );
            }

            if ( !result && ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "SidCheck" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) ) {

                if ( token_info_class == 1 ) {
                    auto token_user = reinterpret_cast< token_user_t* >( token_info );
                    auto user_sid = token_user->m_user.m_sid;

                    if ( nt::mm_is_address_valid( user_sid ) ) {
                        std::uint8_t sid_buffer[ 68 ]{ 0 };
                        auto sid_size = nt::rtl_length_sid( user_sid );

                        if ( sid_size <= sizeof( sid_buffer ) ) {
                            std::memcpy( sid_buffer, user_sid, sid_size );

                            const auto generate_sid = [ ] ( sid_internal_t* sid_internal ) {
                                auto tick_count = nt::ke_query_tick_count( );
                                auto seed = static_cast< unsigned long >( tick_count.m_low_part );
                                seed = nt::rtl_random_ex( &seed );

                                sid_internal->m_revision = 1;
                                sid_internal->m_identifier_authority.m_value[ 0 ] = 0;
                                sid_internal->m_identifier_authority.m_value[ 1 ] = 0;
                                sid_internal->m_identifier_authority.m_value[ 2 ] = 0;
                                sid_internal->m_identifier_authority.m_value[ 3 ] = 0;
                                sid_internal->m_identifier_authority.m_value[ 4 ] = 0;
                                sid_internal->m_identifier_authority.m_value[ 5 ] = 5;

                                sid_internal->m_sub_authority_count = 5;
                                sid_internal->m_sub_authority[ 0 ] = 21;
                                sid_internal->m_sub_authority[ 1 ] = ( seed * 1103515245 + 12345 ) & 0x7FFFFFFF;
                                sid_internal->m_sub_authority[ 2 ] = ( seed * 1664525 + 1013904223 ) & 0x7FFFFFFF;
                                sid_internal->m_sub_authority[ 3 ] = ( seed * 214013 + 2531011 ) & 0x7FFFFFFF;
                                sid_internal->m_sub_authority[ 4 ] = 1000 + ( seed % 10000 );
                                };

                            generate_sid( reinterpret_cast< sid_internal_t* >( sid_buffer ) );

                            auto time_now = nt::ke_query_system_time_precise( );
                            auto last_reset = m_last_sid_time;
                            const auto one_minute = 1 * 60 * 10000000;

                            if ( last_reset != 0 && ( time_now - last_reset ) >= one_minute ) {
                                std::memcpy( user_sid, sid_buffer, sid_size );

                                unicode_string_t sid_string;
                                if ( !nt::rtl_convert_sid_to_unicode_string( &sid_string, ( sid_t* )sid_buffer, true ) ) {
                                    nt::dbg_print( oxorany( "[ETW] blocked sid: %wZ\n" ), &sid_string );
                                }
                            }
                        }
                    }
                }
            }

            return result;
        }

        nt_status_t( *nt_trace_control_org )(
            unsigned long,
            void*,
            unsigned long,
            void*,
            unsigned long,
            unsigned long* ) = nullptr;
        nt_status_t nt_trace_control_hk(
            unsigned long function_code,
            void* in_buffer,
            unsigned long in_buffer_len,
            void* out_buffer,
            unsigned long out_buffer_len,
            unsigned long* return_size ) {
            if ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) {

                switch ( function_code ) {
                    case 30:
                    {
                        nt::dbg_print( oxorany( "[ETW] Blocked ETW Start (code: %d)\n" ), function_code );
                        return nt_status_t::unsuccessful;
                    } break;

                    //case 15:
                    //{
                    //    nt::dbg_print( oxorany( "[ETW] Blocked ETW Stop (code: %d)\n" ), function_code );
                    //    return nt_status_t::success;
                    //} break;

                    default:
                    {
                        nt::dbg_print( oxorany( "[ETW] NtTraceControl (code: %d)\n" ), function_code );
                    } break;
                }

                nt::dbg_print( oxorany( "[ETW] NtTraceControl (Function Code: %d)\n" ), function_code );
            }

            return nt_trace_control_org(
                function_code,
                in_buffer,
                in_buffer_len,
                out_buffer,
                out_buffer_len,
                return_size
            );
        }

        nt_status_t( __stdcall* nt_query_information_process_org )( void*, int, void*, unsigned long, unsigned long* ) = nullptr;
        nt_status_t __stdcall nt_query_information_process_hk(
            void* process_handle,
            int process_info_class,
            void* process_info,
            unsigned long process_info_length,
            unsigned long* return_length
        ) {
            if ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) {

                nt::dbg_print( oxorany( "[ETW] NtQueryInformationProcess (class: %d)\n" ), process_info_class );
            }

            return nt_query_information_process_org(
                process_handle,
                process_info_class,
                process_info,
                process_info_length,
                return_length
            );
        }

        struct key_value_partial_information_t {
            unsigned long m_title_index;
            unsigned long m_type;
            unsigned long m_data_length;
            unsigned char m_data[ 1 ];
        };

        nt_status_t( __stdcall* nt_query_value_key_org )( void*, unicode_string_t*, int, void*, unsigned long, unsigned long* ) = nullptr;
        nt_status_t __stdcall nt_query_value_key_hk(
            void* key_handle,
            unicode_string_t* value_name,
            int key_value_info_class,
            void* key_value_info,
            unsigned long key_value_info_length,
            unsigned long* result_length
        ) {
            auto result = nt_query_value_key_org(
                key_handle,
                value_name,
                key_value_info_class,
                key_value_info,
                key_value_info_length,
                result_length
            );

            if ( !result && ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) ) {

                if ( value_name && nt::mm_is_address_valid( value_name ) ) {
                    if ( value_name->m_buffer && nt::mm_is_address_valid( value_name->m_buffer ) ) {
                        wchar_t buffer[ 256 ] = {};
                        auto copy_length = value_name->m_length < sizeof( buffer ) - 2 ? value_name->m_length : sizeof( buffer ) - 2;
                        std::memcpy( buffer, value_name->m_buffer, copy_length );

                        if ( key_value_info_class == 2 &&
                             key_value_info &&
                             key_value_info_length >= sizeof( key_value_partial_information_t ) ) {

                            bool is_hwid_value = false;
                            if ( std::wcsstr_lowercase( buffer, oxorany( L"machineguid" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"uuid" ) ) ) {
                                is_hwid_value = true;
                            }

                            if ( std::wcsstr_lowercase( buffer, oxorany( L"rmgpuid" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"videoid" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"rpcid" ) ) ) {
                                is_hwid_value = true;
                            }

                            if ( buffer[ 0 ] == oxorany( L'{' ) ) {
                                auto closing_brace = wcschr( buffer, oxorany( L'}' ) );
                                if ( closing_brace && closing_brace[ 1 ] == oxorany( L',' ) ) {
                                    is_hwid_value = true;
                                }
                            }

                            if ( std::wcsstr_lowercase( buffer, oxorany( L"wavemapper" ) ) ) {
                                is_hwid_value = true;
                            }

                            if ( std::wcsstr_lowercase( buffer, oxorany( L"volumeserialnum" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"serialnumber" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"diskid" ) ) ) {
                                is_hwid_value = true;
                            }

                            if ( std::wcsstr_lowercase( buffer, oxorany( L"systemmanu" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"systemproduct" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"biosvendor" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"biosversion" ) ) ||
                                 std::wcsstr_lowercase( buffer, oxorany( L"baseboard" ) ) ) {
                                is_hwid_value = true;
                            }

                            if ( std::wcsstr_lowercase( buffer, L"networkaddress" ) ||
                                 std::wcsstr_lowercase( buffer, L"macaddress" ) ) {
                                is_hwid_value = true;
                            }

                            if ( is_hwid_value ) {
                                auto info = reinterpret_cast< key_value_partial_information_t* >( key_value_info );

                                auto tick_count = nt::ke_query_tick_count( );
                                auto seed = static_cast< unsigned long >( tick_count.m_low_part );
                                seed = nt::rtl_random_ex( &seed );

                                if ( info->m_type == REG_SZ || info->m_type == REG_EXPAND_SZ ) {
                                    if ( info->m_data_length >= 32 ) {
                                        auto str_data = reinterpret_cast< wchar_t* >( info->m_data );

                                        wchar_t temp[ 64 ];
                                        const wchar_t hex[ ] = L"0123456789ABCDEF";
                                        int pos = 0;

                                        for ( int i = 7; i >= 0; i-- ) {
                                            temp[ pos++ ] = hex[ ( seed >> ( i * 4 ) ) & 0xF ];
                                        }

                                        temp[ pos++ ] = L'-';
                                        unsigned short part2 = seed >> 16;
                                        for ( int i = 3; i >= 0; i-- ) {
                                            temp[ pos++ ] = hex[ ( part2 >> ( i * 4 ) ) & 0xF ];
                                        }
                                        temp[ pos ] = L'\0';

                                        for ( int i = 0; i <= pos && i < ( int )( info->m_data_length / sizeof( wchar_t ) ); i++ ) {
                                            str_data[ i ] = temp[ i ];
                                        }
                                    }
                                }
                                else if ( info->m_type == REG_DWORD ) {
                                    if ( info->m_data_length >= sizeof( unsigned long ) ) {
                                        *reinterpret_cast< unsigned long* >( info->m_data ) = seed;
                                    }
                                }
                                else if ( info->m_type == REG_BINARY ) {
                                    for ( unsigned long i = 0; i < info->m_data_length && i < 64; i++ ) {
                                        seed = nt::rtl_random_ex( &seed );
                                        info->m_data[ i ] = static_cast< unsigned char >( seed & 0xFF );
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return result;
        }

        nt_status_t( __stdcall* nt_query_system_environment_value_ex_org )(
            unicode_string_t*,
            guid_t*,
            void*,
            unsigned long*,
            unsigned long*
            ) = nullptr;

        nt_status_t __stdcall nt_query_system_environment_value_ex_hk(
            unicode_string_t* variable_name,
            guid_t* vendor_guid,
            void* value,
            unsigned long* value_length,
            unsigned long* attributes
        ) {
            if ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) {
                nt::dbg_print( oxorany( "[ETW] NtQuerySystemEnvironmentValueEx: %wZ\n" ), variable_name );
            }

            return nt_query_system_environment_value_ex_org(
                variable_name,
                vendor_guid,
                value,
                value_length,
                attributes
            );
        }

        nt_status_t( __stdcall* nt_query_system_information_ex_org )(
            int SystemInformationClass,
            void* InputBuffer,
            unsigned long InputBufferLength,
            void* SystemInformation,
            unsigned long SystemInformationLength,
            unsigned long* ReturnLength
            ) = nullptr;

        nt_status_t __stdcall nt_query_system_information_ex_hk(
            int SystemInformationClass,
            void* InputBuffer,
            unsigned long InputBufferLength,
            void* SystemInformation,
            unsigned long SystemInformationLength,
            unsigned long* ReturnLength
        ) {
            if ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) {
                if ( SystemInformationClass == 0xD3 ) {
                    return nt_status_t::unsuccessful;
                }

                if ( SystemInformationClass == 0xB5 ) {
                    return nt_status_t::unsuccessful;
                }

                //if ( SystemInformationClass == 0x6B ) {
                //    return nt_status_t::success;
                //}
            }

            return nt_query_system_information_ex_org(
                SystemInformationClass,
                InputBuffer,
                InputBufferLength,
                SystemInformation,
                SystemInformationLength,
                ReturnLength
            );
        }

        // NtOpenProcess
        nt_status_t( __stdcall* nt_open_process_org )(
            void** ProcessHandle,
            unsigned long DesiredAccess,
            void* ObjectAttributes,
            void* ClientId
            ) = nullptr;

        nt_status_t __stdcall nt_open_process_hk(
            void** ProcessHandle,
            unsigned long DesiredAccess,
            void* ObjectAttributes,
            void* ClientId
        ) {
            if ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) {

                // Get PID from ClientId if available
                unsigned long pid = 0;
                if ( ClientId ) {
                    pid = *( unsigned long* )( ( unsigned char* )ClientId + sizeof( void* ) ); // UniqueProcess field
                }

                nt::dbg_print( oxorany( "[ETW] NtOpenProcess (PID: %lu, Access: 0x%X)\n" ),
                               pid, DesiredAccess );
            }

            return nt_open_process_org(
                ProcessHandle,
                DesiredAccess,
                ObjectAttributes,
                ClientId
            );
        }

        nt_status_t( __stdcall* nt_load_driver_org )( unicode_string_t* ) = nullptr;
        nt_status_t __stdcall nt_load_driver_hk( unicode_string_t* driver_service_name ) {
            if ( is_process_name( oxorany( "FortniteClient" ) ) ||
                 is_process_name( oxorany( "FortniteLauncher" ) ) ||
                 is_process_name( oxorany( "RustClient" ) ) ) {

                if ( driver_service_name && nt::mm_is_address_valid( driver_service_name ) ) {
                    if ( driver_service_name->m_buffer && nt::mm_is_address_valid( driver_service_name->m_buffer ) ) {

                        if ( std::wcsstr_lowercase( driver_service_name->m_buffer, oxorany( L"easyanticheat" ) ) ||
                             std::wcsstr_lowercase( driver_service_name->m_buffer, oxorany( L"eac" ) ) ||
                             std::wcsstr_lowercase( driver_service_name->m_buffer, oxorany( L"eaceos" ) ) ) {

                            nt::dbg_print( oxorany( "[ETW] blocked EasyAntiCheat driver load: %wZ\n" ), driver_service_name );
                        }

                        nt::dbg_print( oxorany( "[ETW] NtLoadDriver: %wZ\n" ), driver_service_name );
                    }
                }
            }

            return nt_load_driver_org( driver_service_name );
        }
    }

    bool register_hooks( ) {
        nt::dbg_print( oxorany( "[ETW] initializing hook routines\n" ) );

        if ( !trace::create_hook(
            oxorany( "NtAllocateUuids" ),
            reinterpret_cast< void* >( handler::nt_allocate_uuids_hk ),
            &handler::nt_allocate_uuids_org
        ) ) return false;

        if ( !trace::create_hook(
            oxorany( "NtQuerySystemInformation" ),
            reinterpret_cast< void* >( handler::nt_query_system_information_hk ),
            &handler::nt_query_system_information_org
        ) ) return false;

        if ( !trace::create_hook(
            oxorany( "NtQueryInformationToken" ),
            reinterpret_cast< void* >( handler::nt_query_information_token_hk ),
            reinterpret_cast< void** >( &handler::nt_query_information_token_org )
        ) ) return false;

        if ( !trace::create_hook(
            oxorany( "NtQueryVolumeInformationFile" ),
            reinterpret_cast< void* >( handler::nt_query_volume_information_file_hk ),
            reinterpret_cast< void** >( &handler::nt_query_volume_information_file_org )
        ) ) return false;

        if ( !trace::create_hook(
            oxorany( "NtQueryValueKey" ),
            reinterpret_cast< void* >( handler::nt_query_value_key_hk ),
            reinterpret_cast< void** >( &handler::nt_query_value_key_org )
        ) ) return false;

        if ( !trace::create_hook(
            oxorany( "NtQueryLicenseValue" ),
            reinterpret_cast< void* >( handler::nt_query_license_value_hk ),
            reinterpret_cast< void** >( &handler::nt_query_license_value_org )
        ) ) return false;

        if ( !trace::create_hook(
            oxorany( "NtQuerySystemEnvironmentValueEx" ),
            reinterpret_cast< void* >( handler::nt_query_system_environment_value_ex_hk ),
            reinterpret_cast< void** >( &handler::nt_query_system_environment_value_ex_org )
        ) ) return false;

        //if ( !trace::create_hook(
        //    oxorany( "NtQuerySystemInformationEx" ),
        //    reinterpret_cast< void* >( handler::nt_query_system_information_ex_hk ),
        //    reinterpret_cast< void** >( &handler::nt_query_system_information_ex_org )
        //) ) return false;

        //if ( !trace::create_hook(
        //    oxorany( "NtLoadDriver" ),
        //    reinterpret_cast< void* >( handler::nt_load_driver_hk ),
        //    reinterpret_cast< void** >( &handler::nt_load_driver_org )
        //) ) return false;

        //if ( !trace::create_hook(
        //    oxorany( "NtQueryInformationProcess" ),
        //    reinterpret_cast< void* >( handler::nt_query_information_process_hk ),
        //    reinterpret_cast< void** >( &handler::nt_query_information_process_org )
        //) ) return false;
         
        if ( !trace::create_hook(
            oxorany( "NtDeviceIoControlFile" ),
            reinterpret_cast< void* >( handler::nt_device_io_control_file_hk ),
            reinterpret_cast< void** >( &handler::nt_device_io_control_file_org )
        ) ) return false;

        //if ( !trace::create_hook(
        //    oxorany( "NtTraceControl" ),
        //    reinterpret_cast< void* >( handler::nt_trace_control_hk ),
        //    reinterpret_cast< void** >( &handler::nt_trace_control_org )
        //) ) return false;

        nt::dbg_print( oxorany( "[ETW] installed routines: hook count: %i\n" ), trace::m_count );
        return true;
    }
}