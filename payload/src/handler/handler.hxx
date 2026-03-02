#pragma once

namespace handler {
	using namespace client;
	void* m_wnf_subscription{ };
	void* m_wnf_rundown{ };
	void* m_wnf_ctx{ };

	bool create_rundown( ) {
		nt::ex_initialize_rundown_protection( &m_wnf_rundown );
		m_wnf_ctx = nt::ex_allocate_pool_with_tag( 64, sizeof( std::uint32_t ), oxorany( 'Wnfx' ) );
		if ( !m_wnf_ctx )
			return false;


		memset( m_wnf_ctx, 0, sizeof( std::uint32_t ) );
		return true;
	}

	nt_status_t subscribe_callback( ) {
		if ( !client::is_attached( ) ) {
			return nt_status_t::success;
		}

		auto result = nt::ke_wait_for_single_object(
			m_request_handle,
			0,
			0,
			false,
			0
		);

		if ( result == nt_status_t::timeout ) {
			return nt_status_t::success;
		}

		m_data_request->m_status = true;
		switch ( m_data_request->m_request_type ) {
		case control::data_type::ping: { } break;
		case control::data_type::update_root: {
			m_data_request->m_message_count = client::update_root_logger( );
		} break;
		case control::data_type::get_eprocess: {
			m_data_request->m_process = process::get_eprocess( m_data_request->m_process_id );
		} break;
		case control::data_type::get_process_peb: {
			m_data_request->m_process_peb = process::get_process_peb( m_data_request->m_process );
		} break;
		case control::data_type::get_base_address: {
			m_data_request->m_address2 = process::get_section_base_address( m_data_request->m_process );
		} break;
		case control::data_type::get_process_cr3: {
			auto process = m_data_request->m_process;
			auto section_base = reinterpret_cast< std::uint64_t >(
				process::get_section_base_address( process )
				);

			m_data_request->m_status = nt::get_os_version( ) >= 26200
				? paging::get_process_cr3( section_base, &m_data_request->m_address1 )
				: lbr::get_process_cr3( process, &m_data_request->m_address1 );
		} break;
		case control::data_type::swap_context: {
			paging::attach_to_process(
				m_data_request->m_address,
				&m_data_request->m_address1
			);
		} break;
		case control::data_type::hyperspace_create: {
			m_data_request->m_status =
				paging::hyperspace::create_hyperspace(
					m_data_request->m_process
				);
		} break;
		case control::data_type::hyperspace_allocate: {
			m_data_request->m_address =
				paging::hyperspace::allocate_in_hyperspace(
					m_data_request->m_process,
					m_data_request->m_size,
					m_data_request->m_high_address
				);
		} break;
		case control::data_type::threads_to_hyperspace: {
			m_data_request->m_status =
				paging::hyperspace::switch_to_hyperspace(
					m_data_request->m_thread_id,
					m_data_request->m_process
				);
		} break;
		case control::data_type::threads_from_hyperspace: {
			m_data_request->m_status =
				paging::hyperspace::switch_from_hyperspace(
					m_data_request->m_thread_id,
					m_data_request->m_process
				);
		} break;
		case control::data_type::cow_create_exception: {
			auto pte_address = paging::get_pte_address( m_data_request->m_address );
			if ( pte_address ) {
				eprocess_t* org_process = nullptr;
				process::attach( m_data_request->m_process, &org_process );

				auto result = nt::mi_copy_on_write(
					m_data_request->m_address,
					reinterpret_cast< std::uint64_t* >( pte_address )
				);

				m_data_request->m_status = !result;
				root::push( oxorany( "[COW] copy_on_exception: COW result: 0x%x" ), result );

				process::attach( org_process );
			}
			else
				m_data_request->m_status = false;
		} break;
		case control::data_type::eac_ping: {
			m_data_request->m_status = eac::is_active( );
		} break;
		case control::data_type::eac_attach_process: {
			m_data_request->m_status = eac::memory::attach_process(
				m_data_request->m_process,
				&m_data_request->m_apc_state
			);
		} break;
		case control::data_type::eac_detach_process: {
			eac::memory::detach_process(
				m_data_request->m_process,
				&m_data_request->m_apc_state
			);
		} break;
		case control::data_type::eac_read_virtual: {
			m_data_request->m_status = eac::memory::read_virtual(
				m_data_request->m_process,
				m_data_request->m_address,
				m_data_request->m_address2,
				m_data_request->m_size
			);
		} break;
		case control::data_type::eac_write_virtual: {
			m_data_request->m_status = eac::memory::write_virtual(
				m_data_request->m_process,
				m_data_request->m_address,
				m_data_request->m_address2,
				m_data_request->m_size
			);
		} break;
		case control::data_type::eac_allocate_virtual: {
			m_data_request->m_address = eac::memory::allocate_virtual(
				m_data_request->m_process_id,
				m_data_request->m_size,
				m_data_request->m_protection
			);
		} break;
		case control::data_type::eac_page_memory: {
			m_data_request->m_status = eac::memory::page_memory(
				m_data_request->m_process_id,
				reinterpret_cast< void* >( m_data_request->m_address ),
				m_data_request->m_size
			);
		} break;
		case control::data_type::eac_flush_virtual: {
			m_data_request->m_status = eac::memory::flush_virtual(
				m_data_request->m_process,
				&m_data_request->m_address,
				&m_data_request->m_size
			);
		} break;
		case control::data_type::eac_protect_virtual: {
			m_data_request->m_status = eac::memory::protect_virtual(
				m_data_request->m_process,
				&m_data_request->m_address,
				&m_data_request->m_size,
				m_data_request->m_protection,
				&m_data_request->m_old_protection
			);
		} break;
		case control::data_type::eac_set_information: {
			m_data_request->m_status = eac::memory::set_information_virtual(
				m_data_request->m_process,
				reinterpret_cast< void* >( m_data_request->m_address ),
				m_data_request->m_size
			);
		} break;
		case control::data_type::eac_query_virtual: {
			void* process_handle = nullptr;
			auto result = nt::ob_open_object_by_pointer(
				m_data_request->m_process,
				0x200,
				nullptr,
				PROCESS_ALL_ACCESS,
				nt::ps_process_type( ),
				0,
				&process_handle
			);

			if ( !result && process_handle ) {
				m_data_request->m_status = eac::memory::query_virtual(
					process_handle,
					reinterpret_cast< void* >( m_data_request->m_address ),
					0,
					&m_data_request->m_memory_info,
					sizeof( memory_basic_information_t ),
					0
				);

				nt::zw_close( process_handle );
			}
			else {
				m_data_request->m_status = false;
			}
		} break;
		case control::data_type::eac_free_virtual: {
			m_data_request->m_status = eac::memory::free_virtual(
				m_data_request->m_process_id,
				m_data_request->m_address,
				m_data_request->m_size
			);
		} break;
		case control::data_type::vad_allocate_virtual: {
			m_data_request->m_address = vad::allocate_virtual(
				m_data_request->m_process,
				m_data_request->m_size,
				mmu::protection_style::readwrite
			);
		} break;
		case control::data_type::remap_allocate_virtual: {
			m_data_request->m_status = paging::hyperspace::create_hyperspace(
				m_data_request->m_process );
			if ( m_data_request->m_status ) {
				auto base_address = paging::hyperspace::allocate_in_hyperspace(
					m_data_request->m_process,
					m_data_request->m_size
				);

				if ( base_address ) {
					std::uint64_t old_cr3;
					paging::attach_to_process(
						process::get_directory_table_base( paging::hyperspace::m_entries
							[ paging::hyperspace::find_entry( m_data_request->m_process ) ].m_clone_process ),
						&old_cr3 );

					m_data_request->m_address1 = paging::remap_physical_page(
						m_data_request->m_address,
						base_address
					);

					paging::attach_to_process( m_data_request->m_address );
				}
			}
		} break;
		case control::data_type::remap_physical_page: {
			m_data_request->m_address1 = paging::remap_physical_page(
				m_data_request->m_address,
				reinterpret_cast< std::uint64_t >( m_data_request->m_address2 )
			);
		} break;
		case control::data_type::set_page_protection: {
			m_data_request->m_status = paging::change_page_protection(
				m_data_request->m_address,
				m_data_request->m_page_protection,
				m_data_request->m_user_supervisor );
		} break;
		case control::data_type::hyperspace_entries: {
			m_data_request->m_status = paging::hyperspace_entries(
				m_data_request->m_pt_entries,
				m_data_request->m_address );
		} break;
		case control::data_type::translate_linear: {
			m_data_request->m_status = paging::translate_linear(
				m_data_request->m_address,
				&m_data_request->m_address1 );
		} break;
		case control::data_type::map_process_page: {
			m_data_request->m_address2 = paging::ptm::map_page(
				m_data_request->m_address,
				m_data_request->m_page_protection,
				m_data_request->m_user_supervisor );
		} break;
		case control::data_type::read_virtual: {
			auto read_virtual = [ ]( std::uint64_t virtual_address, void* buffer, std::size_t size ) -> bool {
				if ( !mmu::is_user_address( virtual_address, size, 1 ) )
					return false;

				auto current_buffer = static_cast< std::uint8_t* >( buffer );
				auto current_va = virtual_address;
				auto remaining = size;

				while ( remaining > 0 ) {
					std::uint32_t page_size = 0;
					std::uint64_t physical_address = 0;
					if ( !paging::translate_linear( current_va, &physical_address, &page_size ) ) {
						return false;
					}

					auto page_offset = current_va & ( static_cast< unsigned long long >( page_size ) - 1 );
					auto read_size = min( static_cast< std::size_t >( page_size - page_offset ), remaining );

					if ( !paging::dpm::read_physical( physical_address, current_buffer, read_size ) )
						return false;

					current_va += read_size;
					current_buffer += read_size;
					remaining -= read_size;
				}

				return true;
				};

			m_data_request->m_status = read_virtual(
				m_data_request->m_address,
				m_data_request->m_address2,
				m_data_request->m_size
			);
		} break;
		case control::data_type::write_virtual: {
			auto write_virtual = [ ]( std::uint64_t virtual_address, void* buffer, std::size_t size ) -> bool {
					if ( !mmu::is_user_address( virtual_address, size, 1 ) )
						return false;

					auto current_buffer = static_cast< std::uint8_t* >( buffer );
					auto current_va = virtual_address;
					auto remaining = size;

					while ( remaining > 0 ) {
						std::uint32_t page_size = 0;
						std::uint64_t physical_address = 0;
						if ( !paging::translate_linear( current_va, &physical_address, &page_size ) ) {
							return false;
						}

						auto page_offset = current_va & ( static_cast<unsigned long long>( page_size ) - 1 );
						auto write_size = min( static_cast< std::size_t >( page_size - page_offset ), remaining );
						if ( !paging::dpm::write_physical( physical_address, current_buffer, write_size ) )
							return false;

						current_va += write_size;
						current_buffer += write_size;
						remaining -= write_size;
					}

					return true;
				};

			m_data_request->m_status = write_virtual(
				m_data_request->m_address,
				m_data_request->m_address2,
				m_data_request->m_size
			);
		} break;
		case control::data_type::read_physical: {
			void* buffer;
			m_data_request->m_status = paging::dpm::read_physical(
				m_data_request->m_address,
				&buffer,
				m_data_request->m_size );
		} break;
		case control::data_type::write_physical: {
			m_data_request->m_status = paging::dpm::write_physical(
				m_data_request->m_address,
				m_data_request->m_address2,
				m_data_request->m_size );
		} break;
		case control::data_type::attach_process: {
			m_data_request->m_status = process::attach(
				m_data_request->m_process,
				&m_data_request->m_old_process );
		} break;
		case control::data_type::detach_process: {
			m_data_request->m_status = process::attach( m_data_request->m_old_process );
		} break;
		case control::data_type::unload_driver: {
			client::destory( );
		} break;
		case control::data_type::unload_hook: {
			client::destory( );
			callback::remove_image( );
			callback::remove_process( );
			etw::hook::unload( );
			paging::dpm::cleanup( );
		} break;
		default: {
			m_data_request->m_status = false;
		} break;
		}

		if ( m_response_handle ) {
			nt::ke_release_semaphore( m_response_handle, 0, 1, false );
		}

		return nt_status_t::success;
	}

	void process_callback( HANDLE parent_id, HANDLE process_id, bool create ) {
		if ( create ) {
			auto current_process = nt::ps_lookup_process_by_pid(
				reinterpret_cast< std::uint32_t >( process_id ) );
			if ( !current_process )
				return;

			auto image_file_name = process::get_image_file_name( current_process );
			if ( !image_file_name )
				return;

			if ( !std::strcmp( image_file_name, oxorany( "hyperspace.exe" ) ) ) {
				if ( !client::initialize( reinterpret_cast< std::uint32_t >( parent_id ) ) ) {
					nt::dbg_print( oxorany( "[process] callback routine: could not connect to process\n" ) );
					client::destory( );
				}
			}
		}
	}

	void image_callback( unicode_string_t* image_name, HANDLE process_id, image_info_t* image_info ) {
		auto file_name = wcsrchr( image_name->m_buffer, oxorany( L'\\' ) );
		if ( file_name )
			file_name++;
		else
			file_name = image_name->m_buffer;

		if ( !wcscmp( file_name, oxorany( L"EasyAntiCheat_EOS.sys" ) ) ) {
			eac::initialize( image_info );
		}
	}
}