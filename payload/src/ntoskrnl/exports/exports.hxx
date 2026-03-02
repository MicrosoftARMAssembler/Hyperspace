#pragma once

namespace nt {
    kpcr_t* ke_get_pcr( ) {
        return reinterpret_cast< kpcr_t* >( KeGetPcr( ) );
    }

    nt_status_t rtl_ansi_string_to_unicode_string( unicode_string_t* destination_string, ansi_string_t* source_string, bool allocate_destination_string ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlAnsiStringToUnicodeString" ) );

        using fn_t = nt_status_t( * )( unicode_string_t*, ansi_string_t*, bool );
        return reinterpret_cast< fn_t >( function_addr )( destination_string, source_string, allocate_destination_string );
    }

    nt_status_t rtl_unicode_string_to_ansi_string( ansi_string_t* destination_string, unicode_string_t* source_string, bool allocate_destination_string ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlUnicodeStringToAnsiString" ) );

        using fn_t = nt_status_t( * )( ansi_string_t*, unicode_string_t*, bool );
        return reinterpret_cast< fn_t >( function_addr )( destination_string, source_string, allocate_destination_string );
    }

    void rtl_init_ansi_string( ansi_string_t* destination_string, const char* source_string ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlInitAnsiString" ) );

        using fn_t = void( * )( ansi_string_t*, const char* );
        reinterpret_cast< fn_t >( function_addr )( destination_string, source_string );
    }

    void rtl_init_unicode_string( unicode_string_t* destination_string, const wchar_t* source_string ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlInitUnicodeString" ) );

        using fn_t = void( * )( unicode_string_t*, const wchar_t* );
        reinterpret_cast< fn_t >( function_addr )( destination_string, source_string );
    }

    void rtl_free_ansi_string( ansi_string_t* ansi_string ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlFreeAnsiString" ) );

        using fn_t = void( * )( ansi_string_t* );
        reinterpret_cast< fn_t >( function_addr )( ansi_string );
    }

    void rtl_free_unicode_string( unicode_string_t* unicode_string ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlFreeUnicodeString" ) );

        using fn_t = void( * )( unicode_string_t* );
        reinterpret_cast< fn_t >( function_addr )( unicode_string );
    }

    std::uint32_t rtl_ulong_byte_swap( std::uint32_t source ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlUlongByteSwap" ) );

        using fn_t = std::uint32_t( * )( std::uint32_t );
        return reinterpret_cast< fn_t >( function_addr )( source );
    }

    std::uint16_t rtl_ushort_byte_swap( std::uint16_t source ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlUshortByteSwap" ) );

        using fn_t = std::uint16_t( * )( std::uint16_t );
        return reinterpret_cast< fn_t >( function_addr )( source );
    }

    void rtl_zero_memory( void* destination, std::size_t length ) {
        static std::uint64_t function_addr = 0;

        if ( !function_addr )
            function_addr = get_export( m_module_base, oxorany( "RtlZeroMemory" ) );

        using fn_t = void( * )( void*, std::size_t );
        reinterpret_cast< fn_t >( function_addr )( destination, length );
    }

    iop_irp_t* io_allocate_irp( char stack_size, bool charge_quota ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoAllocateIrp" ) );
            if ( !fn_address ) return nullptr;
        }

        using function_t = iop_irp_t * ( * )( char, bool );
        return reinterpret_cast< function_t >( fn_address )( stack_size, charge_quota );
    }

    void io_free_irp( iop_irp_t* irp ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoFreeIrp" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( iop_irp_t* );
        reinterpret_cast< function_t >( fn_address )( irp );
    }

    void io_set_completion_routine(
        iop_irp_t* irp,
        PIO_COMPLETION_ROUTINE completion_routine,
        void* context,
        bool invoke_on_success,
        bool invoke_on_error,
        bool invoke_on_cancel ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoSetCompletionRoutine" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( iop_irp_t*, PIO_COMPLETION_ROUTINE, void*, bool, bool, bool );
        reinterpret_cast< function_t >( fn_address )( irp, completion_routine, context,
            invoke_on_success, invoke_on_error, invoke_on_cancel );
    }

    void io_reuse_irp( iop_irp_t* irp, nt_status_t status ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoReuseIrp" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( iop_irp_t*, nt_status_t );
        reinterpret_cast< function_t >( fn_address )( irp, status );
    }

    nt_status_t rtl_convert_sid_to_unicode_string( unicode_string_t* unicode_string, sid_t* sid, bool allocate_destination_string ) {
        static std::uint64_t function_addr = 0ull;
        if ( !function_addr ) {
            function_addr = nt::get_export( m_module_base, oxorany( "RtlConvertSidToUnicodeString" ) );
            if ( !function_addr )
                return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( __fastcall* )( unicode_string_t*, sid_t*, unsigned char );
        return reinterpret_cast< function_t >( function_addr )( unicode_string, sid, allocate_destination_string );
    }

    ularge_integer_t ke_query_tick_count( ) {
        static std::uint64_t ke_query_tick_count_addr = 0ull;
        if ( !ke_query_tick_count_addr ) {
            ke_query_tick_count_addr = nt::get_export( m_module_base, oxorany( "KeQueryTickCount" ) );
            if ( !ke_query_tick_count_addr )
                return {};
        }
        using ke_query_tick_count_t = void( __fastcall* )( ularge_integer_t* );

        ularge_integer_t tick_count = {};
        reinterpret_cast< ke_query_tick_count_t >( ke_query_tick_count_addr )( &tick_count );
        return tick_count;
    }

    std::uint64_t ke_query_system_time( ) {
        static std::uint64_t ke_query_system_time_addr = 0ull;
        if ( !ke_query_system_time_addr ) {
            ke_query_system_time_addr = nt::get_export( m_module_base, oxorany( "KeQuerySystemTime" ) );
            if ( !ke_query_system_time_addr )
                return {};
        }
        using ke_query_system_time_t = void( __fastcall* )( ularge_integer_t* );

        ularge_integer_t system_time = {};
        reinterpret_cast< ke_query_system_time_t >( ke_query_system_time_addr )( &system_time );
        return system_time.m_quad_part;
    }

    std::uint64_t ke_query_system_time_precise( ) {
        static std::uint64_t ke_query_system_time_precise = 0ull;
        if ( !ke_query_system_time_precise ) {
            ke_query_system_time_precise = nt::get_export( m_module_base, oxorany( "KeQuerySystemTimePrecise" ) );
            if ( !ke_query_system_time_precise )
                return {};
        }
        using ke_query_system_time_precise_t = void( __fastcall* )( ularge_integer_t* );

        ularge_integer_t system_time = {};
        reinterpret_cast< ke_query_system_time_precise_t >( ke_query_system_time_precise )( &system_time );
        return system_time.m_quad_part;
    }

    unsigned long rtl_length_sid( sid_t* sid ) {
        static std::uint64_t rtl_length_sid = 0ull;
        if ( !rtl_length_sid ) {
            rtl_length_sid = nt::get_export( m_module_base, oxorany( "RtlLengthSid" ) );
            if ( !rtl_length_sid )
                return 0;
        }

        using rtl_length_sid_t = unsigned long( __fastcall* )( sid_t* );
        return reinterpret_cast< rtl_length_sid_t >( rtl_length_sid )( sid );
    }

    void kd_acquire_debugger_lock( std::uint8_t* old_irql ) {
        static std::uint64_t kd_acquire_debugger_lock = 0ull;
        if ( !kd_acquire_debugger_lock ) {
            kd_acquire_debugger_lock = nt::get_export( m_module_base, oxorany( "KdAcquireDebuggerLock" ) );
            if ( !kd_acquire_debugger_lock )
                return;
        }

        using kd_acquire_debugger_lock_t = bool( __fastcall* )( std::uint8_t* );
        reinterpret_cast< kd_acquire_debugger_lock_t >( kd_acquire_debugger_lock )( old_irql );
    }

    bool kd_release_debugger_lock( std::uint8_t irql ) {
        static std::uint64_t kd_release_debugger_lock = 0ull;
        if ( !kd_release_debugger_lock ) {
            kd_release_debugger_lock = nt::get_export( m_module_base, oxorany( "KdReleaseDebuggerLock" ) );
            if ( !kd_release_debugger_lock )
                return false;
        }

        using kd_release_debugger_lock_t = bool( __fastcall* )( std::uint8_t );
        return reinterpret_cast< kd_release_debugger_lock_t >( kd_release_debugger_lock )( irql );
    }

    std::uint64_t get_halp_stall_counter( ) {
        static std::uint64_t halp_stall_counter = 0;
        if ( !halp_stall_counter ) {
            auto halp_initialize_timers = reinterpret_cast< std::uint8_t* > (
                nt::scan_ida_pattern(
                    oxorany( "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 48 8B D9" )
                ) );
            if ( !halp_initialize_timers )
                return 0;

            while ( !( halp_initialize_timers[ 0 ] == 0x48 &&
                halp_initialize_timers[ 1 ] == 0x8B &&
                halp_initialize_timers[ 2 ] == 0x1D ) )
                halp_initialize_timers++;

            auto halp_stall_counter_rva = *reinterpret_cast< std::int32_t* >( halp_initialize_timers + 3 );
            halp_stall_counter = *reinterpret_cast< std::uint64_t* >( halp_initialize_timers + 7 + halp_stall_counter_rva );
        }

        return halp_stall_counter;
    }

    bool ke_alert_thread( kthread_t* target_thread ) {
        static std::uint64_t ke_alert_thread = 0ull;
        if ( !ke_alert_thread ) {
            ke_alert_thread = nt::get_export( m_module_base, oxorany( "KeAlertThread" ) );
            if ( !ke_alert_thread )
                return false;
        }

        using ke_alert_thread_t = bool( __fastcall* )( kthread_t*, int );
        return reinterpret_cast< ke_alert_thread_t >( ke_alert_thread )( target_thread, 0 );
    }

    nt_status_t mm_mark_physical_memory_as_bad( std::uint64_t physical_address, std::size_t size ) {
        static std::uint64_t mm_mark_physical_memory_as_bad = 0ull;
        if ( !mm_mark_physical_memory_as_bad ) {
            mm_mark_physical_memory_as_bad = nt::get_export( m_module_base, oxorany( "MmMarkPhysicalMemoryAsBad" ) );
            if ( !mm_mark_physical_memory_as_bad )
                return nt_status_t::unsuccessful;
        }

        physical_address_t phys_addr{ physical_address };
        ularge_integer_t phys_size{ .m_quad_part = size };

        using mi_copy_on_write_t = nt_status_t( __fastcall* )( physical_address_t*, ularge_integer_t* );
        return reinterpret_cast< mi_copy_on_write_t >( mm_mark_physical_memory_as_bad )( &phys_addr, &phys_size );
    }

    nt_status_t mi_copy_on_write( std::uint64_t virtual_address, std::uint64_t* pte_address ) {
        static std::uint64_t mi_copy_on_write = 0ull;
        if ( !mi_copy_on_write ) {
            mi_copy_on_write = m_pdb_symbols.m_mi_copy_on_write;
            if ( !mi_copy_on_write )
                return nt_status_t::unsuccessful;
        }

        using mi_copy_on_write_t = nt_status_t( __fastcall* )( std::uint64_t, std::uint64_t*, std::int64_t, std::int32_t );
        return reinterpret_cast< mi_copy_on_write_t >( mi_copy_on_write )( virtual_address, pte_address, -1, 0 );
    }

    std::uint32_t ps_get_process_session_id( eprocess_t* process ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "PsGetProcessSessionId" ) );
            if ( !fn_address )
                return false;
        }

        using function_t = std::uint32_t( __stdcall* )( eprocess_t* );
        return reinterpret_cast< function_t >( fn_address )( process );
    }

	bool ke_initialize_apc( kapc_t* apc, kthread_t* thread, std::uint8_t environment,
		pkkernel_routine_t kernel_routine, pkrundown_routine_t rundown_routine,
		pknormal_routine_t normal_routine, kprocessor_mode_t apc_mode, void* normal_context ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = nt::get_export( m_module_base, oxorany( "KeInitializeApc" ) );
			if ( !fn_address )
				return false;
		}

		using function_t = bool( __stdcall* )( kapc_t*, kthread_t*, std::uint8_t,
			pkkernel_routine_t, pkrundown_routine_t, pknormal_routine_t,
			kprocessor_mode_t, void* );
		return reinterpret_cast< function_t >( fn_address )( apc, thread, environment,
			kernel_routine, rundown_routine, normal_routine, apc_mode, normal_context );
	}

    nt_status_t rtl_create_user_thread( void* process_handle,
        void* security_descriptor,
        bool create_suspended,
        unsigned long stack_zero_bits,
        std::size_t* stack_reserved,
        std::size_t* stack_commit,
        void* start_address,
        void* start_parameter,
        void** thread_handle,
        client_id_t* client_id ) {
        static std::uint64_t rtl_create_user_thread_addr = 0ull;
        if ( !rtl_create_user_thread_addr ) {
            rtl_create_user_thread_addr = nt::get_export( m_module_base, oxorany( "RtlCreateUserThread" ) );
            if ( !rtl_create_user_thread_addr )
                return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( __fastcall* )(
            void*,
            void*,
            unsigned char,
            unsigned long,
            std::size_t*,
            std::size_t*,
            void*,
            void*,
            void**,
            client_id_t*
            );

        return reinterpret_cast< function_t >( rtl_create_user_thread_addr )(
            process_handle,
            security_descriptor,
            create_suspended ? 1 : 0,
            stack_zero_bits,
            stack_reserved,
            stack_commit,
            start_address,
            start_parameter,
            thread_handle,
            client_id
            );
    }

	bool ke_insert_queue_apc( kapc_t* apc, void* system_arg1 = nullptr, void* system_arg2 = nullptr,
		std::int32_t priority_increment = 0 ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = nt::get_export( m_module_base, oxorany( "KeInsertQueueApc" ) );
			if ( !fn_address )
				return false;
		}

		using function_t = bool( __stdcall* )( kapc_t*, void*, void*, std::int32_t );
		return reinterpret_cast< function_t >( fn_address )( apc, system_arg1, system_arg2, priority_increment );
	}

	bool ke_test_alert_thread( kprocessor_mode_t mode = kprocessor_mode_t::user_mode ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = nt::get_export( m_module_base, oxorany( "KeTestAlertThread" ) );
			if ( !fn_address )
				return false;
		}

		using function_t = bool( __stdcall* )( kprocessor_mode_t );
		return reinterpret_cast< function_t >( fn_address )( mode );
	}

    std::uint8_t* get_etwp_max_pmc_counter( ) {
        return reinterpret_cast< std::uint8_t* >( m_pdb_symbols.m_etwp_max_pmc_counter );
    }


    std::uint64_t get_etwp_debugger_data( ) {
        return m_pdb_symbols.m_etwp_debugger_data;
    }

    std::uint64_t* get_halp_performance_counter( ) {
        auto ke_query_performance_counter = nt::get_export( m_module_base, oxorany( "KeQueryPerformanceCounter" ) );
        if ( !ke_query_performance_counter )
            return nullptr;

        auto scan_ptr = reinterpret_cast< std::uint8_t* >( ke_query_performance_counter + 0x05 );
        while ( !( scan_ptr[ 0 ] == 0x48 && scan_ptr[ 1 ] == 0x8B ) ) {
            scan_ptr++;
        }

        scan_ptr += 0x08;

        auto rel_offset = *reinterpret_cast< std::int32_t* >( scan_ptr + 3 );
        auto halp_performance_counter = reinterpret_cast< std::uint64_t* >( scan_ptr + 7 + rel_offset );

        return halp_performance_counter;
    }

    nt_status_t mi_obtain_section_for_driver( unicode_string_t* string1, unicode_string_t* driver_path, std::uint64_t* driver_section ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = m_pdb_symbols.m_mi_obtain_section_for_driver;
            if ( !fn_address )
                return static_cast< nt_status_t >( -1 );
        }

        using function_t = nt_status_t( __stdcall* )( unicode_string_t*, unicode_string_t*, std::uint64_t, std::uint64_t, std::uint64_t* );
        return reinterpret_cast< function_t >( fn_address )( string1, driver_path, 0, 0, driver_section );
    }

    bool mm_set_page_protection( std::uint64_t virtual_address, std::size_t size, std::uint64_t protection ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = m_pdb_symbols.m_mm_set_page_protection;
            if ( !fn_address )
                return static_cast< nt_status_t >( -1 );
        }

        using function_t = bool( __stdcall* )( std::uint64_t, std::size_t, std::uint64_t );
        return reinterpret_cast< function_t >( fn_address )( virtual_address, size, protection );
    }

    nt_status_t ex_uuid_create( guid_t* uuid ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ExUuidCreate" ) );
            if ( !fn_address ) return static_cast< nt_status_t >( -1 );
        }

        using function_t = nt_status_t( __stdcall* )( guid_t* );
        return reinterpret_cast< function_t >( fn_address )( uuid );
    }

    object_type_t* io_driver_object_type( ) {
        static auto io_driver_object_type_ptr = 0ull;
        if ( !io_driver_object_type_ptr ) {
            io_driver_object_type_ptr = nt::get_export( m_module_base, oxorany( "IoDriverObjectType" ) );
            if ( !io_driver_object_type_ptr ) return nullptr;
        }

        return *reinterpret_cast< object_type_t** >( io_driver_object_type_ptr );
    }

    nt_status_t ob_reference_object_by_name( unicode_string_t* object_name, std::uint32_t attributes, void* access_state, std::uint32_t desired_access, object_type_t* object_type, std::uint8_t access_mode, void* parse_context, void** object ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ObReferenceObjectByName" ) );
            if ( !fn_address )
                return static_cast< nt_status_t >( -1 );
        }

        using function_t = nt_status_t( __fastcall* )( unicode_string_t*, std::uint32_t, void*, std::uint32_t, object_type_t*, std::uint8_t, void*, void** );
        return reinterpret_cast< function_t >( fn_address )( object_name, attributes, access_state, desired_access, object_type, access_mode, parse_context, object );
    }

    driver_object_t* get_driver_object_by_name( const wchar_t* driver_name ) {
        wchar_t driver_path[ 260 ]{};
        std::size_t name_len = wcslen( driver_name );
        if ( name_len >= 4 && _wcsicmp( driver_name + name_len - 4, oxorany( L".sys" ) ) == 0 ) {
            name_len -= 4;
        }

        wcscpy( driver_path, oxorany( L"\\Driver\\" ) );
        wcsncat( driver_path, driver_name, name_len );
        driver_path[ 8 + name_len ] = L'\0';

        unicode_string_t object_name{};
        rtl_init_unicode_string( &object_name, driver_path );

        void* driver_object = nullptr;
        auto status = ob_reference_object_by_name( &object_name, 0x40, nullptr, 0, nt::io_driver_object_type( ), 0, nullptr, &driver_object );
        if ( status >= 0 && driver_object ) {
            return reinterpret_cast< driver_object_t* >( driver_object );
        }

        return nullptr;
    }

    char mi_is_page_on_bad_list( mmpfn_t* pfn_entry ) {
        static auto mi_is_page_on_bad_list = 0ull;
        if ( !mi_is_page_on_bad_list ) {
            mi_is_page_on_bad_list = m_pdb_symbols.m_mi_is_page_on_bad_list;
            if ( !mi_is_page_on_bad_list )
                return 0;
        }

        using function_t = char( __stdcall* )( mmpfn_t* );
        return reinterpret_cast< function_t >( mi_is_page_on_bad_list )( pfn_entry );
    }

    char mi_make_page_bad( mmpfn_t* pfn_entry, char lock ) {
        static auto mi_make_page_bad = 0ull;
        if ( !mi_make_page_bad ) {
            mi_make_page_bad = m_pdb_symbols.m_mi_make_page_bad;
            if ( !mi_make_page_bad )
                return 0;
        }

        using function_t = char( __stdcall* )( mmpfn_t*, char );
        return reinterpret_cast< function_t >( mi_make_page_bad )( pfn_entry, lock );
    }

    std::uint32_t get_os_version( ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "RtlGetVersion" ) );
            if ( !fn_address ) return 0;
        }

        os_version_info_t os_version = { 0 };
        using function_t = nt_status_t( __stdcall* )( os_version_info_t* );
        auto result = reinterpret_cast< function_t >( fn_address )( &os_version );
        if ( !result && os_version.m_build_number ) {
            return os_version.m_build_number;
        }

        return 0;
    }

    bool mm_initialize_process_address_space( eprocess_t* clone_process, eprocess_t* target_process ) {
        static auto mm_initialize_process_address_space = 0ull;
        if ( !mm_initialize_process_address_space ) {
            mm_initialize_process_address_space = m_pdb_symbols.m_mm_initialize_process_address_space;
            if ( !mm_initialize_process_address_space )
                return false;
        }

        ULONG flags = 0;
        using function_t = __int64( eprocess_t*, eprocess_t*, void*, ULONG*, int );
        auto result = reinterpret_cast< function_t* >( mm_initialize_process_address_space )(
            clone_process, target_process, nullptr, &flags, 0 );

        return result == 0;
    }

    unsigned short rtl_capture_stack_back_trace(
        unsigned long FramesToSkip,
        unsigned long FramesToCapture,
        void** BackTrace,
        unsigned long* BackTraceHash ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "RtlCaptureStackBackTrace" ) );
            if ( !fn_address ) return 0;
        }

        using function_t = unsigned short( * )( unsigned long, unsigned long, void**, unsigned long* );
        return reinterpret_cast< function_t >( fn_address )(
            FramesToSkip,
            FramesToCapture,
            BackTrace,
            BackTraceHash
            );
    }

    ULONG __stdcall rtl_walk_frame_chain( PVOID* Callers, ULONG Count, ULONG Flags ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "RtlWalkFrameChain" ) );
            if ( !fn_address ) return 0;
        }

        using function_t = ULONG( __stdcall* )( PVOID*, ULONG, ULONG );
        return reinterpret_cast< function_t >( fn_address )( Callers, Count, Flags );
    }

    void ex_initialize_rundown_protection( void* rundown_ref ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ExInitializeRundownProtection" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( void* );
        reinterpret_cast< function_t >( fn_address )( rundown_ref );
    }

    bool ex_acquire_rundown_protection( void* rundown_ref ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ExAcquireRundownProtection" ) );
            if ( !fn_address ) return false;
        }

        using function_t = bool( * )( void* );
        return reinterpret_cast< function_t >( fn_address )( rundown_ref );
    }

    void ex_release_rundown_protection( void* rundown_ref ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ExReleaseRundownProtection" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( void* );
        reinterpret_cast< function_t >( fn_address )( rundown_ref );
    }

    void rtl_restore_context( context_t* context, exception_record_t* exception_record ) {
        static auto rtl_restore_context = 0ull;
        if ( !rtl_restore_context ) {
            rtl_restore_context = nt::get_export( m_module_base, oxorany( "RtlRestoreContext" ) );
            if ( !rtl_restore_context ) return;
        }

        using function_t = void( context_t*, exception_record_t* );
        reinterpret_cast< function_t* >( rtl_restore_context )( context, exception_record );
    }

    nt_status_t ob_query_name_string(
        void* object,
        object_name_information_t* object_name_info,
        unsigned long length,
        unsigned long* return_length
    ) {
        static auto ob_query_name_string = 0ull;
        if ( !ob_query_name_string ) {
            ob_query_name_string = nt::get_export( m_module_base, oxorany( "ObQueryNameString" ) );
            if ( !ob_query_name_string ) return {};
        }

        using function_t = nt_status_t( void*, object_name_information_t*, unsigned long, unsigned long* );
        return reinterpret_cast< function_t* >( ob_query_name_string )( object, object_name_info, length, return_length );
    }

    object_type_t* ob_get_object_type( void* object ) {
        static auto ob_get_object_type = 0ull;
        if ( !ob_get_object_type ) {
            ob_get_object_type = nt::get_export( m_module_base, oxorany( "ObGetObjectType" ) );
            if ( !ob_get_object_type ) return {};
        }

        using function_t = object_type_t * ( void* );
        return reinterpret_cast< function_t* >( ob_get_object_type )( object );
    }

    bool ps_is_system_thread( ethread_t* thread ) {
        static auto ps_is_system_thread = 0ull;
        if ( !ps_is_system_thread ) {
            ps_is_system_thread = nt::get_export( m_module_base, oxorany( "PsIsSystemThread" ) );
            if ( !ps_is_system_thread ) return {};
        }

        using function_t = bool( ethread_t* );
        return reinterpret_cast< function_t* >( ps_is_system_thread )( thread );
    }

    list_entry_t* get_rtlp_debug_print_callback_list( ) {
        list_entry_t* rtlp_debug_print_callback_list = nullptr;
        if ( !rtlp_debug_print_callback_list ) {
            rtlp_debug_print_callback_list = reinterpret_cast< list_entry_t* >( m_pdb_symbols.m_rtlp_debug_print_callback_list );
            if ( !rtlp_debug_print_callback_list ) return { };
        }

        return rtlp_debug_print_callback_list;
    }

    void ex_initialize_work_item( work_queue_item_t* item, pworker_thread_routine_t routine, void* context ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ExInitializeWorkItem" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( work_queue_item_t*, pworker_thread_routine_t, void* );
        reinterpret_cast< function_t >( fn_address )( item, routine, context );
    }

    void ex_queue_work_item( work_queue_item_t* item, work_queue_type_t queue_type ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ExQueueWorkItem" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( work_queue_item_t*, work_queue_type_t );
        reinterpret_cast< function_t >( fn_address )( item, queue_type );
    }

    nt_status_t ps_remove_load_image_notify_routine( p_load_image_notify_routine notify_routine ) {
        static auto ps_remove_load_image_notify_routine = 0ull;
        if ( !ps_remove_load_image_notify_routine ) {
            ps_remove_load_image_notify_routine = nt::get_export( m_module_base, oxorany( "PsRemoveLoadImageNotifyRoutine" ) );
            if ( !ps_remove_load_image_notify_routine ) return {};
        }

        using function_t = nt_status_t( p_load_image_notify_routine );
        return reinterpret_cast< function_t* >( ps_remove_load_image_notify_routine )( notify_routine );
    }

    p_exception_routine virtual_unwind(
        std::uint32_t hand_flags,
        std::uint64_t image_base,
        std::uint64_t control_pc,
        runtime_function_t* function_entry,
        context_t* context_record,
        void** handler_data,
        std::uint64_t* establisher_frame,
        knonvolatile_context_pointers_t* context_pointers
    ) {
        static auto rtl_virtual_unwind = 0ull;

        if ( !rtl_virtual_unwind ) {
            rtl_virtual_unwind = nt::get_export( m_module_base, oxorany( "RtlVirtualUnwind" ) );
            if ( !rtl_virtual_unwind ) return nullptr;
        }

        using function_t = p_exception_routine(
            std::uint32_t, std::uint64_t, std::uint64_t, runtime_function_t*,
            context_t*, void**, std::uint64_t*, knonvolatile_context_pointers_t*
        );

        return reinterpret_cast< function_t* >( rtl_virtual_unwind )(
            hand_flags, image_base, control_pc, function_entry,
            context_record, handler_data, establisher_frame, context_pointers
            );
    }

    runtime_function_t* lookup_function_entry( std::uint64_t control_pc, std::uint64_t* image_base ) {
        static auto rtl_lookup_function_entry = 0ull;

        if ( !rtl_lookup_function_entry ) {
            rtl_lookup_function_entry = nt::get_export( m_module_base, oxorany( "RtlLookupFunctionEntry" ) );
            if ( !rtl_lookup_function_entry ) return nullptr;
        }

        using function_t = runtime_function_t * ( std::uint64_t, std::uint64_t*, void* );
        return reinterpret_cast< function_t* >( rtl_lookup_function_entry )( control_pc, image_base, nullptr );
    }

    nt_status_t ps_remove_create_thread_notify_routine( p_create_thread_notify_routine notify_routine ) {
        static auto ps_remove_create_thread_notify_routine = 0ull;
        if ( !ps_remove_create_thread_notify_routine ) {
            ps_remove_create_thread_notify_routine = nt::get_export( m_module_base, oxorany( "PsRemoveCreateThreadNotifyRoutine" ) );
            if ( !ps_remove_create_thread_notify_routine ) return {};
        }

        using function_t = nt_status_t( p_create_thread_notify_routine );
        return reinterpret_cast< function_t* >( ps_remove_create_thread_notify_routine )( notify_routine );
    }

    nt_status_t ps_terminate_system_thread( ) {
        static auto ps_terminate_system_thread = 0ull;
        if ( !ps_terminate_system_thread ) {
            ps_terminate_system_thread = nt::get_export( m_module_base, oxorany( "PsTerminateSystemThread" ) );
            if ( !ps_terminate_system_thread ) return {};
        }

        using function_t = nt_status_t( nt_status_t );
        return reinterpret_cast< function_t* >( ps_terminate_system_thread )( nt_status_t::success );
    }

    nt_status_t ex_unsubscribe_wnf_state_change( void* wnf_handle ) {
        static auto ex_unsubscribe_wnf_state_change = 0ull;
        if ( !ex_unsubscribe_wnf_state_change ) {
            ex_unsubscribe_wnf_state_change = nt::get_export( m_module_base, oxorany( "ExUnsubscribeWnfStateChange" ) );
            if ( !ex_unsubscribe_wnf_state_change ) return {};
        }

        using function_t = nt_status_t( void* );
        return reinterpret_cast< function_t* >( ex_unsubscribe_wnf_state_change )( wnf_handle );
    }

    nt_status_t ke_delay_execution_thread( char wait_mode, bool alertable, ularge_integer_t* interval ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeDelayExecutionThread" ) );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( char, bool, ularge_integer_t* );
        return reinterpret_cast< function_t* >( fn_address )( wait_mode, alertable, interval );
    }

    eprocess_t* ps_get_thread_process( ethread_t* thread ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "PsGetThreadProcess" ) );
            if ( !fn_address ) return {};
        }

        using function_t = eprocess_t * ( ethread_t* );
        return reinterpret_cast< function_t* >( fn_address )( thread );
    }

    nt_status_t ke_wait_for_single_object(
        void* object,
        long wait_reason,
        long wait_mode,
        bool alertable,
        int wait_duration
    ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "KeWaitForSingleObject" ) );
            if ( !function_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )(
            void*,
            long,
            long,
            bool,
            ularge_integer_t*
            );

        ularge_integer_t timeout{ };
        timeout.m_quad_part = wait_duration * -10000i64;
        return reinterpret_cast< function_t >( function_address )(
            object,
            wait_reason,
            wait_mode,
            alertable,
            &timeout
            );
    }

    nt_status_t nt_wait_for_keyed_event(
        void* keyed_event,
        void* key,
        bool alertable,
        int64_t* timeout
    ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "NtWaitForKeyedEvent" ) );
            if ( !function_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )(
            void*,
            void*,
            bool,
            int64_t*
            );

        return reinterpret_cast< function_t >( function_address )(
            keyed_event,
            key,
            alertable,
            timeout
            );
    }

    nt_status_t nt_release_keyed_event(
        void* keyed_event,
        void* key,
        bool alertable,
        int64_t* timeout
    ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "NtReleaseKeyedEvent" ) );
            if ( !function_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )(
            void*,
            void*,
            bool,
            int64_t*
            );

        return reinterpret_cast< function_t >( function_address )(
            keyed_event,
            key,
            alertable,
            timeout
            );
    }

    nt_status_t ex_query_wnf_state_data( void* wnf_struct, PULONG time_stamp, void* buffer, PULONG size ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "ExQueryWnfStateData" ) );
            if ( !function_address ) return {};
        }

        using function_t = nt_status_t( * )(
            void*,
            PULONG,
            void*,
            PULONG
            );

        return reinterpret_cast< function_t >( function_address )(
            wnf_struct,
            time_stamp,
            buffer,
            size
            );
    }

    long ke_release_semaphore(
        ksemaphore_t* semaphore,
        long increment,
        long adjustment,
        bool wait
    ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "KeReleaseSemaphore" ) );
            if ( !function_address ) return {};
        }

        using function_t = long ( * )(
            ksemaphore_t*,
            long,
            long,
            bool
            );

        return reinterpret_cast< function_t >( function_address )(
            semaphore,
            increment,
            adjustment,
            wait
            );
    }

    mdl_t* mm_allocate_pages_for_mdl(
        physical_address_t low_address,
        physical_address_t high_address,
        physical_address_t skip_bytes,
        size_t total_bytes
    ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "MmAllocatePagesForMdl" ) );
            if ( !function_address ) return nullptr;
        }

        using function_t = mdl_t * ( * )(
            physical_address_t,
            physical_address_t,
            physical_address_t,
            size_t
            );

        return reinterpret_cast< function_t >( function_address )(
            low_address,
            high_address,
            skip_bytes,
            total_bytes
            );
    }

    long ke_set_event(
        kevent_t* event,
        long increment,
        bool wait ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "KeSetEvent" ) );
            if ( !function_address ) return nt_status_t::unsuccessful;
        }

        using function_t = long( * )(
            kevent_t*,
            long,
            bool
            );

        return reinterpret_cast< function_t >( function_address )(
            event,
            increment,
            wait
            );
    }

    nt_status_t ex_subscribe_wnf_state_change(
        PVOID* wnfStruct,
        PCWNF_STATE_NAME stateName,
        ULONG eventMask,
        PULONG changeStamp,
        PVOID callback,
        PVOID callbackContext ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "ExSubscribeWnfStateChange" ) );
            if ( !function_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )(
            PVOID*,
            PCWNF_STATE_NAME,
            ULONG,
            PULONG,
            PVOID,
            PVOID
            );

        return reinterpret_cast< function_t >( function_address )(
            wnfStruct,
            stateName,
            eventMask,
            changeStamp,
            callback,
            callbackContext
            );
    }

    nt_status_t ob_reference_object_by_handle(
        void* handle,
        std::uint32_t desired_access,
        void* object_type,
        std::uint8_t access_mode,
        void** object,
        void* handle_information
    ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "ObReferenceObjectByHandle" ) );
            if ( !function_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )(
            void*,
            std::uint32_t,
            void*,
            std::uint8_t,
            void**,
            void*
            );

        return reinterpret_cast< function_t >( function_address )(
            handle,
            desired_access,
            object_type,
            access_mode,
            object,
            handle_information
            );
    }

    std::uint32_t ps_get_current_process_id( ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "PsGetCurrentProcessId" ) );
            if ( !function_address ) return nt_status_t::unsuccessful;
        }

        using function_t = std::uint64_t( * )( );
        return static_cast< std::uint32_t >( reinterpret_cast< function_t >( function_address )( ) );
    }

    std::uint64_t mm_user_probe_address( ) {
        static auto mm_user_probe_address = 0ull;
        if ( !mm_user_probe_address ) {
            mm_user_probe_address = nt::get_export( m_module_base, oxorany( "MmUserProbeAddress" ) );
            if ( !mm_user_probe_address ) return nt_status_t::unsuccessful;
        }

        return *reinterpret_cast< std::uint64_t* >( mm_user_probe_address );
    }

    std::uint64_t mm_system_range_start( ) {
        static auto mm_system_range_start = 0ull;
        if ( !mm_system_range_start ) {
            mm_system_range_start = nt::get_export( m_module_base, oxorany( "MmSystemRangeStart" ) );
            if ( !mm_system_range_start ) return nt_status_t::unsuccessful;
        }

        return *reinterpret_cast< std::uint64_t* >( mm_system_range_start );
    }

    nt_status_t ob_open_object_by_pointer(
        void* object,
        std::uint32_t handle_attributes,
        PACCESS_STATE access_state,
        ACCESS_MASK desired_access,
        void* object_type,
        std::int8_t access_mode,
        void** handle
    ) {
        static auto function_address = 0ull;
        if ( !function_address ) {
            function_address = nt::get_export( m_module_base, oxorany( "ObOpenObjectByPointer" ) );
            if ( !function_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )(
            void*,
            std::uint32_t,
            PACCESS_STATE,
            ACCESS_MASK,
            void*,
            std::int8_t,
            void**
            );

        return reinterpret_cast< function_t >( function_address )(
            object,
            handle_attributes,
            access_state,
            desired_access,
            object_type,
            access_mode,
            handle
            );
    }

    std::uint8_t ke_get_current_irql( ) {
        static auto ke_get_current_irql = 0ull;
        if ( !ke_get_current_irql ) {
            ke_get_current_irql = nt::get_export( m_module_base, oxorany( "KeGetCurrentIrql" ) );
            if ( !ke_get_current_irql ) return {};
        }

        using function_t = std::uint8_t( );
        reinterpret_cast< function_t* >( ke_get_current_irql )( );
    }

    void ob_dereference_object( void* object ) {
        static auto ob_dereference_object = 0ull;
        if ( !ob_dereference_object ) {
            ob_dereference_object = nt::get_export( m_module_base, oxorany( "ObDereferenceObject" ) );
            if ( !ob_dereference_object ) return;
        }

        using function_t = void( void* );
        reinterpret_cast< function_t* >( ob_dereference_object )( object );
    }

    void* mm_allocate_independent_pages( std::size_t size ) {
        static auto mm_allocate_independent_pages = 0ull;
        if ( !mm_allocate_independent_pages ) {
            mm_allocate_independent_pages = m_pdb_symbols.m_mm_allocate_independent_pages;
            if ( !mm_allocate_independent_pages )
                return nullptr;
        }

        using function_t = void* ( std::uint64_t, int, std::uint64_t, unsigned int );
        return reinterpret_cast< function_t* >( mm_allocate_independent_pages )( size, -1, 0, 0 );
    }

    void mm_free_independent_pages( std::uint64_t independent_pages, std::size_t size ) {
        static auto mm_free_independent_pages = 0ull;
        if ( !mm_free_independent_pages ) {
            mm_free_independent_pages = m_pdb_symbols.m_mm_free_independent_pages;
            if ( !mm_free_independent_pages )
                return;
        }

        using function_t = void( std::uint64_t, int );
        reinterpret_cast< function_t* >( mm_free_independent_pages )( independent_pages, size );
    }

    nt_status_t ps_get_process_exit_status( eprocess_t* process ) {
        static auto ps_get_process_exit_status = 0ull;
        if ( !ps_get_process_exit_status ) {
            ps_get_process_exit_status = nt::get_export( m_module_base, oxorany( "PsGetProcessExitStatus" ) );
            if ( !ps_get_process_exit_status ) return {};
        }

        using function_t = nt_status_t( eprocess_t* );
        return reinterpret_cast< function_t* >( ps_get_process_exit_status )( process );
    }

    nt_status_t hvl_query_connection( ) {
        static auto hvl_query_connection = 0ull;
        if ( !hvl_query_connection ) {
            hvl_query_connection = nt::get_export( m_module_base, oxorany( "HvlQueryConnection" ) );
            if ( !hvl_query_connection ) return {};
        }

        using function_t = nt_status_t( );
        return reinterpret_cast< function_t* >( hvl_query_connection )( );
    }

    teb_t* ps_get_current_thread_teb( ) {
        static auto ps_get_current_thread_teb = 0ull;
        if ( !ps_get_current_thread_teb ) {
            ps_get_current_thread_teb = nt::get_export( m_module_base, oxorany( "PsGetCurrentThreadTeb" ) );
            if ( !ps_get_current_thread_teb ) return {};
        }

        using function_t = teb_t * ( );
        return reinterpret_cast< function_t* >( ps_get_current_thread_teb )( );
    }

    bool ki_is_process_termination_requested( kthread_t* thread, nt_status_t* termination_status ) {
        static auto ki_is_process_termination_requested = 0ull;
        if ( !ki_is_process_termination_requested ) {
            ki_is_process_termination_requested = m_pdb_symbols.m_ki_is_process_termination_requested;
            if ( !ki_is_process_termination_requested ) return false;
        }

        using function_t = bool( kthread_t*, nt_status_t* );
        return reinterpret_cast< function_t* >( ki_is_process_termination_requested )( thread, termination_status );
    }

    std::uint32_t get_previous_mode_offset( ) {
        static std::uint32_t previous_mode_offset = 0;
        if ( !previous_mode_offset ) {
            auto ex_get_previous_mode = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "ExGetPreviousMode" ) )
                );

            if ( !ex_get_previous_mode )
                return 0;

            while ( !( ex_get_previous_mode[ 0 ] == 0x0F &&
                ex_get_previous_mode[ 1 ] == 0xB6 &&
                ex_get_previous_mode[ 2 ] == 0x80 ) )
                ex_get_previous_mode++;

            previous_mode_offset = *reinterpret_cast< std::uint32_t* >( ex_get_previous_mode + 3 );
        }

        return previous_mode_offset;
    }

    std::uint8_t ex_get_previous_mode( ) {
        static std::uint64_t ex_get_previous_mode = 0;
        if ( !ex_get_previous_mode ) {
            ex_get_previous_mode = nt::get_export( m_module_base, oxorany( "ExGetPreviousMode" ) );
            if ( !ex_get_previous_mode )
                return 0;
        }

        using function_t = std::uint8_t( );
        return reinterpret_cast< function_t* >( ex_get_previous_mode )( );
    }

    std::uint32_t get_thread_to_process_offset( ) {
        static std::uint32_t thread_to_process_offset = 0;
        if ( !thread_to_process_offset ) {
            auto io_thread_to_process = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "IoThreadToProcess" ) )
                );

            if ( !io_thread_to_process )
                return 0;

            while ( !( io_thread_to_process[ 0 ] == 0x48 &&
                io_thread_to_process[ 1 ] == 0x8B &&
                io_thread_to_process[ 2 ] == 0x81 ) )
                io_thread_to_process++;

            thread_to_process_offset = *reinterpret_cast< std::uint32_t* >( io_thread_to_process + 3 );
        }

        return thread_to_process_offset;
    }

    std::uint32_t get_section_base_address_offset( ) {
        static std::uint32_t section_base_address_offset = 0;
        if ( !section_base_address_offset ) {
            auto ps_get_process_section_base_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsGetProcessSectionBaseAddress" ) )
                );

            if ( !ps_get_process_section_base_address )
                return { };

            while ( !( ps_get_process_section_base_address[ 0 ] == 0x48 ||
                ps_get_process_section_base_address[ 1 ] == 0x8B ||
                ps_get_process_section_base_address[ 2 ] == 0x81 ) )
                ps_get_process_section_base_address++;

            section_base_address_offset = *reinterpret_cast< std::uint32_t* >( ps_get_process_section_base_address + 3 );
        }

        return section_base_address_offset;
    }

    std::uint32_t get_process_id_offset( ) {
        static std::uint32_t process_id_offset = 0;
        if ( !process_id_offset ) {
            auto ps_get_process_id = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsGetProcessId" ) )
                );

            if ( !ps_get_process_id )
                return { };

            while ( !( ps_get_process_id[ 0 ] == 0x48 ||
                ps_get_process_id[ 1 ] == 0x8B ||
                ps_get_process_id[ 2 ] == 0x81 ) )
                ps_get_process_id++;

            process_id_offset = *reinterpret_cast< std::uint32_t* >( ps_get_process_id + 3 );
        }

        return process_id_offset;
    }

    std::uint32_t get_process_peb_offset( ) {
        static std::uint32_t process_peb_offset = 0;
        if ( !process_peb_offset ) {
            auto ps_get_process_peb = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsGetProcessPeb" ) )
                );

            if ( !ps_get_process_peb )
                return { };

            while ( !( ps_get_process_peb[ 0 ] == 0x48 ||
                ps_get_process_peb[ 1 ] == 0x8B ||
                ps_get_process_peb[ 2 ] == 0x81 ) )
                ps_get_process_peb++;

            process_peb_offset = *reinterpret_cast< std::uint32_t* >( ps_get_process_peb + 3 );
        }

        return process_peb_offset;
    }

    std::uint32_t get_thread_process_id_offset( ) {
        static std::uint32_t thread_process_id_offset = 0;
        if ( !thread_process_id_offset ) {
            auto ps_get_process_peb = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsGetThreadProcessId" ) )
                );

            if ( !ps_get_process_peb )
                return { };

            while ( !( ps_get_process_peb[ 0 ] == 0x48 ||
                ps_get_process_peb[ 1 ] == 0x8B ||
                ps_get_process_peb[ 2 ] == 0x81 ) )
                ps_get_process_peb++;

            thread_process_id_offset = *reinterpret_cast< std::uint32_t* >( ps_get_process_peb + 3 );
        }

        return thread_process_id_offset;
    }


    std::uint32_t get_image_file_name_offset( ) {
        static std::uint32_t image_file_name_offset = 0;
        if ( !image_file_name_offset ) {
            auto ps_get_process_image_file_name = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsGetProcessImageFileName" ) )
                );

            if ( !ps_get_process_image_file_name ) {
                return 0;
            }

            while ( !( ps_get_process_image_file_name[ 0 ] == 0x48 &&
                ps_get_process_image_file_name[ 1 ] == 0x8D &&
                ps_get_process_image_file_name[ 2 ] == 0x81 ) ) {
                ps_get_process_image_file_name++;
            }

            image_file_name_offset = *reinterpret_cast< std::uint32_t* >( ps_get_process_image_file_name + 3 );
        }

        return image_file_name_offset;
    }

    std::uint8_t* get_ki_cpu_tracing_flags( ) {
        std::uint8_t* ki_cpu_tracing_flags_ptr = nullptr;
        if ( !ki_cpu_tracing_flags_ptr ) {
            auto ke_bug_check_ex = nt::get_export( m_module_base, oxorany( "KeBugCheckEx" ) );
            if ( !ke_bug_check_ex ) {
                return { };
            }

            std::uint64_t ki_cpu_tracing_flags = ke_bug_check_ex;
            while ( *reinterpret_cast< unsigned short* >( ki_cpu_tracing_flags ) != 0xE800 ) ki_cpu_tracing_flags++; ki_cpu_tracing_flags += 2;
            while ( *reinterpret_cast< unsigned short* >( ki_cpu_tracing_flags ) != 0xE800 ) ki_cpu_tracing_flags++; ki_cpu_tracing_flags++;
            ki_cpu_tracing_flags = ( ki_cpu_tracing_flags + 5 ) + *reinterpret_cast< int* >( ki_cpu_tracing_flags + 1 );
            while ( *reinterpret_cast< unsigned short* >( ki_cpu_tracing_flags ) != 0x05F7 ) ki_cpu_tracing_flags++;
            ki_cpu_tracing_flags = ( ki_cpu_tracing_flags + 10 ) + *reinterpret_cast< int* >( ki_cpu_tracing_flags + 2 );
            ki_cpu_tracing_flags_ptr = reinterpret_cast< std::uint8_t* >( ki_cpu_tracing_flags );
            if ( !ki_cpu_tracing_flags_ptr ) {
                return { };
            }
        }

        return ki_cpu_tracing_flags_ptr;
    }

    std::uint32_t* get_ki_dynamic_trace_mask_ptr( ) {
        static std::uint32_t* ki_dynamic_trace_mask_ptr = nullptr;
        if ( !ki_dynamic_trace_mask_ptr ) {
            auto ki_dynamic_trace_mask = m_pdb_symbols.m_ki_dynamic_trace_mask;
            ki_dynamic_trace_mask_ptr = reinterpret_cast< std::uint32_t* >( ki_dynamic_trace_mask );
        }

        return ki_dynamic_trace_mask_ptr;
    }

    std::uint32_t rand( ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "RtlRandomEx" ) );
            if ( !fn_address ) return {};
        }

        unsigned int low = *reinterpret_cast< unsigned int* >( 0xFFFFF78000000000 );
        unsigned int mul = *reinterpret_cast< unsigned int* >( 0xFFFFF78000000004 );
        auto seed = ( ( std::uint64_t )( low ) * ( uint64_t )( mul ) ) >> 24;

        using function_t = std::uint32_t( unsigned long* );
        return reinterpret_cast< function_t* >( fn_address )( ( unsigned long* )&seed );
    }

    std::uint32_t rtl_random_ex( unsigned long* seed ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "RtlRandomEx" ) );
            if ( !fn_address ) return {};
        }

        using function_t = std::uint32_t( unsigned long* );
        return reinterpret_cast< function_t* >( fn_address )( seed );
    }

    __int64 ps_query_thread_start_address( ethread_t* a1 ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = m_pdb_symbols.m_ps_query_thread_start_address;
            if ( !fn_address ) {
                return { };
            }
        }

        using function_t = __int64( ethread_t*, int );
        return reinterpret_cast< function_t* >( fn_address )( a1, 0 );
    }

    __int64 ps_enum_process_threads( eprocess_t* process, __int64( *callback )( ethread_t*, void*, void* ), void* parameter ) {
        static auto ps_enum_process_threads = 0ull;
        if ( !ps_enum_process_threads ) {
            std::uint64_t page_section_base;
            std::uint64_t page_section_size;
            if ( !nt::get_section( oxorany( "PAGE" ), &page_section_base, &page_section_size ) ) {
                return { };
            }

            ps_enum_process_threads = m_pdb_symbols.m_ps_enum_process_threads;
            if ( !ps_enum_process_threads ) {
                return { };
            }
        }

        using function_t = __int64( eprocess_t*, __int64( __fastcall* )( ethread_t*, void*, void* ), void* );
        return reinterpret_cast< function_t* >( ps_enum_process_threads )( process, callback, parameter );
    }

    std::uint32_t ke_query_max_processor_count( ) {
        static auto fn_ke_query_max_processor_count = 0ull;
        if ( !fn_ke_query_max_processor_count ) {
            fn_ke_query_max_processor_count = nt::get_export( m_module_base, oxorany( "KeQueryMaximumProcessorCountEx" ) );
            if ( !fn_ke_query_max_processor_count ) return {};
        }

        using function_t = std::uint32_t( );
        return reinterpret_cast< function_t* >( fn_ke_query_max_processor_count )( );
    }

    std::uint32_t ke_get_current_processor_number( ) {
        static auto fn_ke_get_current_processor_number = 0ull;
        if ( !fn_ke_get_current_processor_number ) {
            fn_ke_get_current_processor_number = nt::get_export( m_module_base, oxorany( "KeGetCurrentProcessorNumberEx" ) );
            if ( !fn_ke_get_current_processor_number ) return {};
        }

        using function_t = std::uint32_t( __int64 );
        return reinterpret_cast< function_t* >( fn_ke_get_current_processor_number )( 0 );
    }

    std::uint32_t ke_query_active_processor_count_ex( ) {
        static auto ke_query_active_processor_count_ex = 0ull;
        if ( !ke_query_active_processor_count_ex ) {
            ke_query_active_processor_count_ex = nt::get_export( m_module_base, oxorany( "KeQueryActiveProcessorCountEx" ) );
            if ( !ke_query_active_processor_count_ex ) return {};
        }

        using function_t = std::uint32_t( std::uint32_t );
        return reinterpret_cast< function_t* >( ke_query_active_processor_count_ex )( 0xFFFF );
    }

    eprocess_t* io_get_current_process( ) {
        static auto io_get_current_process = 0ull;
        if ( !io_get_current_process ) {
            io_get_current_process = nt::get_export( m_module_base, oxorany( "IoGetCurrentProcess" ) );
            if ( !io_get_current_process ) return {};
        }

        using function_t = eprocess_t * ( );
        return reinterpret_cast< function_t* >( io_get_current_process )( );
    }

    eprocess_t* ps_get_current_process( ) {
        static auto ps_get_current_process = 0ull;
        if ( !ps_get_current_process ) {
            ps_get_current_process = nt::get_export( m_module_base, oxorany( "PsGetCurrentProcess" ) );
            if ( !ps_get_current_process ) return {};
        }

        using function_t = eprocess_t * ( );
        return reinterpret_cast< function_t* >( ps_get_current_process )( );
    }

    void* ke_register_nmi_callback( auto callback_routine, void* context ) {
        static auto ke_register_nmi_callback = 0ull;
        if ( !ke_register_nmi_callback ) {
            ke_register_nmi_callback = nt::get_export( m_module_base, oxorany( "KeRegisterNmiCallback" ) );
            if ( !ke_register_nmi_callback ) return {};
        }

        using function_t = void* ( PNMI_CALLBACK, void* );
        return reinterpret_cast< function_t* >( ke_register_nmi_callback )( reinterpret_cast< PNMI_CALLBACK >( callback_routine ), context );
    }

    void ke_initialize_affinity_ex( kaffinity_ex_t* affinity ) {
        static auto ke_initialize_affinity_ex = 0ull;
        if ( !ke_initialize_affinity_ex ) {
            ke_initialize_affinity_ex = nt::get_export( m_module_base, oxorany( "KeInitializeAffinityEx" ) );
            if ( !ke_initialize_affinity_ex ) return;
        }

        using function_t = void( kaffinity_ex_t* affinity );
        reinterpret_cast< function_t* >( ke_initialize_affinity_ex )( affinity );
    }

    void ke_add_processor_affinity_ex( kaffinity_ex_t* affinity, int num ) {
        static auto ke_add_processor_affinity_ex = 0ull;
        if ( !ke_add_processor_affinity_ex ) {
            ke_add_processor_affinity_ex = nt::get_export( m_module_base, oxorany( "KeAddProcessorAffinityEx" ) );
            if ( !ke_add_processor_affinity_ex ) return;
        }

        using function_t = void( kaffinity_ex_t*, int );
        reinterpret_cast< function_t* >( ke_add_processor_affinity_ex )( affinity, num );
    }

    void hal_send_nmi( kaffinity_ex_t* affinity ) {
        static auto hal_send_nmi = 0ull;
        if ( !hal_send_nmi ) {
            hal_send_nmi = nt::get_export( m_module_base, oxorany( "HalSendNMI" ) );
            if ( !hal_send_nmi ) return;
        }

        using function_t = void( kaffinity_ex_t* );
        reinterpret_cast< function_t* >( hal_send_nmi )( affinity );
    }

    object_type_t* ex_callback_object_type( ) {
        static auto ex_callback_object_type_ptr = 0ull;
        if ( !ex_callback_object_type_ptr ) {
            ex_callback_object_type_ptr = nt::get_export( m_module_base, oxorany( "ExCallbackObjectType" ) );
            if ( !ex_callback_object_type_ptr ) return nullptr;
        }

        return *reinterpret_cast< object_type_t** >( ex_callback_object_type_ptr );
    }

    object_type_t* io_file_object_type( ) {
        static auto io_file_object_type_ptr = 0ull;
        if ( !io_file_object_type_ptr ) {
            io_file_object_type_ptr = nt::get_export( m_module_base, oxorany( "IoFileObjectType" ) );
            if ( !io_file_object_type_ptr ) return nullptr;
        }

        return *reinterpret_cast< object_type_t** >( io_file_object_type_ptr );
    }

    object_type_t* io_device_object_type( ) {
        static auto io_device_object_type_ptr = 0ull;
        if ( !io_device_object_type_ptr ) {
            io_device_object_type_ptr = nt::get_export( m_module_base, oxorany( "IoDeviceObjectType" ) );
            if ( !io_device_object_type_ptr ) return nullptr;
        }

        return *reinterpret_cast< object_type_t** >( io_device_object_type_ptr );
    }

    object_type_t* ps_process_type( ) {
        static auto ps_process_type = 0ull;
        if ( !ps_process_type ) {
            ps_process_type = nt::get_export( m_module_base, oxorany( "PsProcessType" ) );
            if ( !ps_process_type ) return nullptr;
        }

        return *reinterpret_cast< object_type_t** >( ps_process_type );
    }

    __int64 __fastcall mi_lock_page_table_page( mmpfn_t* pfn_entry, int a2 ) {
        static auto mi_lock_page_table_page = 0ull;
        if ( !mi_lock_page_table_page ) {

        }

        using function_t = __int64( mmpfn_t*, int );
        return reinterpret_cast< function_t* >( mi_lock_page_table_page )( pfn_entry, a2 );
    }

    void ob_un_register_callbacks( void* handle ) {
        static auto ob_un_register_callbacks = 0ull;
        if ( !ob_un_register_callbacks ) {
            ob_un_register_callbacks = nt::get_export( m_module_base, oxorany( "ObUnRegisterCallbacks" ) );
            if ( !ob_un_register_callbacks ) return;
        }

        using function_t = nt_status_t( void* );
        reinterpret_cast< function_t* >( ob_un_register_callbacks )( handle );
    }

    nt_status_t ke_deregister_nmi_callback( void* handle ) {
        static auto ke_deregister_nmi_callback = 0ull;
        if ( !ke_deregister_nmi_callback ) {
            ke_deregister_nmi_callback = nt::get_export( m_module_base, oxorany( "KeDeregisterNmiCallback" ) );
            if ( !ke_deregister_nmi_callback ) return {};
        }

        using function_t = nt_status_t( void* );
        return reinterpret_cast< function_t* >( ke_deregister_nmi_callback )( handle );
    }

    std::uint64_t get_kprcb( int processor ) {
        static auto fn_ke_get_prcb = 0ull;
        if ( !fn_ke_get_prcb ) {
            fn_ke_get_prcb = m_pdb_symbols.m_ke_get_prcb;
            if ( !fn_ke_get_prcb ) return {};
        }

        using function_t = std::uint64_t( int );
        return reinterpret_cast< function_t* >( fn_ke_get_prcb )( processor );
    }

    eprocess_t* io_thread_to_process( ethread_t* thread ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "IoThreadToProcess" ) );
            if ( !fn_address ) return {};
        }

        using function_t = eprocess_t * ( ethread_t* );
        return reinterpret_cast< function_t* >( fn_address )( thread );
    }

    ethread_t* ps_get_current_thread( ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "PsGetCurrentThread" ) );
            if ( !fn_address ) return {};
        }

        using function_t = ethread_t * ( );
        return reinterpret_cast< function_t* >( fn_address )( );
    }

    void* mm_get_system_routine_address( unicode_string_t* system_routine_name ) {
        static auto mm_get_system_routine_address = 0ull;
        if ( !mm_get_system_routine_address ) {
            mm_get_system_routine_address = nt::get_export( m_module_base, oxorany( "MmGetSystemRoutineAddress" ) );
            if ( !mm_get_system_routine_address ) return {};
        }

        using function_t = void* ( unicode_string_t* );
        return reinterpret_cast< function_t* >( mm_get_system_routine_address )( system_routine_name );
    }

    kthread_t* ke_get_current_thread( ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeGetCurrentThread" ) );
            if ( !fn_address ) return {};
        }

        using function_t = kthread_t * ( );
        return reinterpret_cast< function_t* >( fn_address )( );
    }

    nt_status_t zw_allocate_virtual_memory(
        HANDLE process_handle,
        std::uint64_t* base_address,
        PSIZE_T region_size,
        ULONG allocation_type,
        ULONG protect ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "ZwAllocateVirtualMemory" ) );
            if ( !fn_address ) return {};
        }

        void* alloc_address;
        using function_t = nt_status_t( HANDLE, void**, ULONG_PTR, PSIZE_T, ULONG, ULONG );
        auto result = reinterpret_cast< function_t* >( fn_address )
            ( process_handle, &alloc_address, 0, region_size, allocation_type, protect );

        if ( base_address )
            *base_address = reinterpret_cast
            < std::uint64_t >( alloc_address );

        return result;
    }

    std::uint64_t ke_ipi_generic_call( kipi_broadcast_worker* ipi_broadcast_worker, std::uint64_t argument ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeIpiGenericCall" ) );
            if ( !fn_address ) return {};
        }

        using function_t = std::uint64_t( kipi_broadcast_worker*, std::uint64_t );
        return reinterpret_cast< function_t* >( fn_address )( ipi_broadcast_worker, argument );
    }

    void ke_clear_event( kevent_t* event ) {
        static auto ke_clear_event = 0ull;
        if ( !ke_clear_event ) {
            ke_clear_event = nt::get_export( m_module_base, oxorany( "KeClearEvent" ) );
            if ( !ke_clear_event ) return;
        }

        using function_t = void( kevent_t* );
        reinterpret_cast< function_t* >( ke_clear_event )( event );
    }

    void rtl_capture_context( context_t* context ) {
        static auto rtl_capture_context = 0ull;
        if ( !rtl_capture_context ) {
            rtl_capture_context = nt::get_export( m_module_base, oxorany( "RtlCaptureContext" ) );
            if ( !rtl_capture_context ) return;
        }

        using function_t = void( context_t* );
        reinterpret_cast< function_t* >( rtl_capture_context )( context );
    }

    void ke_set_affinity_thread( kthread_t* thread, kaffinity_ex_t* affinity ) {
        static auto ke_set_affinity_thread = 0ull;
        if ( !ke_set_affinity_thread ) {
            ke_set_affinity_thread = nt::get_export( m_module_base, oxorany( "KeSetAffinityThread" ) );
            if ( !ke_set_affinity_thread ) return;
        }

        using function_t = void( kthread_t*, kaffinity_ex_t* );
        reinterpret_cast< function_t* >( ke_set_affinity_thread )( thread, affinity );
    }

    void ke_query_affinity_thread( kthread_t* thread, kaffinity_ex_t* affinity ) {
        static auto ke_query_affinity_thread = 0ull;
        if ( !ke_query_affinity_thread ) {
            ke_query_affinity_thread = nt::get_export( m_module_base, oxorany( "KeQueryAffinityThread" ) );
            if ( !ke_query_affinity_thread ) return;
        }

        using function_t = void( kthread_t*, kaffinity_ex_t* );
        reinterpret_cast< function_t* >( ke_query_affinity_thread )( thread, affinity );
    }

    void ke_initialize_event( kevent_t* event, long type, bool state ) {
        static auto ke_initialize_event = 0ull;
        if ( !ke_initialize_event ) {
            ke_initialize_event = nt::get_export( m_module_base, oxorany( "KeInitializeEvent" ) );
            if ( !ke_initialize_event ) return;
        }

        using function_t = void( kevent_t*, long, bool );
        reinterpret_cast< function_t* >( ke_initialize_event )( event, type, state );
    }

    void ke_initialize_mutex( kmutant_t* mutex, std::uint32_t level ) {
        static auto ke_initialize_mutex = 0ull;
        if ( !ke_initialize_mutex ) {
            ke_initialize_mutex = nt::get_export( m_module_base, oxorany( "KeInitializeMutex" ) );
            if ( !ke_initialize_mutex ) return;
        }

        using function_t = void( kmutant_t*, std::uint32_t );
        reinterpret_cast< function_t* >( ke_initialize_mutex )( mutex, level );
    }

    long ke_release_mutex( kmutant_t* mutex, bool wait ) {
        static auto ke_release_mutex = 0ull;
        if ( !ke_release_mutex ) {
            ke_release_mutex = nt::get_export( m_module_base, oxorany( "KeReleaseMutex" ) );
            if ( !ke_release_mutex ) return {};
        }

        using function_t = long( kmutant_t*, bool );
        return reinterpret_cast< function_t* >( ke_release_mutex )( mutex, wait );
    }

    kprcb_t* ke_get_current_prcb( ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeGetCurrentPrcb" ) );
            if ( !fn_address ) return {};
        }

        using function_t = kprcb_t * ( );
        return reinterpret_cast< function_t* >( fn_address )( );
    }

    int rtl_compare_unicode_string(
        unicode_string_t* string1,
        unicode_string_t* string2,
        bool case_insensitive
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "RtlCompareUnicodeString" ) );
            if ( !fn_address ) return 0;
        }

        using function_t = int( * )(
            unicode_string_t* string1,
            unicode_string_t* string2,
            bool case_insensitive
            );

        return reinterpret_cast< function_t >( fn_address )(
            string1,
            string2,
            case_insensitive
            );
    }

    nt_status_t ps_set_create_thread_notify_routine( p_create_thread_notify_routine callback_routine ) {
        static std::uint8_t* fn_address = nullptr;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsSetCreateThreadNotifyRoutine" ) )
                );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( __stdcall* )( p_create_thread_notify_routine );
        auto fn = reinterpret_cast< function_t >( fn_address );

        return fn( callback_routine );
    }

    void* ke_register_bound_callback( p_bound_callback callback_routine ) {
        static std::uint8_t* fn_address = nullptr;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "KeRegisterBoundCallback" ) )
                );
            if ( !fn_address ) return {};
        }

        using function_t = void* ( __stdcall* )( p_bound_callback );
        auto fn = reinterpret_cast< function_t >( fn_address );

        return fn( callback_routine );
    }

    nt_status_t ps_set_load_image_notify_routine( p_load_image_notify_routine callback_routine ) {
        static std::uint8_t* fn_address = nullptr;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsSetLoadImageNotifyRoutine" ) )
                );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( __stdcall* )( p_load_image_notify_routine );
        auto fn = reinterpret_cast< function_t >( fn_address );
        return fn( callback_routine );
    }

    nt_status_t ps_set_create_process_notify_routine( p_create_process_notify_routine callback_routine, bool remove ) {
        static std::uint8_t* fn_address = nullptr;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsSetCreateProcessNotifyRoutine" ) )
                );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( __stdcall* )( p_create_process_notify_routine, BOOLEAN );
        auto fn = reinterpret_cast< function_t >( fn_address );

        return fn( callback_routine, remove ? TRUE : FALSE );
    }

    std::uint64_t exf_release_push_lock( std::uint64_t* spin_lock ) {
        static std::uint8_t* fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "ExfReleasePushLock" ) ) );
            if ( !fn_address ) return { };
        }

        using function_t = std::uint64_t( std::uint64_t* );
        return reinterpret_cast< function_t* >( fn_address )( spin_lock );
    }

    std::uint64_t* ke_ab_post_release( std::uint64_t spin_lock ) {
        static std::uint8_t* fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "ExFetchLicenseData" ) ) );
            if ( !fn_address ) return { };
        }

        while ( fn_address[ 0x0 ] != 0xE8
            || fn_address[ 0x1 ] != 0x5F
            || fn_address[ 0x2 ] != 0x6F )
            fn_address++;

        auto rel = *reinterpret_cast< std::int32_t* >( fn_address + 1 );
        auto ke_ab_preacquire = reinterpret_cast< std::uint64_t >( fn_address + 5 + rel );

        using function_t = std::uint64_t* ( std::uint64_t );
        return reinterpret_cast< function_t* >( ke_ab_preacquire )( spin_lock );
    }

    std::uint64_t* exf_acquire_push_lock_exclusive_ex( std::uint64_t* spin_lock, std::uint64_t* thread_lock, std::uint64_t spin_lock2 ) {
        static std::uint8_t* fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "ExfAcquirePushLockExclusive" ) ) );
            if ( !fn_address ) return { };
        }

        while ( fn_address[ 0x0 ] != 0xE8 )
            fn_address++;

        std::int32_t relative_offset = *reinterpret_cast< std::int32_t* >( fn_address + 1 );
        std::uint64_t exf_acquire_push_lock_exclusive_ex = reinterpret_cast< std::uint64_t >( fn_address + 5 + relative_offset );

        using function_t = std::uint64_t* ( std::uint64_t*, std::uint64_t*, std::uint64_t );
        return reinterpret_cast< function_t* >( exf_acquire_push_lock_exclusive_ex )( spin_lock, thread_lock, spin_lock2 );
    }

    std::uint64_t* ke_ab_pre_acquire( std::uint64_t* spin_lock, int priority, char a3 ) {
        static std::uint8_t* fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsRegisterAltSystemCallHandler" ) ) );
            if ( !fn_address ) return { };
        }

        while ( fn_address[ 0x0 ] != 0xE8
            || fn_address[ 0x1 ] != 0x85
            || fn_address[ 0x2 ] != 0xC0 )
            fn_address++;

        std::int32_t relative_offset = *reinterpret_cast< std::int32_t* >( fn_address + 1 );
        std::uint64_t ke_ab_preacquire = reinterpret_cast< std::uint64_t >( fn_address + 5 + relative_offset );

        using function_t = std::uint64_t* ( std::uint64_t*, int, char );
        return reinterpret_cast< function_t* >( ke_ab_preacquire )( spin_lock, priority, a3 );
    }

    nt_status_t ob_close_handle( void* handle, std::uint8_t previous_mode ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "ObCloseHandle" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t( void*, std::uint8_t );
        return reinterpret_cast< function_t* >( fn_address )( handle, previous_mode );
    }

    nt_status_t zw_set_information_process( void* handle, int process_information_class, void* process_information, std::uint32_t process_information_length ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "ZwSetInformationProcess" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t( void*, int, void*, std::uint32_t );
        return reinterpret_cast< function_t* >( fn_address )( handle, process_information_class, process_information, process_information_length );
    }

    nt_status_t zw_open_process( void* handle, std::uint8_t desired_access, object_attributes_t* object_attributes, client_id_t* client_id ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "ZwOpenProcess" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t( void*, std::uint8_t, object_attributes_t*, client_id_t* );
        return reinterpret_cast< function_t* >( fn_address )( handle, desired_access, object_attributes, client_id );
    }

    std::uint64_t get_ps_alt_system_call_registeration_lock( ) {
        static std::uint8_t* fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsRegisterAltSystemCallHandler" ) ) );
            if ( !fn_address ) return { };
        }

        while ( fn_address[ 0x0 ] != 0x4C
            || fn_address[ 0x1 ] != 0x8D
            || fn_address[ 0x2 ] != 0x35 )
            fn_address++;

        return *reinterpret_cast< std::uint64_t* >(
            &fn_address[ 0x7 ] + *reinterpret_cast< std::int32_t* >( &fn_address[ 0x3 ] ) );
    }

    std::uint64_t get_ps_alt_system_call_handlers( ) {
        static std::uint8_t* fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "PsRegisterAltSystemCallHandler" ) ) );
            if ( !fn_address ) return { };
        }

        while ( fn_address[ 0x0 ] != 0x48
            || fn_address[ 0x1 ] != 0x8B
            || fn_address[ 0x2 ] != 0x3D )
            fn_address++;

        return *reinterpret_cast< std::uint64_t* >(
            &fn_address[ 0x7 ] + *reinterpret_cast< std::int32_t* >( &fn_address[ 0x3 ] ) );
    }

    mmpfn_t* get_mm_pfn_database( ) {
        static std::uint8_t* fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = reinterpret_cast< std::uint8_t* >(
                nt::get_export( m_module_base, oxorany( "KeCapturePersistentThreadState" ) ) );
            if ( !fn_address ) return { };
        }

        while ( fn_address[ 0x0 ] != 0x48
            || fn_address[ 0x1 ] != 0x8B
            || fn_address[ 0x2 ] != 0x05 )
            fn_address++;

        return *reinterpret_cast< mmpfn_t** >(
            &fn_address[ 0x7 ] + *reinterpret_cast< std::int32_t* >( &fn_address[ 0x3 ] ) );
    }

    physical_memory_range_t* mm_get_physical_memory_ranges( ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmGetPhysicalMemoryRanges" ) );
            if ( !fn_address ) return nullptr;
        }

        using function_t = physical_memory_range_t * ( void );
        return reinterpret_cast< function_t* >( fn_address )( );
    }

    std::uintptr_t hal_private_dispatch_table( ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "HalPrivateDispatchTable" ) );
            if ( !fn_address ) return 0;
        }

        return *reinterpret_cast< std::uintptr_t* >( fn_address );
    }

    void* ps_get_process_section_base_address( eprocess_t* process ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "PsGetProcessSectionBaseAddress" ) );
            if ( !fn_address ) return nullptr;
        }

        using function_t = void* (
            eprocess_t* process
            );

        return reinterpret_cast< function_t* >( fn_address )( process );
    }

    void ke_unstack_detach_process(
        kapc_state_t* apc_state
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeUnstackDetachProcess" ) );
            if ( !fn_address ) return;
        }

        using function_t = void(
            kapc_state_t* apc_state
            );

        reinterpret_cast< function_t* >( fn_address ) ( apc_state );
    }

    hal_private_dispatch_t* get_hal_private_dispatch( ) {
        static hal_private_dispatch_t* hal_private_dispatch{ };
        if ( !hal_private_dispatch ) {
            hal_private_dispatch = reinterpret_cast
                < hal_private_dispatch_t* >( nt::get_export( m_module_base, oxorany( "HalPrivateDispatchTable" ) ) );
            if ( !hal_private_dispatch )
                return { };
        }

        return hal_private_dispatch;
    }

    void ke_stack_attach_process(
        eprocess_t* process,
        kapc_state_t* apc_state
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeStackAttachProcess" ) );
            if ( !fn_address ) return;
        }

        using function_t = void(
            eprocess_t* process,
            kapc_state_t* apc_state
            );

        reinterpret_cast< function_t* >( fn_address ) (
            process,
            apc_state
            );
    }

    mdl_t* io_allocate_mdl(
        void* virtual_address,
        std::size_t length,
        bool secondary_buffer,
        bool charge_quota,
        iop_irp_t* irp
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "IoAllocateMdl" ) );
            if ( !fn_address ) return {};
        }

        using function_t = mdl_t * (
            void* virtual_address,
            std::size_t length,
            bool secondary_buffer,
            bool charge_quota,
            iop_irp_t* irp
            );

        return reinterpret_cast< function_t* >( fn_address ) (
            virtual_address,
            length,
            secondary_buffer,
            charge_quota,
            irp );
    }

    void mm_build_mdl_for_non_paged_pool(
        mdl_t* mdl
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmBuildMdlForNonPagedPool" ) );
            if ( !fn_address ) return;
        }

        using function_t = void(
            mdl_t* mdl
            );

        reinterpret_cast< function_t* >( fn_address ) ( mdl );
    }


    void* mm_map_locked_pages(
        mdl_t* mdl,
        std::uint8_t access_mode
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmMapLockedPages" ) );
            if ( !fn_address ) return {};
        }

        using function_t = void* (
            mdl_t* mdl,
            std::uint8_t access_mode
            );

        return reinterpret_cast< function_t* >( fn_address ) (
            mdl,
            access_mode );
    }

    void mm_probe_and_lock_pages(
        mdl_t* mdl,
        std::uint8_t access_mode,
        std::uint32_t operation
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmProbeAndLockPages" ) );
            if ( !fn_address ) return;
        }

        using function_t = void(
            mdl_t* mdl,
            std::uint8_t access_mode,
            std::uint32_t operation
            );

        reinterpret_cast< function_t* >( fn_address ) (
            mdl,
            access_mode,
            operation );
    }

    void* mm_map_locked_pages_specify_cache( mdl_t* mdl,
        std::uint8_t access_mode,
        std::uint32_t cache_type,
        void* base_address,
        bool bug_check_on_failure,
        std::uint32_t priority
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmMapLockedPagesSpecifyCache" ) );
            if ( !fn_address ) return {};
        }

        using function_t = void* (
            mdl_t* mdl,
            std::uint8_t access_mode,
            std::uint32_t cache_type,
            void* base_address,
            bool bug_check_on_failure,
            std::uint32_t priority
            );

        return reinterpret_cast< function_t* >( fn_address ) (
            mdl,
            access_mode,
            cache_type,
            base_address,
            bug_check_on_failure,
            priority );
    }

    nt_status_t mm_protect_mdl_system_address( mdl_t* mdl, std::uint32_t new_protect ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmProtectMdlSystemAddress" ) );
            if ( !fn_address ) return {};
        }
        using function_t = nt_status_t(
            mdl_t* mdl,
            std::uint32_t new_protect
        );

        return reinterpret_cast< function_t* >( fn_address ) (
            mdl,
            new_protect );
    }

    void mm_unmap_locked_pages( void* base_address, mdl_t* mdl ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmUnmapLockedPages" ) );
            if ( !fn_address ) return;
        }
        using function_t = void(
            void* base_address,
            mdl_t* mdl
            );
        reinterpret_cast< function_t* >( fn_address ) (
            base_address,
            mdl );
    }

    void mm_unlock_pages( mdl_t* mdl ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmUnlockPages" ) );
            if ( !fn_address ) return;
        }
        using function_t = void( mdl_t* mdl );
        reinterpret_cast< function_t* >( fn_address )( mdl );
    }

    void io_free_mdl( mdl_t* mdl ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "IoFreeMdl" ) );
            if ( !fn_address ) return;
        }
        using function_t = void( mdl_t* mdl );
        reinterpret_cast< function_t* >( fn_address )( mdl );
    }

    void mm_unmap_io_space(
        void* base_address,
        std::size_t number_of_bytes
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmUnmapIoSpace" ) );
            if ( !fn_address ) return;
        }

        using function_t = void(
            void* base_address,
            std::size_t number_of_bytes
            );

        reinterpret_cast< function_t* >( fn_address ) (
            base_address,
            number_of_bytes );
    }

    template<class... args_t>
    std::int8_t dbg_print( const char* format, args_t... va_args ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "DbgPrintEx" ) );
            if ( !fn_address ) return 0;
        }

        using function_t = std::int32_t( std::uint32_t flag, std::uint32_t level,
            const char* format, args_t... va_args );
        return reinterpret_cast< function_t* >( fn_address )( 0, 0, format, va_args... ) ==
            nt_status_t::success;
    }

    void* map_io_space_ex(
        physical_address_t physical_address,
        std::size_t number_of_bytes,
        std::uint32_t protect = 0x04
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmMapIoSpaceEx" ) );
            if ( !fn_address ) return { };
        }

        using function_t = void* (
            physical_address_t physical_address,
            std::size_t number_of_bytes,
            std::uint32_t protect
            );

        return reinterpret_cast< function_t* >( fn_address ) (
            physical_address,
            number_of_bytes,
            protect );
    }

    void ke_flush_entire_tb( bool invalidate, bool all_processors ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeFlushEntireTb" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( bool invalidate, bool all_processors );
        reinterpret_cast< function_t >( fn_address )( invalidate, all_processors );
    }

    void ke_invalidate_all_caches( ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeInvalidateAllCaches" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( );
        reinterpret_cast< function_t >( fn_address )( );
    }

    void ke_flush_single_tb( std::uintptr_t address, bool all_processors, bool invalidate ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "KeFlushSingleTb" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( std::uintptr_t address, bool all_processors, bool invalidate );
        reinterpret_cast< function_t >( fn_address )( address, all_processors, invalidate );
    }

    physical_address_t mm_get_physical_address( void* virtual_address ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmGetPhysicalAddress" ) );
            if ( !fn_address ) return { };
        }

        using function_t = physical_address_t( * )( void* virtual_address );
        return reinterpret_cast< function_t >( fn_address )( virtual_address );
    }

    ethread_t* ps_lookup_thread_by_tid( std::uint32_t thread_id ) {
        static void* ps_lookup_thread_by_tid = nullptr;
        if ( !ps_lookup_thread_by_tid ) {
            ps_lookup_thread_by_tid = reinterpret_cast< void* >(
                nt::get_export( m_module_base,
                    oxorany( "PsLookupThreadByThreadId" )
                )
                );

            if ( !ps_lookup_thread_by_tid )
                return nullptr;
        }

        ethread_t* thread = nullptr;
        using function_t = nt_status_t( * )( HANDLE thread_id, ethread_t** thread );
        auto status = reinterpret_cast< function_t >( ps_lookup_thread_by_tid )(
            reinterpret_cast< HANDLE >( thread_id ),
            &thread
            );

        if ( !status ) {
            return thread;
        }

        return nullptr;
    }

    eprocess_t* ps_lookup_process_by_pid( std::uint32_t process_id ) {
        static void* fn_ps_lookup_process_by_pid = nullptr;
        if ( !fn_ps_lookup_process_by_pid ) {
            fn_ps_lookup_process_by_pid = reinterpret_cast< void* >(
                nt::get_export( m_module_base,
                    oxorany( "PsLookupProcessByProcessId" )
                )
                );

            if ( !fn_ps_lookup_process_by_pid )
                return nullptr;
        }

        eprocess_t* process = nullptr;
        using function_t = nt_status_t( * )( HANDLE process_id, eprocess_t** process );
        auto status = reinterpret_cast< function_t >( fn_ps_lookup_process_by_pid )(
            reinterpret_cast< HANDLE >( process_id ),
            &process
            );

        if ( !status ) {
            return process;
        }

        return nullptr;
    }

    void* mm_get_virtual_for_physical( std::uintptr_t phys_addr ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "MmGetVirtualForPhysical" ) );
            if ( !fn_address ) return nullptr;
        }

        using function_t = void* ( * )( std::uintptr_t physical_address );
        return reinterpret_cast< function_t >( fn_address )( phys_addr );
    }

    void mm_free_non_cached_memory( void* base_address, std::size_t size ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "MmFreeNonCachedMemory" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( void*, std::size_t );
        reinterpret_cast< function_t* >( fn_address )( base_address, size );
    }

    void* mm_allocate_contiguous_memory( std::size_t size, physical_address_t highest_acceptable_address ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "MmAllocateContiguousMemory" ) );
            if ( !fn_address ) return {};
        }

        using function_t = void* ( std::size_t, physical_address_t );
        return reinterpret_cast< function_t* >( fn_address )( size, highest_acceptable_address );
    }

    void* mm_allocate_non_cached_memory( std::size_t size ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "MmAllocateNonCachedMemory" ) );
            if ( !fn_address ) return nullptr;
        }

        using function_t = void* ( std::size_t );
        return reinterpret_cast< function_t* >( fn_address )( size );
    }

    nt_status_t mm_copy_memory(
        void* target_address,
        mm_copy_address_t source_address,
        std::size_t number_of_bytes,
        std::uint32_t flags,
        std::size_t* number_of_bytes_transferred
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "MmCopyMemory" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t(
            void* target_address,
            mm_copy_address_t source_address,
            std::size_t number_of_bytes,
            std::uint32_t flags,
            std::size_t* number_of_bytes_transferred
        );

        return reinterpret_cast< function_t* >( fn_address )(
            target_address,
            source_address,
            number_of_bytes,
            flags,
            number_of_bytes_transferred
            );
    }

    void* mm_get_system_address_for_mdl_safe( void* mdl, std::uint32_t priority ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "MmGetSystemAddressForMdlSafe" ) );
            if ( !fn_address ) return {};
        }
        using function_t = void* ( * )( void* mdl, std::uint32_t priority );
        return reinterpret_cast< function_t >( fn_address )( mdl, priority );
    }

    void iof_complete_request( iop_irp_t* irp, char priority_boost ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IofCompleteRequest" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( iop_irp_t* irp, char priority_boost );
        reinterpret_cast< function_t >( fn_address )( irp, priority_boost );
    }

    std::uint32_t ps_get_process_id( eprocess_t* process ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "PsGetProcessId" ) );
            if ( !fn_address ) return {};
        }

        using function_t = std::uint32_t( eprocess_t* process );
        return reinterpret_cast< function_t* >( fn_address )( process );
    }

    bool mm_is_address_valid( void* virtual_address ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "MmIsAddressValid" ) );
            if ( !fn_address ) return {};
        }

        using function_t = bool( void* virtual_address );
        return reinterpret_cast< function_t* >( fn_address )( virtual_address );
    }

    list_entry_t* ps_active_process_head( ) {
        static list_entry_t* ps_active_process_head = nullptr;
        if ( !ps_active_process_head ) {
            static std::uint8_t* fn_address;
            if ( !fn_address ) {
                fn_address = reinterpret_cast< std::uint8_t* >(
                    get_export( m_module_base, oxorany( "KeCapturePersistentThreadState" ) )
                    );
                if ( !fn_address ) return {};
            }

            while ( fn_address[ 0x0 ] != 0x20
                || fn_address[ 0x1 ] != 0x48
                || fn_address[ 0x2 ] != 0x8d )
                fn_address++;

            ps_active_process_head = *reinterpret_cast< list_entry_t** >
                ( &fn_address[ 0x8 ] + *reinterpret_cast< std::int32_t* >( &fn_address[ 0x4 ] ) );
        }

        return ps_active_process_head;
    }

    std::uintptr_t ps_initial_system_process( ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "PsInitialSystemProcess" ) );
            if ( !fn_address ) return {};
        }

        return *reinterpret_cast< std::uintptr_t* >( fn_address );
    }

    void ke_lower_irql_to_passive( std::uint8_t irql ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "KeLowerIrql" ) );
            if ( !fn_address ) return;
        }

        using ke_lower_irql_t = void( * )( std::uint8_t );
        auto ke_lower_irql = reinterpret_cast< ke_lower_irql_t >( fn_address );
        ke_lower_irql( irql );
    }

    void ke_raise_irql( std::uint8_t new_irql ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "KfRaiseIrql" ) );
            if ( !fn_address ) return;
        }

        using kf_raise_irql_t = std::uint8_t( * )( std::uint8_t );
        auto kf_raise_irql = reinterpret_cast< kf_raise_irql_t >( fn_address );
        kf_raise_irql( new_irql );
    }

    iop_irp_stack_profiler_t* io_get_current_irp_stack_location( iop_irp_t* irp ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoGetCurrentIrpStackLocation" ) );
            if ( !fn_address ) return {};
        }

        using function_t = iop_irp_stack_profiler_t * ( * )( iop_irp_t* irp );
        return reinterpret_cast< function_t >( fn_address )( irp );
    }

    nt_status_t io_create_driver( unicode_string_t* driver_name, pdriver_initialize init ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoCreateDriver" ) );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( * )(
            unicode_string_t* driver_name,
            pdriver_initialize init );

        return reinterpret_cast< function_t >( fn_address )(
            driver_name,
            init
            );
    }

    nt_status_t create_system_thread(
        void** thread_handle,
        pkstart_routine start_routine,
        void* start_context
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "PsCreateSystemThread" ) );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( * )(
            void** thread_handle,
            std::uint32_t desired_access,
            object_attributes_t* object_attributes,
            void* process_handle,
            client_id_t* client_id,
            pkstart_routine start_routine,
            void* start_context
            );

        return reinterpret_cast< function_t >( fn_address )(
            thread_handle,
            THREAD_ALL_ACCESS,
            nullptr,
            nullptr,
            nullptr,
            start_routine,
            start_context
            );
    }

    nt_status_t io_delete_device( device_object_t* device_object ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoDeleteDevice" ) );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( * )(
            device_object_t* device_object
            );

        return reinterpret_cast< function_t >( fn_address )(
            device_object
            );
    }

    nt_status_t io_delete_symbolic_link( unicode_string_t* symbolic_link_name ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoDeleteSymbolicLink" ) );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( * )(
            unicode_string_t* symbolic_link_name
            );

        return reinterpret_cast< function_t >( fn_address )(
            symbolic_link_name
            );
    }

    nt_status_t io_create_symbolic_link(
        unicode_string_t* symbolic_link_name,
        unicode_string_t* device_name
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoCreateSymbolicLink" ) );
            if ( !fn_address ) return {};
        }

        using function_t = nt_status_t( * )(
            unicode_string_t* symbolic_link_name,
            unicode_string_t* device_name
            );

        return reinterpret_cast< function_t >( fn_address )(
            symbolic_link_name,
            device_name
            );
    }

    nt_status_t io_create_device(
        driver_object_t* driver_object,
        unsigned long device_extension_size,
        unicode_string_t* device_name,
        device_type_t device_type,
        unsigned long device_characteristics,
        bool exclusive,
        device_object_t** device_object
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "IoCreateDevice" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t( * )(
            driver_object_t* driver_object,
            unsigned long device_extension_size,
            unicode_string_t* device_name,
            device_type_t device_type,
            unsigned long device_characteristics,
            bool exclusive,
            device_object_t** device_object
            );

        return reinterpret_cast< function_t >( fn_address )(
            driver_object,
            device_extension_size,
            device_name,
            device_type,
            device_characteristics,
            exclusive,
            device_object
            );
    }

    nt_status_t zw_create_file(
        void* file_handle,
        std::uint32_t desired_access,
        object_attributes_t* object_attributes,
        io_status_block_t* io_status,
        ularge_integer_t* allocation_size,
        std::uint32_t file_attributes,
        std::uint32_t share_access,
        std::uint32_t create_disposition,
        std::uint32_t create_options,
        void* ea_buffer,
        std::uint32_t ea_length
    ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ZwCreateFile" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t( * )(
            void*, std::uint32_t, object_attributes_t*, io_status_block_t*, ularge_integer_t*,
            std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, void*, std::uint32_t );
        return reinterpret_cast< function_t >( fn_address )(
            file_handle, desired_access, object_attributes, io_status, allocation_size,
            file_attributes, share_access, create_disposition, create_options, ea_buffer, ea_length );
    }

    nt_status_t zw_write_file(
        void* file_handle,
        void* event,
        void* apc_routine,
        void* apc_context,
        io_status_block_t* io_status_block,
        void* buffer,
        std::uint32_t length,
        ularge_integer_t* byte_offset,
        std::uint32_t* key
    ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ZwWriteFile" ) );
            if ( !fn_address ) return { };
        }
        using function_t = nt_status_t( * )(
            void*, void*, void*, void*, io_status_block_t*,
            void*, std::uint32_t, ularge_integer_t*, std::uint32_t* );
        return reinterpret_cast< function_t >( fn_address )(
            file_handle, event, apc_routine, apc_context, io_status_block,
            buffer, length, byte_offset, key );
    }

    nt_status_t zw_query_information_file(
        void* file_handle,
        io_status_block_t* io_status,
        void* file_information,
        std::uint32_t length,
        std::uint32_t file_information_class
    ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ZwQueryInformationFile" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t( * )( void*, io_status_block_t*, void*, std::uint32_t, std::uint32_t );
        return reinterpret_cast< function_t >( fn_address )( file_handle, io_status, file_information, length, file_information_class );
    }

    int vsnprintf(
        char* buffer,
        std::size_t count,
        const char* format,
        va_list args
    ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "_vsnprintf" ) );
            if ( !fn_address ) return -1;
        }
        using function_t = int( * )( char*, std::size_t, const char*, va_list );
        return reinterpret_cast< function_t >( fn_address )( buffer, count, format, args );
    }

    nt_status_t zw_read_file(
        void* file_handle,
        void* event_handle,
        void* apc_routine,
        void* apc_context,
        io_status_block_t* io_status,
        void* buffer,
        std::uint32_t length,
        ularge_integer_t* byte_offset,
        std::uint32_t* key
    ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ZwReadFile" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t( * )(
            void*, void*, void*, void*, io_status_block_t*, void*, std::uint32_t, ularge_integer_t*, std::uint32_t* );
        return reinterpret_cast< function_t >( fn_address )(
            file_handle, event_handle, apc_routine, apc_context, io_status, buffer, length, byte_offset, key );
    }

    nt_status_t zw_close( void* handle ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ZwClose" ) );
            if ( !fn_address ) return { };
        }

        using function_t = nt_status_t( * )( void* );
        return reinterpret_cast< function_t >( fn_address )( handle );
    }

    void* ex_allocate_pool_with_tag( std::uint32_t pool_type, std::size_t number_of_bytes, std::uint32_t tag ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ExAllocatePoolWithTag" ) );
            if ( !fn_address ) return { };
        }

        using function_t = void* ( * )( std::uint32_t, std::size_t, std::uint32_t );
        return reinterpret_cast< function_t >( fn_address )( pool_type, number_of_bytes, tag );
    }

    void ex_free_pool_with_tag( void* p, std::uint32_t tag ) {
        static std::uint64_t fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "ExFreePoolWithTag" ) );
            if ( !fn_address ) return;
        }

        using function_t = void( * )( void*, std::uint32_t );
        reinterpret_cast< function_t >( fn_address )( p, tag );
    }

    std::uint32_t kprcb_context_offset( ) {
        static std::uint32_t offset = 0;
        if ( !offset ) {
            auto ke_bugcheck_ex = reinterpret_cast< std::uint8_t* >(
                get_export( m_module_base, oxorany( "KeBugCheckEx" ) )
                );
            if ( !ke_bugcheck_ex ) return {};

            auto scan_ptr = ke_bugcheck_ex;
            while ( scan_ptr < ke_bugcheck_ex + 0x100 ) {
                if ( ( scan_ptr[ 0 ] == 0x48 || scan_ptr[ 0 ] == 0x49 ) && scan_ptr[ 1 ] == 0x8B ) {
                    if ( scan_ptr[ 7 ] == 0xE8 ) {
                        offset = *reinterpret_cast< std::uint32_t* >( scan_ptr + 3 );
                        break;
                    }
                }
                scan_ptr++;
            }
        }

        return offset;
    }

    std::uint32_t kprcb_ipi_frozen_offset( ) {
        static std::uint32_t offset = 0;
        if ( !offset ) {
            auto ke_bugcheck_ex = reinterpret_cast< std::uint8_t* >(
                get_export( m_module_base, oxorany( "KeBugCheckEx" ) )
                );
            if ( !ke_bugcheck_ex ) return {};

            // Find KeBugCheck2
            auto scan_ptr = ke_bugcheck_ex;
            std::uint8_t* ke_bugcheck2 = nullptr;

            while ( scan_ptr < ke_bugcheck_ex + 0x200 ) {
                if ( scan_ptr[ 0 ] == 0xE8 ) {
                    auto target = reinterpret_cast< std::uint64_t >( scan_ptr + 5 + *reinterpret_cast< std::int32_t* >( scan_ptr + 1 ) );
                    if ( ( target & 0x3 ) == 0 ) {
                        ke_bugcheck2 = reinterpret_cast< std::uint8_t* >( target );
                        break;
                    }
                }
                scan_ptr++;
            }

            if ( !ke_bugcheck2 ) return {};

            // Find spinloop
            scan_ptr = ke_bugcheck2;
            std::uint8_t* spinloop_begin = nullptr;

            while ( scan_ptr < ke_bugcheck2 + 0x1000 ) {
                if ( !memcmp( scan_ptr, "\xF3\x90\xEB", 3 ) ) {
                    spinloop_begin = scan_ptr + 4 + *reinterpret_cast< char* >( scan_ptr + 3 );
                    break;
                }
                scan_ptr++;
            }

            if ( !spinloop_begin ) return {};

            if ( spinloop_begin[ 0 ] == 0x33 && spinloop_begin[ 4 ] == 0xE8 ) {
                auto pt = spinloop_begin + 9 + *reinterpret_cast< std::int32_t* >( spinloop_begin + 5 );
                while ( pt[ 0 ] != 0x83 || pt[ 2 ] != 0x05 )
                    pt++;

                offset = *reinterpret_cast< std::int32_t* >( pt - 4 );
            }
            else if ( spinloop_begin[ 0 ] == 0x65 && spinloop_begin[ 9 ] == 0x8B ) {
                offset = *reinterpret_cast< std::int32_t* >( spinloop_begin + 0xB );
            }
        }

        return offset;
    }

    std::uint32_t kpcr_debugger_saved_irql_offset( ) {
        static std::uint32_t offset = 0;
        if ( !offset ) {
            auto ke_bugcheck_ex = reinterpret_cast< std::uint8_t* >(
                get_export( m_module_base, oxorany( "KeBugCheckEx" ) )
                );
            if ( !ke_bugcheck_ex ) return {};

            auto scan_ptr = ke_bugcheck_ex;
            while ( scan_ptr < ke_bugcheck_ex + 0x200 ) {
                if ( !memcmp( scan_ptr, "\x65\x88\x04", 3 ) ) {
                    offset = *reinterpret_cast< std::uint32_t* >( scan_ptr + 4 );
                    break;
                }
                scan_ptr++;
            }
        }

        return offset;
    }

    volatile std::int32_t* ki_hardware_trigger( ) {
        static volatile std::int32_t* ptr = nullptr;
        if ( !ptr ) {
            auto ke_bugcheck_ex = reinterpret_cast< std::uint8_t* >(
                get_export( m_module_base, oxorany( "KeBugCheckEx" ) )
                );
            if ( !ke_bugcheck_ex ) return {};

            auto scan_ptr = ke_bugcheck_ex;
            while ( scan_ptr < ke_bugcheck_ex + 0x200 ) {
                if ( !memcmp( scan_ptr, "\xF0\xFF\x05", 3 ) ) {
                    ptr = reinterpret_cast< volatile std::int32_t* >(
                        scan_ptr + 7 + *reinterpret_cast< std::int32_t* >( scan_ptr + 3 )
                        );
                    break;
                }
                scan_ptr++;
            }
        }

        return ptr;
    }

    volatile long* ki_freeze_execution_lock( ) {
        static volatile long* ptr = nullptr;
        if ( !ptr ) {
            auto ke_bugcheck_ex = reinterpret_cast< std::uint8_t* >(
                get_export( m_module_base, oxorany( "KeBugCheckEx" ) )
                );
            if ( !ke_bugcheck_ex ) return {};

            // Find KeBugCheck2
            auto scan_ptr = ke_bugcheck_ex;
            std::uint8_t* ke_bugcheck2 = nullptr;

            while ( scan_ptr < ke_bugcheck_ex + 0x200 ) {
                if ( scan_ptr[ 0 ] == 0xE8 ) {
                    auto target = reinterpret_cast< std::uint64_t >( scan_ptr + 5 + *reinterpret_cast< std::int32_t* >( scan_ptr + 1 ) );
                    if ( ( target & 0x3 ) == 0 ) {
                        ke_bugcheck2 = reinterpret_cast< std::uint8_t* >( target );
                        break;
                    }
                }
                scan_ptr++;
            }

            if ( !ke_bugcheck2 ) return {};

            // Find spinloop
            scan_ptr = ke_bugcheck2;
            std::uint8_t* spinloop_begin = nullptr;

            while ( scan_ptr < ke_bugcheck2 + 0x1000 ) {
                if ( !memcmp( scan_ptr, "\xF3\x90\xEB", 3 ) ) {
                    spinloop_begin = scan_ptr + 4 + *reinterpret_cast< char* >( scan_ptr + 3 );
                    break;
                }
                scan_ptr++;
            }

            if ( !spinloop_begin ) return {};

            std::uint8_t* pt = nullptr;

            if ( spinloop_begin[ 0 ] == 0x33 && spinloop_begin[ 4 ] == 0xE8 ) {
                pt = spinloop_begin + 9 + *reinterpret_cast< std::int32_t* >( spinloop_begin + 5 );

                while ( pt[ 0 ] != 0x83 || pt[ 2 ] != 0x05 )
                    pt++;

                while ( pt[ 0 ] != 0xE8 || pt[ 4 ] != 0x00 )
                    pt++;

                pt = pt + 5 + *reinterpret_cast< std::int32_t* >( pt + 1 );
            }
            else if ( spinloop_begin[ 0 ] == 0x65 && spinloop_begin[ 0x18 ] == 0xE8 ) {
                pt = spinloop_begin + 0x1D + *reinterpret_cast< std::int32_t* >( spinloop_begin + 0x19 );
            }

            if ( !pt ) return {};

            while ( pt ) {
                if ( *reinterpret_cast< std::uint16_t* >( pt ) == 0x394C ) {
                    ptr = reinterpret_cast< volatile long* >(
                        pt + 7 + *reinterpret_cast< long* >( pt + 3 )
                        );
                    break;
                }
                else if ( *reinterpret_cast< std::uint16_t* >( pt ) == 0x8348 ) {
                    ptr = reinterpret_cast< volatile long* >(
                        pt + 8 + *reinterpret_cast< long* >( pt + 3 )
                        );
                    break;
                }
                pt++;
            }
        }

        return ptr;
    }

    void ke_bug_check( std::uint32_t code ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = get_export( m_module_base, oxorany( "KeBugCheck" ) );
            if ( !fn_address ) return;
        }
        using function_t = void( * )( std::uint32_t );
		reinterpret_cast< function_t >( fn_address )( code );
    }

    volatile std::uint32_t* ki_bugcheck_active( ) {
        static volatile std::uint32_t* ptr = nullptr;
        if ( !ptr ) {
            auto ke_bugcheck_ex = reinterpret_cast< std::uint8_t* >(
                get_export( m_module_base, oxorany( "KeBugCheckEx" ) )
                );
            if ( !ke_bugcheck_ex ) return {};

            // Find KeBugCheck2
            auto scan_ptr = ke_bugcheck_ex;
            std::uint8_t* ke_bugcheck2 = nullptr;

            while ( scan_ptr < ke_bugcheck_ex + 0x200 ) {
                if ( scan_ptr[ 0 ] == 0xE8 ) {
                    auto target = reinterpret_cast< std::uint64_t >( scan_ptr + 5 + *reinterpret_cast< std::int32_t* >( scan_ptr + 1 ) );
                    if ( ( target & 0x3 ) == 0 ) {
                        ke_bugcheck2 = reinterpret_cast< std::uint8_t* >( target );
                        break;
                    }
                }
                scan_ptr++;
            }

            if ( !ke_bugcheck2 ) return {};

            scan_ptr = ke_bugcheck2;
            while ( scan_ptr < ke_bugcheck2 + 0x500 ) {
                if ( !memcmp( scan_ptr, "\xF0\x0F\xB1", 3 ) ) {
                    ptr = reinterpret_cast< volatile std::uint32_t* >(
                        scan_ptr + 8 + *reinterpret_cast< std::int32_t* >( scan_ptr + 4 )
                        );
                    break;
                }
                scan_ptr++;
            }
        }

        return ptr;
    }

    context_t* get_processor_context( kprcb_t* prcb = nullptr ) {
        if ( !prcb )
            prcb = reinterpret_cast< kprcb_t* >( __readgsqword( 0x20 ) );

        auto offset = kprcb_context_offset( );
        if ( !offset )
            return nullptr;

        return *reinterpret_cast< context_t** >( reinterpret_cast< std::uint8_t* >( prcb ) + offset );
    }

    kspecial_registers_t* get_processor_state( kprcb_t* prcb = nullptr ) {
        if ( !prcb )
            prcb = reinterpret_cast< kprcb_t* >( __readgsqword( 0x20 ) );

        return reinterpret_cast< kspecial_registers_t* >(
            reinterpret_cast< std::uint8_t* >( prcb ) + 0x40
            );
    }

    std::uint32_t get_processor_index( kprcb_t* prcb = nullptr ) {
        if ( !prcb )
            prcb = reinterpret_cast< kprcb_t* >( __readgsqword( 0x20 ) );

        return *reinterpret_cast< std::uint32_t* >(
            reinterpret_cast< std::uint8_t* >( prcb ) + 0x24
            );
    }

    std::uint32_t& processor_ipi_frozen( kprcb_t* prcb = nullptr ) {
        if ( !prcb )
            prcb = reinterpret_cast< kprcb_t* >( __readgsqword( 0x20 ) );

        auto offset = kprcb_ipi_frozen_offset( );
        return *reinterpret_cast< std::uint32_t* >(
            reinterpret_cast< std::uint8_t* >( prcb ) + offset
            );
    }

    std::uint8_t& processor_debugger_saved_irql( kpcr_t* pcr = nullptr ) {
        if ( !pcr )
            pcr = reinterpret_cast< kpcr_t* >( __readgsqword( 0 ) );

        auto offset = kpcr_debugger_saved_irql_offset( );
        return *reinterpret_cast< std::uint8_t* >(
            reinterpret_cast< std::uint8_t* >( pcr ) + offset
            );
    }

    nt_status_t zw_trace_control(
        std::uint32_t function_code,
        void* in_buffer,
        std::uint32_t in_buffer_len,
        void* out_buffer,
        std::uint32_t out_buffer_len,
        std::uint32_t* return_length
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "ZwTraceControl" ) );
            if ( !fn_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )(
            std::uint32_t,
            void*,
            std::uint32_t,
            void*,
            std::uint32_t,
            std::uint32_t*
            );

        return reinterpret_cast<function_t>( fn_address )(
            function_code,
            in_buffer,
            in_buffer_len,
            out_buffer,
            out_buffer_len,
            return_length
            );
    }

    nt_status_t zw_set_system_information(
        system_information_class_t system_information_class,
        void* system_information,
        std::uint32_t system_information_length
    ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "ZwSetSystemInformation" ) );
            if ( !fn_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )(
            system_information_class_t,
            void*,
            std::uint32_t
            );

        return reinterpret_cast<function_t>( fn_address )(
            system_information_class,
            system_information,
            system_information_length
            );
    }

    nt_status_t nt_set_information_process( void* process_handle, int process_information_class, void* process_information, std::uint32_t process_information_length ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "NtSetInformationProcess" ) );
            if ( !fn_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )( void*, int, void*, std::uint32_t );
        return reinterpret_cast< function_t >( fn_address )( process_handle, process_information_class, process_information, process_information_length );
    }

    nt_status_t zw_terminate_process( void* process_handle, nt_status_t exit_status ) {
        static auto fn_address = 0ull;
        if ( !fn_address ) {
            fn_address = nt::get_export( m_module_base, oxorany( "ZwTerminateProcess" ) );
            if ( !fn_address ) return nt_status_t::unsuccessful;
        }

        using function_t = nt_status_t( * )( void*, nt_status_t );
        return reinterpret_cast< function_t >( fn_address )( process_handle, exit_status );
    }
    std::uint64_t get_exception_offset( ) {
        static std::uint64_t exception_offset = 0;
        if ( !exception_offset ) {
            auto ki_initialize_boot_structures = reinterpret_cast< std::uint8_t* >(
                nt::m_pdb_symbols.m_ki_initialize_boot_structures );
            if ( !ki_initialize_boot_structures )
                return 0;

            while ( !( ki_initialize_boot_structures[ 0 ] == 0x48 &&
                ki_initialize_boot_structures[ 1 ] == 0x89 &&
                ki_initialize_boot_structures[ 2 ] == 0x48 ) )
                ki_initialize_boot_structures++;

            ki_initialize_boot_structures += 3;
            while ( !( ki_initialize_boot_structures[ 0 ] == 0x48 &&
                ki_initialize_boot_structures[ 1 ] == 0x89 &&
                ki_initialize_boot_structures[ 2 ] == 0x87 ) )
                ki_initialize_boot_structures++;

            exception_offset = *reinterpret_cast< std::uint32_t* >( ki_initialize_boot_structures + 3 );
        }
        return exception_offset;
    }
}