#pragma once

namespace client {
	eprocess_t* m_eprocess{ nullptr };
	control::data_request_t* m_data_request{ nullptr };
	control::root_request_t* m_root_request{ nullptr };
	ksemaphore_t* m_response_handle{ nullptr };
	ksemaphore_t* m_request_handle{ nullptr };

	bool is_attached( ) {
		return m_eprocess && m_data_request &&
			m_request_handle && m_response_handle;
	}

	bool initialize( std::uint32_t process_id ) {
		m_eprocess = process::get_eprocess( process_id );
		if ( !m_eprocess )
			return false;

		eprocess_t* orig_process;
		process::attach( m_eprocess, &orig_process );

		{
			paging::cr3_guard_t guard( current_cr3 );

			if ( !paging::ptm::initialize( ) )
				return false;

			if ( !paging::hyperspace::create_hyperspace( m_eprocess ) )
				return false;

			m_data_request = reinterpret_cast< control::data_request_t* >(
				paging::hyperspace::allocate_in_hyperspace(
					m_eprocess, sizeof( control::data_request_t )
				) );
			if ( !m_data_request )
				return false;

			m_root_request = reinterpret_cast< control::root_request_t* >(
				paging::hyperspace::allocate_in_hyperspace(
					m_eprocess, sizeof( control::root_request_t ) * control::m_max_messages
				) );
			if ( !m_root_request )
				return false;

			if ( !paging::spoof_pte_range(
				reinterpret_cast< std::uint64_t >( m_data_request ),
				sizeof( control::data_request_t )
			) ) return false;

			if ( !paging::spoof_pte_range(
				reinterpret_cast< std::uint64_t >( m_root_request ),
				sizeof( control::root_request_t ) * control::m_max_messages
			) ) return false;
		}

		auto thread_teb = nt::ps_get_current_thread_teb( );
		if ( !thread_teb )
			return false;

		auto response_handle = *reinterpret_cast< HANDLE* >(
			thread_teb->m_instrumentation[ 11 ]
			);
		if ( !response_handle )
			return false;

		if ( nt::ob_reference_object_by_handle(
			response_handle,
			0x1F0003,
			nullptr,
			0,
			reinterpret_cast< void** >( &m_response_handle ),
			nullptr
		) ) return false;

		response_handle = *reinterpret_cast< HANDLE* >(
			thread_teb->m_instrumentation[ 6 ]
			);
		if ( !response_handle )
			return false;

		if ( nt::ob_reference_object_by_handle(
			response_handle,
			0x1F0003,
			nullptr,
			0,
			reinterpret_cast< void** >( &m_request_handle ),
			nullptr
		) ) return false;

		thread_teb->m_instrumentation[ 7 ] = reinterpret_cast< std::uint8_t* >( phys::mm_lowest_physical_page );
		thread_teb->m_instrumentation[ 8 ] = reinterpret_cast< std::uint8_t* >( phys::mm_highest_physical_page );
		thread_teb->m_instrumentation[ 9 ] = reinterpret_cast< std::uint8_t* >( m_data_request );
		thread_teb->m_instrumentation[ 10 ] = reinterpret_cast< std::uint8_t* >( m_root_request );

		paging::attach_to_process( paging::m_system_cr3 );
		process::attach( orig_process );
		return true;
	}

	std::uint32_t update_root_logger( ) {
		if ( !is_attached( ) || !m_root_request ) {
			return 0;
		}

		auto message_count = 0;
		for ( auto idx = root::m_tail_index;
			idx != root::m_head_index;
			idx = ( idx + 1 ) % control::m_max_messages ) {
			if ( message_count >= control::m_max_messages ) {
				break;
			}

			std::memcpy(
				&m_root_request[ message_count ],
				&root::m_root_mappings[ idx ],
				sizeof( control::root_request_t )
			);

			memset(
				&root::m_root_mappings[ idx ],
				0,
				sizeof( control::root_request_t )
			);

			message_count++;
		}

		root::m_tail_index = root::m_head_index;
		return message_count;
	}

	void destory( ) {
		nt::dbg_print( oxorany( "[client] destroying client\n" ) );

		if ( m_data_request )
			m_data_request = 0;

		if ( m_root_request )
			m_root_request = 0;

		if ( m_response_handle ) {
			nt::ob_dereference_object( m_response_handle );
			m_response_handle = nullptr;
		}

		if ( m_request_handle ) {
			nt::ob_dereference_object( m_request_handle );
			m_request_handle = nullptr;
		}

		void* process_handle = nullptr;
		auto result = nt::ob_open_object_by_pointer(
			paging::hyperspace::m_entries
			[ paging::hyperspace::find_entry( m_eprocess ) ].m_clone_process,
			OBJ_KERNEL_HANDLE,
			nullptr,
			PROCESS_ALL_ACCESS,
			nt::ps_process_type( ),
			0,
			&process_handle
		);

		if ( !result && process_handle ) {
			result = nt::zw_terminate_process( process_handle, nt_status_t::success );
			nt::dbg_print( oxorany( "[client] terminating process: 0x%x\n" ), result );
		}
	}
}