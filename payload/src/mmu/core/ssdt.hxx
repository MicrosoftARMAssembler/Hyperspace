#pragma once

namespace ssdt {

    service_descriptor_table_t* m_ke_service_descriptor_table = nullptr;
    service_descriptor_table_t* m_ke_service_descriptor_table_shadow = nullptr;

    std::uint64_t get_ke_service_descriptor_table( ) {
        auto ki_system_call64 = reinterpret_cast< std::uint8_t* >( __readmsr( 0xC0000082 ) );
        if ( !ki_system_call64 )
            return 0;

        while ( ki_system_call64[ 0 ] != 0x4C || ki_system_call64[ 1 ] != 0x8D || ki_system_call64[ 2 ] != 0x15 )
            ki_system_call64++;

        return ( std::uint64_t )( *( std::int32_t* )( ki_system_call64 + 3 ) + ki_system_call64 + 7 );
    }

    std::uint64_t get_ke_service_descriptor_table_shadow( ) {
        auto ki_system_call64 = reinterpret_cast< std::uint8_t* >( __readmsr( 0xC0000082 ) );
        if ( !ki_system_call64 )
            return 0;

        while ( ki_system_call64[ 0 ] != 0x4C || ki_system_call64[ 1 ] != 0x8D || ki_system_call64[ 2 ] != 0x1D )
            ki_system_call64++;

        return ( std::uint64_t )( *( std::int32_t* )( ki_system_call64 + 3 ) + ki_system_call64 + 7 );
    }

    std::uint64_t get_ki_system_service_repeat( ) {
        auto ki_system_call64 = reinterpret_cast< std::uint8_t* >( __readmsr( 0xC0000082 ) );
        if ( !ki_system_call64 )
            return 0;

        while ( ki_system_call64[ 0 ] != 0x4C || ki_system_call64[ 1 ] != 0x8D || ki_system_call64[ 2 ] != 0x15 )
            ki_system_call64++;

        return ( std::uint64_t )( ki_system_call64 );
    }

    std::uint32_t rva_to_offset( nt_headers_t* nt_headers, std::uint32_t rva ) {
        if ( rva < static_cast< std::uint32_t >( nt_headers->m_size_of_headers ) )
            return rva;

        auto sections = reinterpret_cast< section_header_t* >(
            reinterpret_cast< std::uint64_t >( nt_headers ) + offsetof( nt_headers_t, m_magic ) +
            nt_headers->m_size_of_optional_header );

        for ( std::int32_t i = 0; i < nt_headers->m_number_of_sections; ++i ) {
            auto& sec = sections[ i ];
            const auto va = static_cast< std::uint32_t >( sec.m_virtual_address );
            const auto vsz = static_cast< std::uint32_t >( sec.m_virtual_size );

            if ( rva >= va && rva < va + vsz )
                return ( rva - va ) + static_cast< std::uint32_t >( sec.m_pointer_to_raw_data );
        }

        return 0;
    }

    std::size_t safe_strnlen( const char* str, std::size_t max_len ) {
        std::size_t len = 0;
        while ( len < max_len && str[ len ] != '\0' )
            len++;
        return len;
    }

    bool safe_strcmp( const char* str1, const char* str2, std::size_t max_len ) {
        std::size_t i = 0;
        while ( i < max_len ) {
            if ( str1[ i ] != str2[ i ] )
                return false;
            if ( str1[ i ] == '\0' )
                return true;
            i++;
        }
        return false;
    }

    std::uint32_t get_syscall_index( const char* export_name ) {
        std::uint32_t syscall_index = static_cast< std::uint32_t >( -1 );

        unicode_string_t file_name{ };
        nt::rtl_init_unicode_string( &file_name, oxorany( L"\\SystemRoot\\System32\\ntdll.dll" ) );

        object_attributes_t obj_attr{ };
        obj_attr.m_length = sizeof( object_attributes_t );
        obj_attr.m_root_directory = nullptr;
        obj_attr.m_object_name = &file_name;
        obj_attr.m_attributes = 0x00000040L | 0x00000200L;
        obj_attr.m_security_descriptor = nullptr;

        io_status_block_t io_status{ };
        void* file_handle = nullptr;

        auto status = nt::zw_create_file(
            &file_handle,
            0x00100000L,
            &obj_attr,
            &io_status,
            nullptr,
            0x00000080,
            0x00000001 | 0x00000002,
            0x00000001,
            0x00000040 | 0x00000020,
            nullptr,
            0 );

        if ( status ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: zwcreatefile failed: 0x%X\n" ), status );
            return static_cast< std::uint32_t >( -1 );
        }

        FILE_STANDARD_INFORMATION file_info;
        status = nt::zw_query_information_file(
            file_handle,
            &io_status,
            &file_info,
            sizeof( file_info ),
            FileStandardInformation );

        if ( status ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: zyqueryinformationfile failed: 0x%X\n" ), status );
            nt::zw_close( file_handle );
            return static_cast< std::uint32_t >( -1 );
        }

        auto file_size = file_info.EndOfFile.LowPart;

        if ( file_size < 0x1000 || file_size > 0x10000000 )
        {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Invalid file size: 0x%X\n" ), file_size );
            nt::zw_close( file_handle );
            return static_cast< std::uint32_t >( -1 );
        }

        auto file_data = reinterpret_cast< std::uint8_t* >( nt::ex_allocate_pool_with_tag( NonPagedPool, file_size, 'ldNT' ) );
        if ( !file_data ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: ExAllocatePoolWithTag failed\n" ) );
            nt::zw_close( file_handle );
            return static_cast< std::uint32_t >( -1 );
        }

        ularge_integer_t offset = { 0 };
        status = nt::zw_read_file(
            file_handle,
            nullptr,
            nullptr,
            nullptr,
            &io_status,
            file_data,
            file_size,
            &offset,
            nullptr );

        nt::zw_close( file_handle );

        if ( status ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: ZwReadFile failed: 0x%X\n" ), status );
            nt::ex_free_pool_with_tag( file_data, oxorany( 'ldNT' ) );
            return static_cast< std::uint32_t >( -1 );
        }

        auto dos_header = reinterpret_cast< dos_header_t* >( file_data );
        if ( !dos_header->is_valid( ) ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Invalid DOS header: 0x%X\n" ), dos_header->m_magic );
            nt::ex_free_pool_with_tag( file_data, 'ldNT' );
            return static_cast< std::uint32_t >( -1 );
        }

        if ( dos_header->m_lfanew >= file_size || dos_header->m_lfanew < sizeof( dos_header_t ) ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Invalid e_lfanew: 0x%X\n" ), dos_header->m_lfanew );
            nt::ex_free_pool_with_tag( file_data, 'ldNT' );
            return static_cast< std::uint32_t >( -1 );
        }

        auto nt_headers = reinterpret_cast< nt_headers_t* >( file_data + dos_header->m_lfanew );

        if ( dos_header->m_lfanew + sizeof( nt_headers_t ) > file_size ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: NT headers out of bounds\n" ) );
            nt::ex_free_pool_with_tag( file_data, 'ldNT' );
            return static_cast< std::uint32_t >( -1 );
        }

        if ( !nt_headers->is_valid( ) ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Invalid NT headers\n" ) );
            nt::ex_free_pool_with_tag( file_data, 'ldNT' );
            return static_cast< std::uint32_t >( -1 );
        }

        auto export_dir_rva = static_cast< std::uint32_t >( nt_headers->m_export_table.m_virtual_address );
        auto export_dir_size = static_cast< std::uint32_t >( nt_headers->m_export_table.m_size );

        if ( export_dir_rva == 0 || export_dir_size == 0 ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: No export directory found\n" ) );
            nt::ex_free_pool_with_tag( file_data, 'ldNT' );
            return static_cast< std::uint32_t >( -1 );
        }

        auto export_dir_offset = rva_to_offset( nt_headers, export_dir_rva );
        if ( !export_dir_offset || export_dir_offset + sizeof( export_directory_t ) > file_size ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Export directory offset invalid\n" ) );
            nt::ex_free_pool_with_tag( file_data, oxorany( 'ldNT' ) );
            return static_cast< std::uint32_t >( -1 );
        }

        auto export_dir = reinterpret_cast< export_directory_t* >( file_data + export_dir_offset );
        auto number_of_names = static_cast< std::uint32_t >( export_dir->m_number_of_names );
        if ( number_of_names > 100000 ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Suspicious number of exports: %u\n" ), number_of_names );
            nt::ex_free_pool_with_tag( file_data, oxorany( 'ldNT' ) );
            return static_cast< std::uint32_t >( -1 );
        }

        auto addr_of_funcs_offset = rva_to_offset( nt_headers, static_cast< std::uint32_t >( export_dir->m_address_of_functions ) );
        auto addr_of_name_ordinals_offset = rva_to_offset( nt_headers, static_cast< std::uint32_t >( export_dir->m_address_of_names_ordinals ) );
        auto addr_of_names_offset = rva_to_offset( nt_headers, static_cast< std::uint32_t >( export_dir->m_address_of_names ) );

        if ( !addr_of_funcs_offset || !addr_of_name_ordinals_offset || !addr_of_names_offset ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Export table offsets invalid\n" ) );
            nt::ex_free_pool_with_tag( file_data, oxorany( 'ldNT' ) );
            return static_cast< std::uint32_t >( -1 );
        }

        if ( addr_of_funcs_offset + ( number_of_names * sizeof( std::uint32_t ) ) > file_size ||
            addr_of_name_ordinals_offset + ( number_of_names * sizeof( std::uint16_t ) ) > file_size ||
            addr_of_names_offset + ( number_of_names * sizeof( std::uint32_t ) ) > file_size ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Export arrays out of bounds\n" ) );
            nt::ex_free_pool_with_tag( file_data, oxorany( 'ldNT' ) );
            return static_cast< std::uint32_t >( -1 );
        }

        auto addr_of_funcs = reinterpret_cast< std::uint32_t* >( file_data + addr_of_funcs_offset );
        auto addr_of_name_ordinals = reinterpret_cast< std::uint16_t* >( file_data + addr_of_name_ordinals_offset );
        auto addr_of_names = reinterpret_cast< std::uint32_t* >( file_data + addr_of_names_offset );

        std::uint32_t func_offset = 0;
        std::size_t export_name_len = std::strlen( export_name );

        for ( std::uint32_t i = 0; i < number_of_names; i++ ) {
            auto curr_name_offset = rva_to_offset( nt_headers, addr_of_names[ i ] );
            if ( !curr_name_offset || curr_name_offset >= file_size )
                continue;

            std::size_t remaining_bytes = file_size - curr_name_offset;
            if ( remaining_bytes < export_name_len + 1 )
                continue;

            auto curr_name = reinterpret_cast< const char* >( file_data + curr_name_offset );

            std::size_t curr_name_len = safe_strnlen( curr_name, remaining_bytes );
            if ( curr_name_len == remaining_bytes ) {
                nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Export name not null-terminated at 0x%X\n" ), curr_name_offset );
                continue;
            }

            if ( safe_strcmp( curr_name, export_name, remaining_bytes ) ) {
                auto ordinal = addr_of_name_ordinals[ i ];
                if ( ordinal >= export_dir->m_number_of_functions ) {
                    nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Invalid ordinal %u for %s\n" ), ordinal, export_name );
                    continue;
                }

                auto func_rva = addr_of_funcs[ ordinal ];

                if ( func_rva >= export_dir_rva && func_rva < export_dir_rva + export_dir_size ) {
                    nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Forwarded export, ignoring: %s\n" ), export_name );
                    continue;
                }

                func_offset = rva_to_offset( nt_headers, func_rva );
                break;
            }
        }

        if ( !func_offset || func_offset >= file_size ) {
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Function %s not found in export table\n" ), export_name );
            nt::ex_free_pool_with_tag( file_data, oxorany( 'ldNT' ) );
            return static_cast< std::uint32_t >( -1 );
        }

        auto func_code = file_data + func_offset;

        for ( int i = 0; i < 32 && func_offset + i + 5 <= file_size; i++ ) {
            if ( func_code[ i ] == 0xC2 || func_code[ i ] == 0xC3 )
                break;

            if ( func_code[ i ] == 0xB8 ) {
                if ( func_offset + i + 5 <= file_size ) {
                    syscall_index = *reinterpret_cast< std::uint32_t* >( func_code + i + 1 );
                    break;
                }
            }
        }

        if ( syscall_index == static_cast< std::uint32_t >( -1 ) )
            nt::dbg_print( oxorany( "[ssdt] get_syscall_index: Syscall index not found in function %s\n" ), export_name );

        nt::ex_free_pool_with_tag( file_data, oxorany( 'ldNT' ) );
        return syscall_index;
    }

}