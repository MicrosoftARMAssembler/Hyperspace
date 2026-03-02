#pragma once

namespace nt {
	std::uint64_t ps_get_process_image_file_name( eprocess_t* process );
	std::uint32_t ps_get_process_session_id( eprocess_t* process );
	bool is_address_valid( void* address );
	void* ps_get_process_peb( void* eprocess );
	physical_memory_range_t* mm_get_physical_memory_ranges( );
	std::uint64_t ps_get_process_section_base_address( void* process );
	std::uint64_t get_ps_active_process_head( );
	void memcpy( void* dst, void* src, std::uint64_t size );
	void* mm_allocate_independent_pages( std::size_t size );
	void mm_free_independent_pages( std::uint64_t independent_pages, std::size_t size );
	void* get_pte_address( std::uint64_t address );
	void* get_pde_address( std::uint64_t address );
	void* ps_lookup_process_by_pid( std::uint32_t process_id );
	bool ex_acquire_resource( void* resource, bool wait );
	void mi_clean_vad( void* vad_root );
	void mi_remove_vad( eprocess_t* process, void* vad );
	piddb_cache_entry_t* lookup_element_table( avl_table_t* table, void* buffer );
	void release_resource( void* resource );
	bool delete_table_entry( avl_table_t* table, void* buffer );
	bool rtl_equal_unicode_string( UNICODE_STRING* str1, UNICODE_STRING* str2, bool case_insensitive );
	void* ex_allocate_pool( std::size_t size );
	void* mm_allocate_contiguous_memory( std::size_t size, physical_address_t highest_acceptable_address );
	void* mm_get_virtual_for_physical( physical_address_t physical_address );
	physical_address_t mm_get_physical_address( void* virtual_address );
	void* mm_map_io_space( physical_address_t physical_address, std::size_t number_of_bytes, std::uint32_t cache_type );
	void mm_unmap_io_space( void* base_address, std::size_t number_of_bytes );
	std::uint64_t read_msr( std::uint32_t msr_index );
	void* ke_get_current_processor( );
	NTSTATUS ke_set_target_processor_dpc( void* dpc, void* processor_number );
	void* ke_insert_queue_dpc( void* dpc, void* system_arg1, void* system_arg2 );
	void* ke_initialize_dpc( void* dpc, void* routine, void* context );
	std::uint32_t ke_query_max_processor_count( );
	KAFFINITY ke_query_active_processors( );
	void ke_set_system_affinity_thread( KAFFINITY affinity );
	bool ke_cancel_timer( void* timer );
	std::uint64_t get_kprcb( int processor );
	std::uint64_t get_current_kprcb( );
	std::uint64_t query_system_information( int information_class, void* buffer, std::uint32_t buffer_size, std::uint32_t* return_length );
	std::pair< std::uint8_t*, std::uint8_t* > get_gdt_idt( );
	void* get_current_process_eprocess( );
	NTSTATUS mm_copy_memory(
		void* target_address,
		mm_copy_address_t source_address,
		std::size_t number_of_bytes,
		std::uint32_t flags,
		std::size_t* number_of_bytes_transferred
	);
	void ke_flush_entire_tb( bool invalidate, bool all_processors );
	void ke_invalidate_all_caches( );
	void ke_flush_single_tb( std::uintptr_t address, bool all_processors, bool invalidate );
	void flush_caches( void* address );
	void memset( void* dst, int value, std::uint64_t size );
	void memcpy_flush( void* dst, void* src, std::size_t size );
	bool rtl_compare_unicode_string( unicode_string_t* str1, unicode_string_t* str2, bool case_insensitive );
	void rtl_init_unicode_string( unicode_string_t* destination_string, const wchar_t* source_string );
	void* ps_lookup_thread_by_tid( std::uint32_t tid );
	void ob_dereference_object( void* object );
	void* interlocked_exchange_pointer( void* target, void* value );
	void initialize_list_head( void* list_head );
	bool mm_initialize_process_address_space( eprocess_t* clone_process, eprocess_t* target_process );
	std::uint64_t interlocked_exchange64( void* destination, std::uint64_t value );
	void interlocked_and( void* destination, std::uint32_t value );
	std::uint32_t interlocked_exchange( void* destination, std::uint32_t value );
	std::uint32_t interlocked_compare_exchange( void* destination, std::uint32_t exchange, std::uint32_t comperand );
	std::uint64_t interlocked_compare_exchange64( void* destination, std::uint64_t exchange, std::uint64_t comperand );
	void mi_delete_inserted_clone_vads( eprocess_t* process );
	void mi_clone_discard_vad_commit( void* vad );
	void mi_free_vad_event_bitmap( eprocess_t* process, void* vad, int bit );
	void mi_free_vad_events( void* vad );
	void mi_lock_down_working_set( eprocess_t* process, int index );
	void mi_delete_vad_bitmap( void* vad );
	bool mm_set_page_protection( std::uint64_t virtual_address, std::size_t size, std::uint64_t protection );
	std::int32_t mi_copy_on_write( std::uint64_t virtual_address, std::uint64_t* pte_address );
	void ke_stack_attach_process( eprocess_t* process, kapc_state_t* apc_state );
	void ke_unstack_detach_process( kapc_state_t* apc_state );
	void* ps_loaded_module_list( );
}