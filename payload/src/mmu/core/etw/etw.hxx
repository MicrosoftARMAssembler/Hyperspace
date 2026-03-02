#pragma once

namespace etw {
	constexpr auto etwp_start_trace = 1;
	constexpr auto etwp_stop_trace = 2;
	constexpr auto etwp_query_trace = 3;
	constexpr auto etwp_update_trace = 4;
	constexpr auto etwp_flush_trace = 5;

	constexpr auto wnode_flag_traced_guid = 0x00020000;
	constexpr auto event_trace_buffering_mode = 0x00000400;
	constexpr auto event_trace_flag_systemcall = 0x00000080;

	std::uint64_t m_ki_system_service_repeat = 0;
	const guid_t ckcl_session_guid =
	{ 0x54dea73a, 0xed1f, 0x42a4, { 0xaf, 0x71, 0x3e, 0x63, 0xd0, 0x56, 0xf1, 0x74 } };

	namespace trace {
		void map_hook( std::uint32_t syscall_idx, void** target_address );
	}

	namespace hook {
		paging::pth::hook_state_t m_hal_collect_pmc_counters;

		void ( *hal_collect_pmc_counters_org )( hal_pmc_counters_t*, uint64_t* ) = nullptr;
		void hal_collect_pmc_counters_hk( hal_pmc_counters_t* hal_pmc_counters, uint64_t* a2 ) {
			if ( KeGetCurrentIrql( ) > DISPATCH_LEVEL ) {
				return hal_collect_pmc_counters_org( hal_pmc_counters, a2 );
			}

			if ( ExGetPreviousMode( ) == KernelMode ) {
				return hal_collect_pmc_counters_org( hal_pmc_counters, a2 );
			}

			auto current_thread = nt::ps_get_current_thread( );
			if ( !current_thread )
				return hal_collect_pmc_counters_org( hal_pmc_counters, a2 );

			auto system_call_number = reinterpret_cast< std::uint32_t* >(
				reinterpret_cast< std::uint8_t* >( current_thread ) + 0x80 );
			if ( !nt::mm_is_address_valid( system_call_number ) ) {
				return hal_collect_pmc_counters_org( hal_pmc_counters, a2 );
			}

			auto stack_top = reinterpret_cast< void** >( __readgsqword( 0x1A8 ) );
			auto stack_bottom = reinterpret_cast< void** >( _AddressOfReturnAddress( ) );
			for ( auto stack = stack_top; stack > stack_bottom; --stack ) {
				auto magic1 = reinterpret_cast< std::uint32_t* >( stack );
				if ( *magic1 != 0x501802ul )
					continue;

				--stack;
				auto magic2 = reinterpret_cast< std::uint16_t* >( stack );
				if ( *magic2 != 0xF33ul )
					continue;

				for ( ; stack < stack_top; ++stack ) {
					if ( !nt::mm_is_address_valid( stack ) )
						continue;

					if ( page_align( *stack ) >= page_align( m_ki_system_service_repeat ) &&
						 page_align( *stack ) <= page_align( m_ki_system_service_repeat + paging::page_4kb_size * 2 ) ) {
						trace::map_hook( *system_call_number, &stack[ 9 ] );

						//auto process = nt::ps_get_current_process( );
						//if ( nt::mm_is_address_valid( process ) ) {
						//	auto image_file_name = process::get_image_file_name( process );
						//	if ( std::strstr_lowercase( image_file_name, "Fortnite" ) ||
						//		 std::strstr_lowercase( image_file_name, "EAC" ) ||
						//		 std::strstr_lowercase( image_file_name, "eac" ) ||
						//		 std::strstr_lowercase( image_file_name, "fortnite" ) ) {
						//		nt::dbg_print( oxorany( "[ETW] syscall captured: 0x%x process=%s\n" ), *system_call_number, image_file_name );
						//	}
						//}
						break;
					}
				}
				break;
			}

			return hal_collect_pmc_counters_org( hal_pmc_counters, a2 );
		}

		bool configure_trace( ckcl_trace_operation_t operation ) {
			auto property = reinterpret_cast
				< ckcl_trace_properties_t* >( mmu::alloc_page( paging::page_4kb_size ) );
			if ( !property )
				return false;

			property->m_wnode.m_buffer_size = paging::page_4kb_size;
			property->m_wnode.m_flags = wnode_flag_traced_guid;
			property->m_provider_name = RTL_CONSTANT_STRING( L"Circular Kernel Context Logger" );
			property->m_wnode.m_guid = ckcl_session_guid;
			property->m_wnode.m_client_context = 1;
			property->m_buffer_size = sizeof( unsigned long );
			property->m_minimum_buffers = property->m_maximum_buffers = 2;
			property->m_log_file_mode = event_trace_buffering_mode;

			auto result = nt_status_t::success;
			std::uint32_t return_length = 0;

			switch ( operation ) {
			case ckcl_trace_operation_t::start:
			{
				result = nt::zw_trace_control( etwp_start_trace, property, paging::page_4kb_size,
					property, paging::page_4kb_size, &return_length );
			} break;
			case ckcl_trace_operation_t::end:
			{
				result = nt::zw_trace_control( etwp_stop_trace, property, paging::page_4kb_size,
					property, paging::page_4kb_size, &return_length );
			} break;
			case ckcl_trace_operation_t::syscall:
			{
				property->m_enable_flags = event_trace_flag_systemcall;

				result = nt::zw_trace_control( etwp_update_trace, property, paging::page_4kb_size,
					property, paging::page_4kb_size, &return_length );
			} break;
			}

			mmu::free_page( reinterpret_cast< std::uint64_t >( property ), paging::page_4kb_size );
			return !result;
		}

		bool initialize( ) {
			if ( !( m_ki_system_service_repeat = ssdt::get_ki_system_service_repeat( ) ) )
				return false;

			if ( !configure_trace( ckcl_trace_operation_t::syscall ) &&
				( !configure_trace( ckcl_trace_operation_t::start ) ||
					!configure_trace( ckcl_trace_operation_t::syscall ) ) )
				return false;

			auto hal_private_dispatch = nt::get_hal_private_dispatch( );
			if ( !hal_private_dispatch )
				return false;

			auto hal_collect_pmc_counters = reinterpret_cast
				< std::uint64_t >( hal_private_dispatch->m_hal_collect_pmc_counters );
			if ( !hal_collect_pmc_counters )
				return false;

			if ( !paging::pth::create(
				hal_collect_pmc_counters,
				hal_collect_pmc_counters_hk,
				&hal_collect_pmc_counters_org,
				&m_hal_collect_pmc_counters ) )
				return false;

			auto etwp_debugger_data = nt::get_etwp_debugger_data( );
			if ( !etwp_debugger_data ) return false;

			auto etwp_debugger_data_silo = *reinterpret_cast< void*** >( etwp_debugger_data + 0x10 );
			if ( !etwp_debugger_data_silo ) return false;

			auto ckcl_wmi_logger = etwp_debugger_data_silo[ 2 ];
			if ( !ckcl_wmi_logger ) return false;

			auto ckcl_wmi_logger_id = *reinterpret_cast< std::uint32_t* >( ckcl_wmi_logger );
			if ( !ckcl_wmi_logger_id ) return false;

			auto etwp_max_pmc_counter = nt::get_etwp_max_pmc_counter( );
			if ( !etwp_max_pmc_counter ) return false;

			auto org_etwp_max_pmc_counter = *etwp_max_pmc_counter;
			if ( org_etwp_max_pmc_counter <= 1 ) *etwp_max_pmc_counter = 2;

			auto pmc_count = reinterpret_cast< event_trace_profile_counter_information_t* >(
				mmu::alloc_page( sizeof( event_trace_profile_counter_information_t ) ) );
			if ( !pmc_count ) {
				if ( org_etwp_max_pmc_counter <= 1 )
					*etwp_max_pmc_counter = org_etwp_max_pmc_counter;
				return false;
			}

			pmc_count->m_event_trace_information_class = event_trace_information_class_t::event_trace_profile_counter_list_information;
			pmc_count->m_trace_handle = reinterpret_cast< void* >( ckcl_wmi_logger_id );
			pmc_count->m_profile_source[ 0 ] = 1;

			auto status = nt::zw_set_system_information( system_information_class_t::system_performance_trace_information,
				pmc_count, sizeof( event_trace_profile_counter_information_t ) );

			if ( org_etwp_max_pmc_counter <= 1 )
				*etwp_max_pmc_counter = org_etwp_max_pmc_counter;

			mmu::free_page( reinterpret_cast< std::uint64_t >( pmc_count ), sizeof( event_trace_profile_counter_information_t ) );

			if ( status != nt_status_t::success ) return false;

			auto pmc_event = reinterpret_cast< event_trace_system_event_information_t* >(
				mmu::alloc_page( sizeof( event_trace_system_event_information_t ) ) );
			if ( !pmc_event ) return false;

			pmc_event->m_event_trace_information_class = event_trace_information_class_t::event_trace_profile_event_list_information;
			pmc_event->m_trace_handle = reinterpret_cast< void* >( ckcl_wmi_logger_id );
			pmc_event->m_hook_id[ 0 ] = 0xf33ul;

			status = nt::zw_set_system_information( system_information_class_t::system_performance_trace_information,
				pmc_event, sizeof( event_trace_system_event_information_t ) );

			mmu::free_page( reinterpret_cast< std::uint64_t >( pmc_event ), sizeof( event_trace_system_event_information_t ) );
			return status == nt_status_t::success;
		}

		bool unload( ) {
			if ( !configure_trace( ckcl_trace_operation_t::end ) )
				return false;

			auto hal_private_dispatch = nt::get_hal_private_dispatch( );
			if ( !hal_private_dispatch )
				return false;

			auto hal_collect_pmc_counters = reinterpret_cast
				< std::uint64_t >( hal_private_dispatch->m_hal_collect_pmc_counters );
			if ( !hal_collect_pmc_counters )
				return false;

			if ( !paging::pth::remove( hal_collect_pmc_counters, m_hal_collect_pmc_counters ) ) {
				return false;
			}

			auto etwp_debugger_data = nt::get_etwp_debugger_data( );
			if ( etwp_debugger_data ) {
				auto etwp_debugger_data_silo = *reinterpret_cast< void*** >( etwp_debugger_data + 0x10 );
				if ( nt::mm_is_address_valid( etwp_debugger_data_silo ) ) {
					auto ckcl_wmi_logger = etwp_debugger_data_silo[ 2 ];
					if ( nt::mm_is_address_valid( ckcl_wmi_logger ) ) {
						auto ckcl_wmi_logger_id = *reinterpret_cast< std::uint32_t* >( ckcl_wmi_logger );
						if ( ckcl_wmi_logger_id ) {
							auto pmc_count = reinterpret_cast< event_trace_profile_counter_information_t* >(
								mmu::alloc_page( sizeof( event_trace_profile_counter_information_t ) ) );

							if ( pmc_count ) {
								pmc_count->m_event_trace_information_class =
									event_trace_information_class_t::event_trace_profile_counter_list_information;
								pmc_count->m_trace_handle = reinterpret_cast< void* >( ckcl_wmi_logger_id );
								pmc_count->m_profile_source[ 0 ] = 0;

								nt::zw_set_system_information(
									system_information_class_t::system_performance_trace_information,
									pmc_count,
									sizeof( event_trace_profile_counter_information_t ) );

								mmu::free_page( reinterpret_cast< std::uint64_t >( pmc_count ),
									sizeof( event_trace_profile_counter_information_t ) );
							}

							auto pmc_event = reinterpret_cast< event_trace_system_event_information_t* >(
								mmu::alloc_page( sizeof( event_trace_system_event_information_t ) ) );

							if ( pmc_event ) {
								pmc_event->m_event_trace_information_class =
									event_trace_information_class_t::event_trace_profile_event_list_information;
								pmc_event->m_trace_handle = reinterpret_cast< void* >( ckcl_wmi_logger_id );
								pmc_event->m_hook_id[ 0 ] = 0;

								nt::zw_set_system_information(
									system_information_class_t::system_performance_trace_information,
									pmc_event,
									sizeof( event_trace_system_event_information_t ) );

								mmu::free_page( reinterpret_cast< std::uint64_t >( pmc_event ),
									sizeof( event_trace_system_event_information_t ) );
							}
						}
					}
				}
			}

			return true;
		}
	}

	namespace trace {
		struct trace_entry_t {
			std::uint32_t m_syscall_index;
			void* m_hook_handler;
			void** m_original_address;
		};

		constexpr std::size_t max_hooks = 64;
		trace_entry_t m_trace_hooks[ max_hooks ]{ };
		std::size_t m_count = 0;

		std::size_t get_trace_entry( std::uint32_t syscall_idx ) {
			for ( auto idx = 0; idx < max_hooks; idx++ ) {
				if ( m_trace_hooks[ idx ].m_syscall_index == syscall_idx )
					return idx;
			}
			return static_cast< std::size_t >( -1 );
		}

		void map_hook( std::uint32_t syscall_idx, void** target_address ) {
			if ( !target_address )
				return;

			auto hook_idx = get_trace_entry( syscall_idx );
			if ( hook_idx == static_cast< std::size_t >( -1 ) )
				return;

			auto& entry = m_trace_hooks[ hook_idx ];
			if ( !entry.m_original_address || !entry.m_hook_handler )
				return;

			*entry.m_original_address = *target_address;
			*target_address = entry.m_hook_handler;
		}

		template < typename org_t >
		bool create_hook( const char* function_name, void* hook_handler, org_t* orig_fn ) {
			if ( !hook_handler || !orig_fn )
				return false;

			if ( m_count >= max_hooks ) {
				nt::dbg_print( oxorany( "[ETW] hook failed: max hooks reached (%zu)\n" ), max_hooks );
				return false;
			}

			nt::dbg_print( oxorany( "[ETW] hook installed: count=0x%x\n" ), m_count );

			auto syscall_index = ssdt::get_syscall_index( function_name );
			if ( syscall_index == static_cast< std::uint32_t >( -1 ) ) {
				nt::dbg_print( oxorany( "[ETW] hook failed: could not get %s syscall index\n" ), function_name );
				return false;
			}

			nt::dbg_print( oxorany( "[ETW] hook installed: syscall=0x%x\n" ), syscall_index );

			auto hook_index = get_trace_entry( syscall_index );
			if ( hook_index != -1 ) {
				nt::dbg_print( oxorany( "[ETW] hook failed: hook already registered: %s (0x%x)\n" ), function_name, syscall_index );
				return false;
			}

			nt::dbg_print( oxorany( "[ETW] hook installed: hook_index=0x%x\n" ), hook_index );

			auto& entry = m_trace_hooks[ m_count++ ];
			entry.m_syscall_index = syscall_index;
			entry.m_hook_handler = hook_handler;
			entry.m_original_address = reinterpret_cast< void** >( orig_fn );
			return true;
		}
	}
}