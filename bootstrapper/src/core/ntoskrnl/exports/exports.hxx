#pragma once

namespace nt {
	std::uint64_t ps_get_process_image_file_name( eprocess_t* process ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "PsGetProcessImageFileName" ) );
		}

		return g_hook->call_kernel<std::uint64_t>( fn_address, process );
	}

	std::uint32_t ps_get_process_session_id( eprocess_t* process ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "PsGetProcessSessionId" ) );
		}

		return g_hook->call_kernel<std::uint32_t>( fn_address, process );
	}

	bool is_address_valid( void* address ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmIsAddressValid" ) );
		}

		if ( !fn_address )
			return false;

		return g_hook->call_kernel<bool>( fn_address, address );
	}

	void memcpy( void* dst, void* src, std::uint64_t size ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "memcpy" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel( fn_address, dst, src, size );
	}

	void memset( void* dst, int value, std::uint64_t size ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "memset" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel( fn_address, dst, value, size );
	}

	std::uint64_t get_ps_active_process_head( ) {
		static auto function_address = 0ull;
		if ( !function_address ) {
			auto pattern_address = g_ntoskrnl->find_pattern(
				oxorany( "4C 8D 2D ? ? ? ? EB ? 48 8D 97" )
			);

			if ( pattern_address ) {
				function_address = g_ntoskrnl->rva( pattern_address, 3, 7 );
			}

			if ( !function_address )
				return 0;
		}

		std::uint64_t return_address = 0;
		nt::memcpy( &return_address, reinterpret_cast< void* >( function_address ), sizeof( std::uint64_t ) );
		return return_address;
	}

	void* get_pde_address( std::uint64_t address ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiGetPdeAddress" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, address );
	}

	physical_memory_range_t* mm_get_physical_memory_ranges( ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmGetPhysicalMemoryRanges" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<physical_memory_range_t*>( fn_address );
	}

	void* mm_allocate_independent_pages( std::size_t size ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmAllocateIndependentPages" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, size, -1 );
	}

	void mm_free_independent_pages( std::uint64_t independent_pages, std::size_t size ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmFreeIndependentPages" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address, reinterpret_cast< void* >( independent_pages ), size );
	}

	void* get_pte_address( std::uint64_t address ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "GetPteAddress" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, address );
	}

	void* ps_get_process_peb( void* eprocess ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "PsGetProcessPeb" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, eprocess );
	}

	void* ps_lookup_process_by_pid( std::uint32_t process_id ) {
		static void* fn_ps_lookup_process_by_pid = nullptr;
		if ( !fn_ps_lookup_process_by_pid ) {
			fn_ps_lookup_process_by_pid = reinterpret_cast< void* >(
				g_ntoskrnl->get_export(
					oxorany( "PsLookupProcessByProcessId" )
				)
				);
		}

		void* process = nullptr;
		auto status = g_hook->call_kernel<NTSTATUS>(
			fn_ps_lookup_process_by_pid,
			reinterpret_cast<HANDLE>(process_id),
			&process
		);

		if ( !NT_SUCCESS( status ) ) {
			logging::print( oxorany( "Could not lookup process with %d" ) , status );
			return nullptr;
		}

		return process;
	}

	std::uint64_t ps_get_process_section_base_address( void* process ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "PsGetProcessSectionBaseAddress" ) );
		}

		if ( !fn_address )
			return 0;

		return g_hook->call_kernel<std::uint64_t>( fn_address, process );
	}

	bool ex_acquire_resource( void* resource, bool wait ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "ExAcquireResourceExclusiveLite" ) );
		}

		if ( !fn_address )
			return false;

		return g_hook->call_kernel<bool>( fn_address, resource, wait );
	}

	piddb_cache_entry_t* lookup_element_table( avl_table_t* table, void* buffer ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "RtlLookupElementGenericTableAvl" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<piddb_cache_entry_t*>( fn_address, table, buffer );
	}

	void release_resource( void* resource ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "ExReleaseResourceLite" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel( fn_address, resource );
	}

	bool delete_table_entry( avl_table_t* table, void* buffer ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "RtlDeleteElementGenericTableAvl" ) );
		}

		if ( !fn_address )
			return false;

		return g_hook->call_kernel<bool>( fn_address, table, buffer );
	}

	bool rtl_equal_unicode_string( UNICODE_STRING* str1, UNICODE_STRING* str2, bool case_insensitive ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "RtlEqualUnicodeString" ) );
		}

		if ( !fn_address )
			return false;

		return g_hook->call_kernel<bool>( fn_address, str1, str2, case_insensitive );
	}

	void* ex_allocate_pool( std::size_t size ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "ExAllocatePool" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, 0, size );
	}

	void* mm_allocate_contiguous_memory( std::size_t size, physical_address_t highest_acceptable_address ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmAllocateContiguousMemory" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, size, highest_acceptable_address );
	}

	void* mm_get_virtual_for_physical( physical_address_t physical_address ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmGetVirtualForPhysical" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, physical_address );
	}

	physical_address_t mm_get_physical_address( void* virtual_address ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmGetPhysicalAddress" ) );
		}

		if ( !fn_address )
			return { };

		physical_address_t result{};
		result.m_quad_part = g_hook->call_kernel<std::uint64_t>( fn_address, virtual_address );
		return result;
	}

	void* mm_map_io_space( physical_address_t physical_address, std::size_t number_of_bytes, std::uint32_t cache_type ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmMapIoSpace" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, physical_address, number_of_bytes, cache_type );
	}

	void mm_unmap_io_space( void* base_address, std::size_t number_of_bytes ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmUnmapIoSpace" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address, base_address, number_of_bytes );
	}

	std::uint64_t read_msr( std::uint32_t msr_index ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "HalpWheaNativeReadMsr" ) );
		}

		if ( !fn_address )
			return 0;

		return g_hook->call_kernel<std::uint64_t>( fn_address, 0, msr_index );
	}

	void* ke_get_current_processor( ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeGetCurrentProcessorNumberEx" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, nullptr );
	}

	NTSTATUS ke_set_target_processor_dpc( void* dpc, void* processor_number ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeSetTargetProcessorDpcEx" ) );
		}

		if ( !fn_address )
			return 0xC0000001;

		return g_hook->call_kernel<NTSTATUS>( fn_address, dpc, processor_number );
	}

	void* ke_insert_queue_dpc( void* dpc, void* system_arg1, void* system_arg2 ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeInsertQueueDpc" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, dpc, system_arg1, system_arg2 );
	}

	void* mm_get_system_routine_address( std::wstring routine_name ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmGetSystemRoutineAddress" ) );
		}

		if ( !fn_address )
			return nullptr;

		unicode_string_t system_routine_name;
		rtl_init_unicode_string( &system_routine_name, routine_name.c_str( ) );
		return g_hook->call_kernel<void*>( fn_address, &system_routine_name );
	}

	void* ke_initialize_dpc( void* dpc, void* routine, void* context ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeInitializeDpc" ) );
		}

		if ( !fn_address )
			return nullptr;

		return g_hook->call_kernel<void*>( fn_address, dpc, routine, context );
	}

	std::uint32_t ke_query_max_processor_count( ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeQueryMaximumProcessorCountEx" ) );
		}

		if ( !fn_address )
			return 0;

		return g_hook->call_kernel<std::uint32_t>( fn_address, 0 );
	}


	KAFFINITY ke_query_active_processors( ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeQueryActiveProcessors" ) );
		}

		if ( !fn_address )
			return { };

		return g_hook->call_kernel<KAFFINITY>( fn_address );
	}

	void ke_set_system_affinity_thread( KAFFINITY affinity ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeSetSystemAffinityThread" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address, affinity );
	}

	bool ke_cancel_timer( void* timer ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeCancelTimer" ) );
		}

		if ( !fn_address )
			return 0;

		return g_hook->call_kernel<bool>( fn_address, timer );
	}

	std::uint64_t get_kprcb( int processor ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeGetPrcb" ) );
		}

		if ( !fn_address )
			return 0;

		return g_hook->call_kernel<std::uint64_t>( fn_address, processor );
	}

	std::uint64_t get_current_kprcb( ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeGetCurrentPrcb" ) );
		}

		if ( !fn_address )
			return 0;

		return g_hook->call_kernel<std::uint64_t>( fn_address );
	}

	std::uint64_t query_system_information( int information_class, void* buffer, std::uint32_t buffer_size, std::uint32_t* return_length ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "ZwQuerySystemInformation" ) );
		}

		if ( !fn_address )
			return 0;

		return g_hook->call_kernel<NTSTATUS>(
			fn_address,
			information_class,
			buffer,
			buffer_size,
			return_length
		);
	}

	NTSTATUS mm_copy_memory(
		void* target_address,
		mm_copy_address_t source_address,
		std::size_t number_of_bytes,
		std::uint32_t flags,
		std::size_t* number_of_bytes_transferred
	) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmCopyMemory" ) );
		}

		if ( !fn_address )
			return 1;

		return g_hook->call_kernel<NTSTATUS>( fn_address, target_address, source_address, number_of_bytes, flags );
	}

	inline void ke_flush_entire_tb( bool invalidate, bool all_processors ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeFlushEntireTb" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address, invalidate, all_processors );
	}

	inline void ke_invalidate_all_caches( ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeInvalidateAllCaches" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address );
	}

	inline void ke_flush_single_tb( std::uintptr_t address, bool all_processors, bool invalidate ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeFlushSingleTb" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address, address, all_processors, invalidate );
	}

	std::pair< std::uint8_t*, std::uint8_t* > get_gdt_idt( ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KiGetGdtIdt" ) );
		}

		if ( !fn_address )
			return { };

		uint8_t gdt_info[ 10 ];
		uint8_t idt_info[ 10 ];
		g_hook->call_kernel<void*>( fn_address, &gdt_info, &idt_info );
		return std::make_pair( gdt_info, idt_info );
	}

    void* get_current_process_eprocess() {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "PsGetCurrentProcess" ) );
		}

		if ( !fn_address )
			return nullptr;

        return g_hook->call_kernel<void*>( fn_address );
    }

	bool rtl_compare_unicode_string( unicode_string_t* str1, unicode_string_t* str2, bool case_insensitive ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "RtlCompareUnicodeString" ) );
		}

		if ( !fn_address )
			return false;

		return g_hook->call_kernel<bool>( fn_address, str1, str2, case_insensitive );
	}

	void rtl_init_unicode_string( unicode_string_t* destination_string, const wchar_t* source_string ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "RtlInitUnicodeString" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address, destination_string, source_string );
	}

	void* ps_lookup_thread_by_tid( std::uint32_t tid ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "PsLookupThreadByThreadId" ) );
		}

		if ( !fn_address )
			return nullptr;

		void* thread = nullptr;
		auto status = g_hook->call_kernel< NTSTATUS >(
			fn_address,
			reinterpret_cast< void* >( static_cast< std::uint64_t >( tid ) ),
			&thread
		);

		if ( !NT_SUCCESS( status ) || !thread )
			return nullptr;

		return thread;
	}

	void ob_dereference_object( void* object ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "ObfDereferenceObject" ) );
		}

		if ( !fn_address )
			return;

		g_hook->call_kernel< void* >( fn_address, object );
	}

	bool mm_initialize_process_address_space( eprocess_t* clone_process, eprocess_t* target_process ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmInitializeProcessAddressSpace" ) );
		}

		if ( !fn_address )
			return false;

		auto flags = mm_allocate_independent_pages( sizeof( ULONG ) );
		if ( !flags )
			return false;

		auto result = g_hook->call_kernel< std::int64_t >(
			fn_address,
			clone_process,
			target_process,
			nullptr,
			flags,
			0
		);

		if ( result ) {
			logging::print( oxorany( "MmInitializeProcessAddressSpace failed with: 0x%llx" ), result );
			return false;
		}

		return true;
	}

	std::int32_t mi_copy_on_write( std::uint64_t virtual_address, std::uint64_t* pte_address ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiCopyOnWrite" ) );
		}

		if ( !fn_address )
			return false;

		return g_hook->call_kernel< std::int32_t >( fn_address, virtual_address, pte_address, -1, 0 );
	}

	void ke_stack_attach_process(
		eprocess_t* process,
		kapc_state_t* apc_state
	) {
		static auto fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeStackAttachProcess" ) );
			if ( !fn_address ) return;
		}

		g_hook->call_kernel< void* >( fn_address, process, apc_state );
	}

	void ke_unstack_detach_process(
		kapc_state_t* apc_state
	) {
		static auto fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "KeUnstackDetachProcess" ) );
			if ( !fn_address ) return;
		}

		g_hook->call_kernel< void* >( fn_address, apc_state );
	}

	bool mm_set_page_protection( std::uint64_t virtual_address, std::size_t size, std::uint64_t protection ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MmSetPageProtection" ) );
			if ( !fn_address )
				return 0;
		}

		return g_hook->call_kernel< bool >( fn_address, virtual_address, size, protection );
	}

	void mi_delete_vad( eprocess_t* process, void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiDeleteVad" ) );
		}
		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address, process, vad );
	}

	void mi_delete_inserted_clone_vads( eprocess_t* process ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiDeleteInsertedCloneVads" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, process );
	}

	void mi_clean_vad( void* vad_root ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiCleanVad" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, vad_root );
	}

	void mi_remove_vad( eprocess_t* process, void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiRemoveVad" ) );
		}
		if ( !fn_address )
			return;

		g_hook->call_kernel<void>( fn_address, process, vad );
	}

	void mi_finish_vad_deletion( eprocess_t* process, void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiFinishVadDeletion" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, process, vad );
	}

	void mi_free_vad_events( void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiFreeVadEvents" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, vad );
	}

	void mi_lock_down_working_set( eprocess_t* process, int index ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiLockDownWorkingSet" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, process, index );
	}

	void mi_release_vad_event_blocks( eprocess_t* process, void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiReleaseVadEventBlocks" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, process, vad );
	}

	void mi_return_vad_quota( eprocess_t* process, void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiReturnVadQuota" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, process, vad );
	}

	void mi_delete_vad_bitmap( void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiDeleteVadBitmap" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, vad );
	}

	void mi_free_vad_event_bitmap( eprocess_t* process, void* vad, int bit ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiFreeVadEventBitmap" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, process, vad, bit );
	}

	void mi_return_process_vads( eprocess_t * process ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiReturnProcessVads" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, process );
	}

	void mi_dereference_vad( void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiDereferenceVad" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, vad );
	}

	void mi_clone_discard_vad_commit( void* vad ) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "MiCloneDiscardVadCommit" ) );
		}
		if ( !fn_address )
			return;
		g_hook->call_kernel<void>( fn_address, vad );
	}

	bool rtl_create_user_thread(
		void* process_handle,
		void* security_descriptor,
		bool create_suspended,
		std::uint32_t stack_zero_bits,
		std::size_t stack_reserved,
		std::size_t stack_commit,
		void* start_address,
		void* parameter,
		void** thread_handle,
		void* client_id
	) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "RtlCreateUserThread" ) );
		}

		if ( !fn_address )
			return false;

		auto result = g_hook->call_kernel<std::int64_t>(
			fn_address,
			process_handle,
			security_descriptor,
			create_suspended,
			stack_zero_bits,
			stack_reserved,
			stack_commit,
			start_address,
			parameter,
			thread_handle,
			client_id
		);

		if ( result ) {
			logging::print( oxorany( "RtlCreateUserThread failed with: 0x%llx" ), result );
			return false;
		}

		return true;
	}

	bool ps_create_system_thread(
		void** thread_handle,
		std::uint32_t desired_access,
		void* object_attributes,
		void* process_handle,
		void* client_id,
		void* start_routine,
		void* start_context
	) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "PsCreateSystemThread" ) );
		}

		if ( !fn_address )
			return 0xC0000001;

		auto result = g_hook->call_kernel<NTSTATUS>(
			fn_address,
			thread_handle,
			desired_access,
			object_attributes,
			process_handle,
			client_id,
			start_routine,
			start_context
		);

		if ( result ) {
			logging::print( oxorany( "PsCreateSystemThread failed with: 0x%llx" ), result );
			return false;
		}

		return true;
	}

	std::int32_t ob_open_object_by_pointer(
		void* object,
		std::uint32_t handle_attributes,
		void* access_state,
		std::uint32_t desired_access,
		void* object_type,
		std::uint8_t processor_mode,
		void** handle
	) {
		static std::uint64_t fn_address = 0ull;
		if ( !fn_address ) {
			fn_address = g_ntoskrnl->get_export( oxorany( "ObOpenObjectByPointer" ) );
		}

		if ( !fn_address )
			return 0xC0000001;

		return g_hook->call_kernel<std::int32_t>(
			fn_address,
			object,
			handle_attributes,
			access_state,
			desired_access,
			object_type,
			processor_mode,
			handle
		);
	}

	void* ps_process_type( ) {
		static void* process_type = nullptr;
		if ( !process_type ) {
			auto process_type_ptr = g_ntoskrnl->get_export( oxorany( "PsProcessType" ) );
			if ( process_type_ptr ) {
				process_type = rw::read_virt<void*>( process_type_ptr );
			}
		}
		return process_type;
	}

	NTSTATUS zw_close( void* handle ) {
		static void* fn_zw_close = nullptr;
		if ( !fn_zw_close ) {
			fn_zw_close = reinterpret_cast< void* >(
				g_ntoskrnl->get_export( oxorany( "ZwClose" ) )
				);
		}

		if ( !fn_zw_close )
			return 1;

		return g_hook->call_kernel<NTSTATUS>( fn_zw_close, handle );
	}

	void* ps_loaded_module_list( ) {
		static void* ps_loaded_module_list = nullptr;
		if ( !ps_loaded_module_list ) {
			ps_loaded_module_list = reinterpret_cast< void* >(
				g_ntoskrnl->get_export( oxorany( "PsLoadedModuleList" ) )
				);
		}

		if ( !ps_loaded_module_list )
			return { };

		return ps_loaded_module_list;
	}

	std::uint64_t interlocked_exchange64( void* destination, std::uint64_t value ) {
		if ( !destination )
			return 0;

		auto dest_addr = reinterpret_cast< std::uint64_t >( destination );
		auto old_value = rw::read_virt<std::uint64_t>( dest_addr );

		rw::write_virt<std::uint64_t>( dest_addr, value );
		return old_value;
	}

	void* interlocked_exchange_pointer( void* destination, void* value ) {
		if ( !destination )
			return nullptr;

		auto dest_addr = reinterpret_cast< std::uint64_t >( destination );
		auto old_value = rw::read_virt<std::uint64_t>( dest_addr );

		rw::write_virt<std::uint64_t>( dest_addr, reinterpret_cast< std::uint64_t >( value ) );
		return reinterpret_cast< void* >( old_value );
	}

	void initialize_list_head( void* list_head ) {
		if ( !list_head )
			return;

		auto head_addr = reinterpret_cast< std::uint64_t >( list_head );

		rw::write_virt<std::uint64_t>( head_addr + 0, head_addr );
		rw::write_virt<std::uint64_t>( head_addr + 8, head_addr );
	}

	void interlocked_and( void* destination, std::uint32_t value ) {
		if ( !destination )
			return;

		auto dest_addr = reinterpret_cast< std::uint64_t >( destination );
		auto current =rw::read_virt<std::uint32_t>( dest_addr );

		auto new_value = current & value;
		rw::write_virt<std::uint32_t>( dest_addr, new_value );
	}

	std::uint32_t interlocked_exchange( void* destination, std::uint32_t value ) {
		if ( !destination )
			return 0;

		auto dest_addr = reinterpret_cast< std::uint64_t >( destination );
		auto old_value =rw::read_virt<std::uint32_t>( dest_addr );
		rw::write_virt<std::uint32_t>( dest_addr, value );

		return old_value;
	}

	std::uint32_t interlocked_compare_exchange( void* destination, std::uint32_t exchange, std::uint32_t comperand ) {
		if ( !destination )
			return 0;

		auto dest_addr = reinterpret_cast< std::uint64_t >( destination );

		auto current = rw::read_virt<std::uint32_t>( dest_addr );
		if ( current == comperand ) {
			rw::write_virt<std::uint32_t>( dest_addr, exchange );
		}

		return current;
	}

	std::uint64_t interlocked_compare_exchange64( void* destination, std::uint64_t exchange, std::uint64_t comperand ) {
		if ( !destination )
			return 0;

		auto dest_addr = reinterpret_cast< std::uint64_t >( destination );

		auto current =rw::read_virt<std::uint64_t>( dest_addr );
		if ( current == comperand ) {
			rw::write_virt<std::uint64_t>( dest_addr, exchange );
		}

		return current;
	}

	void flush_caches( void* address ) {
		ke_flush_entire_tb( true, true );
		ke_invalidate_all_caches( );
		ke_flush_single_tb( reinterpret_cast< std::uint64_t >( address ), true, true );
	}

	//bool execute_on_specific_core( std::uint32_t core_id, void* entry_point, void* context ) {
	//	logging::print( oxorany( "Queuing DPC execution on core: %d" ), core_id );

	//	auto dpc = allocate_pool( 0x40 );
	//	if ( !dpc ) {
	//		logging::print( oxorany( "Could not allocate DPC structure" ) );
	//		return false;
	//	}

	//	auto processor_number = allocate_pool( 8 );
	//	if ( !processor_number ) {
	//		logging::print( oxorany( "Could not allocate processor number structure" ) );
	//		return false;
	//	}

	//	logging::print( oxorany( "Processor number: %llx" ), processor_number );

	//	g_kernel->write_va<std::uint16_t>( reinterpret_cast< std::uint64_t >( processor_number ), 0 ); // Group = 0
	//	g_kernel->write_va<std::uint8_t>( reinterpret_cast< std::uint64_t >( processor_number ) + 2, core_id ); // Number = core_id
	//	g_kernel->write_va<std::uint8_t>( reinterpret_cast< std::uint64_t >( processor_number ) + 3, 0 ); // Reserved = 0

	//	ke_initialize_dpc( dpc, entry_point, context );

	//	auto result = ke_set_target_processor_dpc( dpc, processor_number );
	//	if ( result ) {
	//		logging::print( oxorany( "Could not set target processor for DPC with: %x" ), result );
	//		return false;
	//	}

	//	if ( !ke_insert_queue_dpc( dpc, nullptr, nullptr ) ) {
	//		logging::print( oxorany( "Could not insert DPC into queue" ) );
	//		return false;
	//	}

	//	logging::print( oxorany( "Successfully queued execution\n" ) );
	//	return true;
	//}

	//bool is_entry_empty( const mm_unloaded_drivers_t* entry ) {
	//	return entry->m_name.Length == 0 ||
	//		entry->m_name.Buffer == nullptr ||
	//		entry->m_unload_time == 0;
	//}

	//bool remove_unloaded_drivers( PWSTR driver_name ) {
	//	logging::print( oxorany( "Removing MmUnloadedDrivers entry" ) );

	//	auto mm_unloaded_drivers = g_kernel->read_va<mm_unloaded_drivers_t*>(
	//		g_ntoskrnl->rva(
	//		
	//		g_ntoskrnl->find_pattern(
	//		
	//		oxorany( "4C 8B 0D ? ? ? ? 83 FE" )
	//	), 7 )
	//	);
	//	if ( !mm_unloaded_drivers )
	//		return false;

	//	auto mm_last_unloaded_driver = g_kernel->read_va<ULONG*>(
	//		g_ntoskrnl->rva(
	//		
	//		g_ntoskrnl->find_pattern(
	//		
	//		oxorany( "4C 8B 35 ? ? ? ? BA" )
	//	),7 ) );
	//	if ( !mm_last_unloaded_driver )
	//		return false;

	//	auto ps_loaded_module_resource = g_kernel->read_va<eresource_t*>(
	//		g_ntoskrnl->rva(
	//		
	//		g_ntoskrnl->find_pattern(
	//		
	//		oxorany( "48 8D 0D ? ? ? ? E8 ? ? ? ? F7 43" )
	//	),
	//		7
	//	)
	//	);
	//	if ( !ps_loaded_module_resource )
	//		return false;

	//	logging::print( oxorany( "MmUnloadedDrivers: 0x%llx" ), mm_unloaded_drivers );
	//	logging::print( oxorany( "MmLastUnloadedDrivers: 0x%llx" ), mm_last_unloaded_driver );
	//	logging::print( oxorany( "PsLoadedModuleResource: 0x%llx" ), ps_loaded_module_resource );

	//	if ( !ex_acquire_resource( ps_loaded_module_resource, true ) )
	//		return false;

	//	bool modified = false;
	//	bool filled = true;
	//	UNICODE_STRING driver_name_us = utility::make_unicode_string( driver_name );

	//	for ( ULONG i = 0; i < 50; i++ ) {
	//		auto current_entry = mm_unloaded_drivers_t{};
	//		nt::memcpy(
	//			&current_entry,
	//			&mm_unloaded_drivers[ i ],
	//			sizeof( mm_unloaded_drivers_t )
	//		);

	//		if ( is_entry_empty( &current_entry ) ) {
	//			filled = false;
	//			continue;
	//		}

	//		if ( modified ) {
	//			if ( i == 49 ) {
	//				mm_unloaded_drivers_t empty_entry{};
	//				nt::memcpy(
	//					&mm_unloaded_drivers[ i ],
	//					&empty_entry,
	//					sizeof( mm_unloaded_drivers_t )
	//				);
	//			}
	//		}
	//		else if ( rtl_equal_unicode_string( &driver_name_us, &current_entry.m_name, TRUE ) ) {
	//			mm_unloaded_drivers_t empty_entry{};
	//			nt::memcpy(
	//				&mm_unloaded_drivers[ i ],
	//				&empty_entry,
	//				sizeof( mm_unloaded_drivers_t )
	//			);

	//			auto last_unloaded = g_kernel->read_va<ULONG>( mm_last_unloaded_driver );
	//			last_unloaded = ( filled ? 50 : last_unloaded ) - 1;
	//			g_kernel->write_va( mm_last_unloaded_driver, last_unloaded );

	//			modified = true;
	//		}
	//	}

	//	if ( modified ) {
	//		std::uint64_t previous_time = 0;

	//		for ( LONG i = 48; i >= 0; --i ) {
	//			auto current_entry = mm_unloaded_drivers_t{};
	//			nt::memcpy(
	//				&current_entry,
	//				&mm_unloaded_drivers[ i ],
	//				sizeof( mm_unloaded_drivers_t )
	//			);

	//			if ( is_entry_empty( &current_entry ) )
	//				continue;

	//			if ( previous_time != 0 && current_entry.m_unload_time > previous_time ) {
	//				current_entry.m_unload_time = previous_time - 100;
	//				nt::memcpy(
	//					&mm_unloaded_drivers[ i ],
	//					&current_entry,
	//					sizeof( mm_unloaded_drivers_t )
	//				);
	//			}

	//			previous_time = current_entry.m_unload_time;
	//		}
	//	}

	//	release_resource( ps_loaded_module_resource );
	//	return modified;
	//}

	//std::uint32_t find_least_active_core( ) {
	//	auto processor_count = ke_query_max_processor_count( );
	//	if ( processor_count == 0 )
	//		return 0;
	//	struct core_activity_t {
	//		std::uint64_t m_prcb;
	//		std::uint32_t m_core_id;
	//		std::uint64_t m_dpc_count;
	//		std::uint64_t m_interrupt_count;
	//		std::uint64_t m_idle_time;
	//		double m_activity_score;
	//	};

	//	auto activity_data = reinterpret_cast< core_activity_t* >(
	//		allocate_system_memory( sizeof( core_activity_t ) * processor_count )
	//		);

	//	if ( !activity_data ) {
	//		logging::print( oxorany( "Could not allocate memory for core activity data" ) );
	//		return processor_count - 1;
	//	}

	//	const int SystemProcessorPerformanceInformation = 8;
	//	const int SYSTEM_PROCESSOR_PERFORMANCE_INFO_SIZE = 48;
	//	auto perf_info = allocate_system_memory( SYSTEM_PROCESSOR_PERFORMANCE_INFO_SIZE * processor_count );
	//	if ( perf_info ) {
	//		std::uint32_t return_length = 0;
	//		auto status = query_system_information(
	//			SystemProcessorPerformanceInformation,
	//			perf_info,
	//			SYSTEM_PROCESSOR_PERFORMANCE_INFO_SIZE * processor_count,
	//			&return_length
	//		);

	//		if ( NT_SUCCESS( status ) ) {
	//			for ( auto i = 0; i < processor_count; i++ ) {
	//				auto perf_base = reinterpret_cast< std::uint64_t >( perf_info ) +
	//					( i * SYSTEM_PROCESSOR_PERFORMANCE_INFO_SIZE );

	//				auto idle_time = g_kernel->read_va<std::uint64_t>( perf_base + 0 );

	//				auto activity_base = reinterpret_cast< std::uint64_t >( activity_data ) +
	//					( i * sizeof( core_activity_t ) );

	//				g_kernel->write_va<std::uint32_t>(
	//					activity_base + offsetof( core_activity_t, m_core_id ),
	//					i
	//				);

	//				g_kernel->write_va<std::uint64_t>(
	//					activity_base + offsetof( core_activity_t, m_idle_time ),
	//					idle_time
	//				);
	//			}
	//		}

	//		free_system_memory( perf_info );
	//	}

	//	for ( auto i = 0; i < processor_count; i++ ) {
	//		auto prcb = get_kprcb( i );
	//		if ( prcb ) {
	//			auto dpc_count = g_kernel->read_va<std::uint64_t>( prcb + 0x21E0 );
	//			auto interrupt_count = g_kernel->read_va<std::uint64_t>( prcb + 0x2290 );

	//			auto activity_base = reinterpret_cast< std::uint64_t >( activity_data ) +
	//				( i * sizeof( core_activity_t ) );

	//			g_kernel->write_va<std::uint64_t>(
	//				activity_base + offsetof( core_activity_t, m_prcb ),
	//				prcb
	//			);

	//			g_kernel->write_va<std::uint64_t>(
	//				activity_base + offsetof( core_activity_t, m_dpc_count ),
	//				dpc_count
	//			);

	//			g_kernel->write_va<std::uint64_t>(
	//				activity_base + offsetof( core_activity_t, m_interrupt_count ),
	//				interrupt_count
	//			);
	//		}
	//		else {
	//			logging::print( oxorany( "Could not get PRCB for core %d" ), i );
	//		}
	//	}

	//	std::vector<core_activity_t> local_activity_data( processor_count );
	//	for ( auto i = 0; i < processor_count; i++ ) {
	//		auto activity_base = reinterpret_cast< std::uint64_t >( activity_data ) +
	//			( i * sizeof( core_activity_t ) );

	//		local_activity_data[ i ].m_core_id = g_kernel->read_va<std::uint32_t>(
	//			activity_base + offsetof( core_activity_t, m_core_id )
	//		);

	//		local_activity_data[ i ].m_prcb = g_kernel->read_va<std::uint64_t>(
	//			activity_base + offsetof( core_activity_t, m_prcb )
	//		);

	//		local_activity_data[ i ].m_dpc_count = g_kernel->read_va<std::uint64_t>(
	//			activity_base + offsetof( core_activity_t, m_dpc_count )
	//		);

	//		local_activity_data[ i ].m_interrupt_count = g_kernel->read_va<std::uint64_t>(
	//			activity_base + offsetof( core_activity_t, m_interrupt_count )
	//		);

	//		local_activity_data[ i ].m_idle_time = g_kernel->read_va<std::uint64_t>(
	//			activity_base + offsetof( core_activity_t, m_idle_time )
	//		);

	//		local_activity_data[ i ].m_activity_score =
	//			( local_activity_data[ i ].m_dpc_count * 0.4 ) +
	//			( local_activity_data[ i ].m_interrupt_count * 0.4 ) -
	//			( local_activity_data[ i ].m_idle_time * 0.2 );

	//		g_kernel->write_va<double>(
	//			activity_base + offsetof( core_activity_t, m_activity_score ),
	//			local_activity_data[ i ].m_activity_score
	//		);
	//	}

	//	core_activity_t best_core = local_activity_data[ 0 ];
	//	auto best_score = local_activity_data[ 0 ].m_activity_score;
	//	for ( auto i = 1; i < processor_count; i++ ) {
	//		auto& local_activity = local_activity_data[ i ];
	//		if ( local_activity.m_activity_score < best_score ) {
	//			best_score = local_activity.m_activity_score;
	//			best_core = local_activity;
	//		}
	//	}

	//	logging::print( oxorany( "System has %d cores, targeting core %d" ), processor_count, best_core.m_core_id );
	//	logging::print( oxorany( "Core %d: PRCB=0x%llx, DPC=%llu, Interrupt=%llu\n" ),
	//		best_core.m_core_id, best_core.m_prcb, best_core.m_dpc_count, best_core.m_interrupt_count );

	//	free_system_memory( activity_data );
	//	return best_core.m_core_id;
	//}

	//std::pair<std::uintptr_t, std::uintptr_t> resolve_token_offset( ) {
	//	std::uintptr_t token_offset = 0;
	//	std::uintptr_t system_token = 0;

	//	constexpr uintptr_t PID_OFFSET = 0x440;
	//	constexpr uintptr_t TOKEN_OFFSET = 0x4B8;

	//	const auto ps_initial_system_process =
	//		g_ntoskrnl->get_export(  oxorany( "PsInitialSystemProcess" ) );

	//	const auto system_eprocess =
	//		g_kernel->read_va<std::uintptr_t>( ps_initial_system_process );

	//	if ( g_kernel->read_va<std::uintptr_t>( system_eprocess + PID_OFFSET ) != 4 ) {
	//		logging::print( oxorany( "SYSTEM process validation failed!" ) );
	//		return { 0, 0 };
	//	}

	//	token_offset = TOKEN_OFFSET;
	//	system_token = g_kernel->read_va<std::uintptr_t>( system_eprocess + token_offset );

	//	if ( ( system_token & 0xF ) == 0 ) {
	//		logging::print( oxorany( "Invalid SYSTEM token structure!" ) );
	//		return { 0, 0 };
	//	}

	//	return { token_offset, system_token };
	//}

	//bool remove_piddb_cache( PWSTR driver_name, std::uint32_t timestamp ) {
	//	logging::print( oxorany( "Removing PiDDB cache entry" ) );

	//	auto piddb_cache_table = reinterpret_cast<avl_table_t*>(
	//		g_ntoskrnl->rva_lea(
	//			
	//			g_ntoskrnl->find_pattern(
	//			
	//			oxorany( "48 8D 0D ? ? ? ? 45 33 F6 48 89 44 24" ) ), 7 ) );
	//	if ( !piddb_cache_table )
	//		return false;

	//	auto piddb_lock = reinterpret_cast<eresource_t*>(
	//		g_ntoskrnl->rva_lea( 
	//		
	//		g_ntoskrnl->find_pattern(
	//		
	//		oxorany( "48 8D 0D ? ? ? ? E8 ? ? ? ? 4C 8B 8C 24" ) ), 7 ) );
	//	if ( !piddb_lock )
	//		return false;

	//	avl_table_t piddb_table;
	//	nt::memcpy(
	//		&piddb_table,
	//		piddb_cache_table,
	//		sizeof( avl_table_t )
	//	);

	//	logging::print( oxorany( "PiDDBCacheTable: 0x%llx" ), piddb_cache_table );
	//	logging::print( oxorany( "PiDDBLock: 0x%llx" ), piddb_lock );
	//	//logging::print( oxorany( "PiDDB Elements=%d, Tree depth=%d\n" ),
	//	//	piddb_table.m_table_elements, piddb_table.m_tree_depth );

	//	piddb_cache_entry_t piddb_entry{};
	//	piddb_entry.m_timestamp = timestamp;
	//	rtl_init_unicode_string( &piddb_entry.m_driver_name, driver_name );

	//	if ( !ex_acquire_resource( piddb_lock, true ) )
	//		return false;

	//	auto cache_entry = reinterpret_cast< piddb_cache_entry_t* > (
	//		lookup_element_table( piddb_cache_table, &piddb_entry )
	//		);
	//	if ( !cache_entry ) {
	//		release_resource( piddb_lock );
	//		return false;
	//	}

	//	nt::memcpy(
	//		&piddb_entry,
	//		&cache_entry,
	//		sizeof( piddb_cache_entry_t )
	//	);

	//	list_entry_t next_entry{ };
	//	nt::memcpy(
	//		&next_entry,
	//		&piddb_entry.m_list.m_flink,
	//		sizeof( list_entry_t )
	//	);

	//	list_entry_t previous_entry{ };
	//	nt::memcpy(
	//		&previous_entry,
	//		&piddb_entry.m_list.m_blink,
	//		sizeof( list_entry_t )
	//	);

	//	previous_entry.m_flink = piddb_entry.m_list.m_flink;
	//	previous_entry.m_blink = piddb_entry.m_list.m_blink;

	//	nt::memcpy(
	//		&piddb_entry.m_list.m_blink,
	//		&previous_entry,
	//		sizeof( list_entry_t )
	//	);

	//	nt::memcpy(
	//		&piddb_entry.m_list.m_flink,
	//		&next_entry,
	//		sizeof( list_entry_t )
	//	);

	//	delete_table_entry( piddb_cache_table, cache_entry );
	//	piddb_table.m_delete_count--;

	//	piddb_cache_entry_t old_piddb_entry{};
	//	old_piddb_entry.m_driver_name = utility::make_unicode_string( driver_name );
	//	old_piddb_entry.m_timestamp = timestamp;
	//	auto result = lookup_element_table( piddb_cache_table, &old_piddb_entry );

	//	logging::print( oxorany( "Successfully removed PiDDB cache entry\n" ) );
	//	release_resource( piddb_lock );
	//	return result == nullptr;
	//}
}