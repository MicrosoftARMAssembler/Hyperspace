#pragma once

namespace process {
	peb_t* get_process_peb( eprocess_t* eprocess ) {
		auto process_peb_offset = nt::get_process_peb_offset( );
		return *reinterpret_cast< peb_t** >(
			reinterpret_cast< std::uintptr_t >( eprocess ) + process_peb_offset
			);
	}

	void* get_section_base_address( eprocess_t* eprocess ) {
		auto section_base_address_offset = nt::get_section_base_address_offset( );
		return *reinterpret_cast< void** >( 
			reinterpret_cast< std::uintptr_t >( eprocess ) + section_base_address_offset 
			);
	}

	void* get_process_id( eprocess_t* eprocess ) {
		auto process_id_offset = nt::get_process_id_offset( );
		return *reinterpret_cast< void** >(
			reinterpret_cast< std::uintptr_t >( eprocess ) + process_id_offset
			);
	}

	const char* get_image_file_name( eprocess_t* eprocess ) {
		if ( !nt::mm_is_address_valid( eprocess ) )
			return 0;

		auto image_file_name_offset = nt::get_image_file_name_offset( );
		auto image_file_name_ptr = reinterpret_cast< const char* >(
			reinterpret_cast< std::uintptr_t >( eprocess ) + image_file_name_offset
			);

		if ( !nt::mm_is_address_valid( const_cast< char* >( image_file_name_ptr ) ) )
			return 0;

		char image_file_name[ 16 ] = { 0 };
		__try {
			for ( int i = 0; i < 15; i++ ) {
				image_file_name[ i ] = image_file_name_ptr[ i ];
				if ( image_file_name[ i ] == '\0' )
					break;
			}
			image_file_name[ 15 ] = '\0';
		}
		__except ( 1 ) {
			return 0;
		}

		return image_file_name;
	}

	std::uint64_t get_directory_table_base( eprocess_t* eprocess ) {
		return eprocess->m_pcb.m_directory_table_base;
	}

	eprocess_t* get_eprocess( std::uint32_t target_pid ) {
		auto process_list_head = nt::ps_active_process_head( );
		if ( !process_list_head )
			return nullptr;

		auto linkage_va = reinterpret_cast< std::uintptr_t >( process_list_head ) -
			nt::ps_initial_system_process( );
		if ( !linkage_va )
			return nullptr;

		for ( auto flink = process_list_head->m_flink; flink; flink = flink->m_flink ) {
			if ( !nt::mm_is_address_valid( flink ) )
				break;

			auto curr_eprocess = reinterpret_cast< eprocess_t* >(
				reinterpret_cast< std::uintptr_t >( flink ) - linkage_va
				);
			if ( !curr_eprocess )
				continue;

			auto process_id = nt::ps_get_process_id( curr_eprocess );
			if ( process_id == target_pid )
				return curr_eprocess;
		}

		return nullptr;
	}

	eprocess_t* get_eprocess( const char* process_name ) {
		auto process_list_head = nt::ps_active_process_head( );
		if ( !process_list_head )
			return nullptr;

		auto linkage_va = reinterpret_cast< std::uintptr_t >( process_list_head ) -
			nt::ps_initial_system_process( );
		if ( !linkage_va )
			return nullptr;

		for ( auto flink = process_list_head->m_flink; flink; flink = flink->m_flink ) {
			if ( !nt::mm_is_address_valid( flink ) )
				break;

			auto curr_eprocess = reinterpret_cast< eprocess_t* >(
				reinterpret_cast< std::uintptr_t >( flink ) - linkage_va
				);
			if ( !curr_eprocess )
				continue;

			if ( !nt::mm_is_address_valid( curr_eprocess ) )
				continue;

			auto image_file_name = get_image_file_name( curr_eprocess );
			if ( _stricmp( image_file_name, process_name ) == 0 ) {
				return curr_eprocess;
			}
		}

		return nullptr;
	}

	eprocess_t* get_session_process( const char* process_name ) {
		auto process_list_head = nt::ps_active_process_head( );
		if ( !process_list_head )
			return nullptr;

		auto linkage_va = reinterpret_cast< std::uintptr_t >( process_list_head ) -
			nt::ps_initial_system_process( );
		if ( !linkage_va )
			return nullptr;

		for ( auto flink = process_list_head->m_flink; flink; flink = flink->m_flink ) {
			if ( !nt::mm_is_address_valid( flink ) )
				break;

			auto curr_eprocess = reinterpret_cast< eprocess_t* >(
				reinterpret_cast< std::uintptr_t >( flink ) - linkage_va
				);
			if ( !curr_eprocess )
				continue;

			if ( !nt::mm_is_address_valid( curr_eprocess ) )
				continue;

			auto session_id = nt::ps_get_process_session_id( curr_eprocess );
			if ( !session_id ) continue;

			auto image_file_name = get_image_file_name( curr_eprocess );
			if ( _stricmp( image_file_name, process_name ) == 0 ) {
				return curr_eprocess;
			}
		}

		return nullptr;
	}

	bool attach( eprocess_t* eprocess, eprocess_t** org_process = nullptr ) {
		auto current_thread = reinterpret_cast< ethread_t* >( __readgsqword( 0x188 ) );
		if ( !current_thread ) {
			return false;
		}

		auto apc_state = &current_thread->m_kthread.m_apc_state;
		if ( org_process )
			*org_process = apc_state->m_process;

		apc_state->m_process = eprocess;
		__writecr3( eprocess->m_pcb.m_directory_table_base );
		return true;
	}

	std::uint64_t allocate_in_system_working( eprocess_t* eprocess, std::uint32_t size ) {
		auto vm_support = reinterpret_cast< std::uint64_t >( eprocess ) + 0x680;
		auto wsle_array = *reinterpret_cast< std::uint64_t* >( vm_support + 0x10 );
		if ( !wsle_array )
			return 0;

		auto ws_size = *reinterpret_cast< std::uint32_t* >( vm_support + 0x4C );
		auto needed_pages = ( size + 0xFFF ) >> 12;

		for ( auto i = 0; i < ws_size - 1; i++ ) {
			auto current_wsle = *reinterpret_cast< std::uint64_t* >( wsle_array + ( i * 8 ) );
			auto next_wsle = *reinterpret_cast< std::uint64_t* >( wsle_array + ( ( i + 1 ) * 8 ) );

			auto current_vpn = ( current_wsle >> 12 );
			auto next_vpn = ( next_wsle >> 12 );

			auto current_va = current_vpn << 12;
			auto next_va = next_vpn << 12;

			auto gap_pages = next_vpn - current_vpn - 1;
			if ( gap_pages >= needed_pages ) {
				auto gap_va = current_va + 0x1000;

				nt::dbg_print( oxorany( "[module] Found working set gap at 0x%llx (gap: %lld pages)\n" ),
					gap_va, gap_pages );

				if ( paging::map_page_tables( gap_va, size ) ) {
					nt::dbg_print( oxorany( "[module] Successfully allocated in System working set gap\n" ) );
					return gap_va;
				}
			}
		}

		nt::dbg_print( oxorany( "[module] Could not find suitable gap in System working set\n" ) );
		return 0;
	}
}