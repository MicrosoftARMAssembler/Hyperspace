#pragma once

namespace control {
	const std::uint32_t m_magic_code = 0xceedbeef;
	const std::uint32_t m_max_messages = 512;
	const std::uint32_t m_max_message_size = 256;

	enum data_type : std::uint8_t {
		none = 0,
		ping,
		hyperspace_create,
		hyperspace_allocate,
		threads_to_hyperspace,
		threads_from_hyperspace,
		cow_create_exception,
		eac_ping,
		eac_attach_process,
		eac_detach_process,
		eac_allocate_virtual,
		eac_page_memory,
		eac_flush_virtual,
		eac_free_virtual,
		eac_read_virtual,
		eac_write_virtual,
		eac_protect_virtual,
		eac_set_information,
		eac_query_virtual,
		vad_allocate_virtual,
		remap_allocate_virtual,
		remap_physical_page,
		update_root,
		get_eprocess,
		get_process_peb,
		get_base_address,
		get_process_cr3,
		swap_context,
		set_page_protection,
		hyperspace_entries,
		translate_linear,
		map_process_page,
		read_virtual,
		write_virtual,
		read_physical,
		write_physical,
		attach_hyperspace,
		detach_hyperspace,
		create_hook,
		attach_process,
		detach_process,
		flush_virtual,
		unload_driver,
		unload_hook
	};

	struct root_request_t {
		char m_root_message[ m_max_message_size ];
	};

	struct data_request_t {
		std::uint32_t m_magic_code;
		data_type m_request_type;
		std::uint32_t m_message_count;
		eprocess_t* m_process;
		eprocess_t* m_old_process;
		peb_t* m_process_peb;
		std::uint32_t m_process_id;
		std::uint32_t m_thread_id;
		std::uint64_t m_address;
		std::uint64_t m_address1;
		void* m_address2;
		std::size_t m_size;
		kapc_state_t m_apc_state;
		mmpfn_t* m_pfn_database;
		mmpfn_t m_pfn_entry;
		pml4e m_pml4e;
		pdpte m_pdpte;
		pde m_pde;
		pte m_pte;
		bool m_large_page;
		bool m_user_supervisor;
		paging::pt_entries_t m_pt_entries;
		paging::page_protection m_page_protection;
		memory_basic_information_t m_memory_info;
		std::uint32_t m_protection;
		std::uint32_t m_old_protection;
		bool m_high_address;
		bool m_status;
	};
}