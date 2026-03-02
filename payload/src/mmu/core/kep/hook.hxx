#pragma once

namespace kep {
	namespace hooks {
		bool unlink_debug_print_callback( ) {
			auto rtlp_debug_print_callback_list = nt::get_rtlp_debug_print_callback_list( );
			if ( !rtlp_debug_print_callback_list ) {
				nt::dbg_print( oxorany( "[KEP] unblock_debug_print fail: could not get callback list\n" ) );
				return false;
			}

			auto debug_print_callback = rtlp_debug_print_callback_list->m_flink;
			while ( debug_print_callback != rtlp_debug_print_callback_list ) {
				auto debug_print_callback_next = debug_print_callback->m_flink;
				auto debug_print_callback_entry = CONTAINING_RECORD( debug_print_callback,
					rtlp_debug_print_entry_t, m_list_entry );

				auto callback = debug_print_callback_entry->m_callback;
				if ( eac::memory::is_inside_module( callback ) ) {
					debug_print_callback->m_blink->m_flink = debug_print_callback->m_flink;
					debug_print_callback->m_flink->m_blink = debug_print_callback->m_blink;

					nt::dbg_print( oxorany( "[KEP] unblocked debug print\n" ) );
					return true;
				}

				debug_print_callback = debug_print_callback_next;
			}

			nt::dbg_print( oxorany( "[KEP] unblock_debug_print fail: could not get callback\n" ) );
			return false;
		}

		// Prevent them from doing TPM checks and loading libraries into the client
		void* ( *ex_allocate_pool_with_tag_org )( pool_type_t, std::size_t, ULONG ) = nullptr;
		void* ex_allocate_pool_with_tag( pool_type_t pool_type, std::size_t number_of_bytes, ULONG tag ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				nt::dbg_print( oxorany( "[KEP] LeProxy LTC -> LPTBBuKFxy6dCiQtiGn5hznrwfQHNMeknK\n" ) );

				//if ( /*number_of_bytes == 0x400 || number_of_bytes == 0x330 ||number_of_bytes == 0x212 || number_of_bytes == 0x200*/ ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible Module Report (Size: %i)\n" ), number_of_bytes );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x20 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Signed Module Load\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x8000 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible Hash Table\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x3C0 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible Telemetry Information\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x4D0 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible VM Instruction execution\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x68 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible Module Report\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0xC000 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Address in Module check\n" ) );
				//}

				//if ( number_of_bytes == 0x100 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Module Export\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x1000 ) {
				//	nt::dbg_print( oxorany( "[KEP] blocked tpm (or process) check\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x1004 ) {
				//	nt::dbg_print( oxorany( "[KEP] blocked pci device check\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x1260 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible Process check\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x2000 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible Stackwalk\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x6200 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible Mapped Module Check\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x100000 || number_of_bytes == 0x800 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible PE Header check\n" ) );
				//	return nullptr;
				//}

				//if ( number_of_bytes == 0x4D0 ) {
				//	nt::dbg_print( oxorany( "[KEP] Blocked Possible Module check\n" ) );
				//	return nullptr;
				//}
			}

			return ex_allocate_pool_with_tag_org( pool_type, number_of_bytes, tag );
		}

		// Block them for scanning physical memory
		nt_status_t( *mm_copy_memory_org )(
			void*,
			mm_copy_address_t,
			std::size_t,
			std::uint32_t,
			std::size_t* ) = nullptr;
		nt_status_t mm_copy_memory(
			void* target_address,
			mm_copy_address_t source_address,
			std::size_t number_of_bytes,
			std::uint32_t flags,
			std::size_t* number_of_bytes_transferred
		) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				return nt_status_t::unsuccessful;
			}

			return mm_copy_memory_org( target_address, source_address, number_of_bytes, flags, number_of_bytes_transferred );
		}

		// Block them from stack walking
		runtime_function_t* ( *rtl_lookup_function_entry_org )(
			std::uint64_t,
			std::uint64_t*,
			unwind_history_table_t* ) = nullptr;
		runtime_function_t* rtl_lookup_function_entry(
			std::uint64_t control_pc, std::uint64_t* image_base, unwind_history_table_t* history_table ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				return nullptr;
			}

			return rtl_lookup_function_entry_org( control_pc, image_base, history_table );
		}

		nt_status_t( *nt_query_symbolic_link_object_org )( void*, unicode_string_t*, unsigned long* ) = nullptr;
		nt_status_t nt_query_symbolic_link_object( void* sym_link_obj_handle, unicode_string_t* link_target, unsigned long* data_written ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				nt::dbg_print( oxorany( "[KEP] NtQuerySymbolicLinkObject result:\n" ) );
				if ( link_target && link_target->m_buffer && link_target->m_length > 0 ) {
					nt::dbg_print( oxorany( "    Target: %wZ\n" ), link_target );
				}
			}

			return nt_query_symbolic_link_object_org( sym_link_obj_handle, link_target, data_written );
		}

		nt_status_t( *nt_query_value_key_org )( void*, unicode_string_t*, e_key_value_information_class, void*, unsigned long, unsigned long* );
		nt_status_t nt_query_value_key(
			void* key_handle,
			unicode_string_t* value_name,
			e_key_value_information_class key_value_information_class,
			void* key_value_information,
			unsigned long length,
			unsigned long* result_length ) {
			//if ( eac::memory::is_inside_module( return_address ) ) {
			//	nt::dbg_print( oxorany( "[KEP] NtQueryValueKey result:\n" ) );
			//	if ( value_name && value_name->m_buffer && value_name->m_length > 0 ) {
			//		nt::dbg_print( oxorany( "    Target: %wZ\n" ), value_name );
			//	}
			//}

			return nt_query_value_key_org( key_handle, value_name, key_value_information_class, key_value_information, length, result_length );
		}

		// Block them from preventing DbgPrints
		nt_status_t( *dbg_set_debug_print_callback_org )( void*, bool ) = nullptr;
		nt_status_t dbg_set_debug_print_callback( void* debug_print_callback, bool enable ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				auto result = dbg_set_debug_print_callback_org( debug_print_callback, enable );
				if ( !result )
					unlink_debug_print_callback( );

				return result;
			}

			return dbg_set_debug_print_callback_org( debug_print_callback, enable );
		}

		// Prevent them from getting driver information
		nt_status_t( *nt_query_information_file_org )( void*, io_status_block_t*, void*, unsigned long, unsigned long ) = nullptr;
		nt_status_t nt_query_information_file(
			void* file_handle,
			io_status_block_t* io_status_block,
			void* file_information,
			unsigned long length,
			unsigned long file_information_class
		) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				void* file_object = nullptr;
				auto result = nt::ob_reference_object_by_handle(
					file_handle, 0, nt::io_file_object_type( ), 0, &file_object, 0
				);

				if ( !result ) {
					auto file_obj = reinterpret_cast< file_object_t* >( file_object );
					if ( file_obj->m_file_name.m_buffer && file_obj->m_file_name.m_length ) {
						auto filename = file_obj->m_file_name.m_buffer;
						auto filename_len = file_obj->m_file_name.m_length / sizeof( wchar_t );
						if ( filename_len >= 4 ) {
							auto end_tail = &filename[ filename_len - 4 ];
							if ( !_wcsicmp( end_tail, oxorany( L".sys" ) ) ) {
								nt::dbg_print( oxorany( "[KEP] blocked access to: %ls\n" ), filename );

								unicode_string_t file_name{ };
								nt::rtl_init_unicode_string( &file_name, oxorany( L"\\SystemRoot\\System32\\drivers\\ntfs.sys" ) );

								object_attributes_t obj_attr{ };
								obj_attr.m_object_name = &file_name;
								obj_attr.m_attributes = 0x00000040L | 0x00000200L;

								void* new_handle{ };
								io_status_block_t io_status{ };
								auto result = nt::zw_create_file(
									&new_handle,
									0x0080 | 0x00100000L,
									&obj_attr,
									&io_status,
									nullptr,
									0x00000080,
									0x00000001 | 0x00000002,
									0x00000001,
									0x00000040 | 0x00000020,
									nullptr,
									0
								);

								if ( !result ) {
									result = nt_query_information_file_org(
										new_handle, io_status_block, file_information, length, file_information_class
									);

									nt::ob_dereference_object( file_object );
									nt::zw_close( new_handle );

									return result;
								}
							}
						}
					}

					nt::ob_dereference_object( file_object );
				}
			}

			return nt_query_information_file_org( file_handle, io_status_block, file_information, length, file_information_class );
		}

		// Prevent them from getting DISK and HWID information
		nt_status_t( __stdcall* io_get_device_interfaces_org )( const guid_t*, device_object_t*, unsigned long, wchar_t** ) = nullptr;
		nt_status_t __stdcall io_get_device_interfaces(
			const guid_t* interface_class_guid,
			device_object_t* physical_device_object,
			unsigned long flags,
			wchar_t** symbolic_link_list
		) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				nt::dbg_print( oxorany( "[KEP] blocked hardware id\n" ) );

				auto empty_list = reinterpret_cast< wchar_t* >( nt::ex_allocate_pool_with_tag(
					1,
					sizeof( wchar_t ),
					oxorany( 0x20207050 )
				) );

				if ( empty_list ) {
					*empty_list = L'\0';
					if ( symbolic_link_list )
						*symbolic_link_list = empty_list;
					return nt_status_t::success;
				}
			}

			return io_get_device_interfaces_org( interface_class_guid, physical_device_object, flags, symbolic_link_list );
		}

		// Prevent them from doing protected process checks
		__int64( __fastcall* ps_get_process_inherited_from_unique_process_id_org )( eprocess_t* ) = nullptr;
		__int64 __fastcall ps_get_process_inherited_from_unique_process_id( eprocess_t* process ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				return 0;
			}

			return ps_get_process_inherited_from_unique_process_id_org( process );
		}

		// Prevent them from getting big pools, HVCI, handles, processes, debuggers, and e.g..
		nt_status_t( __stdcall* nt_query_system_information_org )( system_information_class_t, void*, unsigned long, unsigned long* ) = nullptr;
		nt_status_t __stdcall nt_query_system_information(
			system_information_class_t system_information_class,
			void* system_information,
			unsigned long system_information_length,
			unsigned long* return_length
		) {
			//if ( eac::memory::is_inside_module( return_address ) ) {
				switch ( system_information_class ) {
					case system_information_class_t::system_process_information: {
						nt::dbg_print( oxorany( "[KEP] system_process_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_call_time_information: {
						nt::dbg_print( oxorany( "[KEP] system_call_time_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_interrupt_information: {
						nt::dbg_print( oxorany( "[KEP] system_interrupt_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_handle_information: {
						nt::dbg_print( oxorany( "[KEP] system_handle_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_kernel_debugger_information: {
						nt::dbg_print( oxorany( "[KEP] system_kernel_debugger_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_physical_memory_information: {
						nt::dbg_print( oxorany( "[KEP] system_physical_memory_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_kernel_debugger_information_ex: {
						nt::dbg_print( oxorany( "[KEP] system_kernel_debugger_information_ex\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_code_integrity_information: {
						nt::dbg_print( oxorany( "[KEP] system_code_integrity_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_boot_environment_information: {
						nt::dbg_print( oxorany( "[KEP] system_boot_environment_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_big_pool_information: {
						nt::dbg_print( oxorany( "[KEP] system_big_pool_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case system_information_class_t::system_pool_tag_information: {
						nt::dbg_print( oxorany( "[KEP] system_pool_tag_information\n" ) );
						return nt_status_t::unsuccessful;
					} break;
					case 0x91 /*SystemSecureBootInformation*/: {
						nt::dbg_print( oxorany( "[KEP] SystemSecureBootInformation\n" ) );
						return nt_status_t::unsuccessful;
					} break;
				}
			//}

			return nt_query_system_information_org(
				system_information_class, system_information,
				system_information_length, return_length
			);
		}

		// Prevent them from TPM checks
		std::uint32_t( __stdcall* tbsi_context_create_org )( void*, void** ) = nullptr;
		std::uint32_t __stdcall tbsi_context_create( void* context_params, void** context ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				nt::dbg_print( oxorany( "[KEP] tpm blocked\n" ) );
				//return 0x80284001; // TBS_E_INTERNAL_ERROR
			}

			return tbsi_context_create_org( context_params, context );
		}

		// Prevent them from taking screenshots
		ethread_t* ( __fastcall* ps_get_thread_win32_thread_org )( ethread_t* a1 ) = nullptr;
		ethread_t* __fastcall ps_get_thread_win32_thread( ethread_t* thread ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				nt::dbg_print( oxorany( "[KEP] blocked overlay check\n" ) );
				return nullptr;
			}

			return ps_get_thread_win32_thread_org( thread );
		}

		nt_status_t( __stdcall* io_enumerate_device_object_list_org )( driver_object_t*, device_object_t**, unsigned long, unsigned long* ) = nullptr;
		nt_status_t __stdcall io_enumerate_device_object_list(
			driver_object_t* driver_object,
			device_object_t** device_object_list,
			unsigned long device_object_list_size,
			unsigned long* actual_aumber_device_objects ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				nt::dbg_print( oxorany( "[KEP] blocked device disk.\n" ) );
				return nt_status_t::unsuccessful;
			}

			return io_enumerate_device_object_list_org( driver_object, device_object_list, device_object_list_size, actual_aumber_device_objects );
		}

		nt_status_t( *nt_write_file_org )( void*, void*, io_apc_routine_t, void*, io_status_block_t*, void*, std::uint32_t, ularge_integer_t*, std::uint32_t* ) = nullptr;
		nt_status_t nt_write_file(
			void* file_handle,
			void* event,
			io_apc_routine_t apc_routine,
			void* apc_context,
			io_status_block_t* io_status_block,
			void* buffer,
			std::uint32_t length,
			ularge_integer_t* byte_offset,
			std::uint32_t* key
		) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				nt::dbg_print( oxorany( "[KEP] blocked eac injection\n" ) );
				return nt_status_t::access_violation;
			}

			return nt_write_file_org(
				file_handle,
				event,
				apc_routine,
				apc_context,
				io_status_block,
				buffer,
				length,
				byte_offset,
				key
			);
		}

		nt_status_t( __fastcall* nt_query_information_thread_org )( __m128i*, int, __m128i*, unsigned int, unsigned int* ) = nullptr;
		nt_status_t __fastcall nt_query_information_thread( __m128i* handle, int a2, __m128i* a3, unsigned int a4, unsigned int* a5 ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				switch ( a2 ) {
					case 0: {
						return nt_status_t::unsuccessful;
					} break;
					default: {
					}
				}
			}
			return nt_query_information_thread_org( handle, a2, a3, a4, a5 );
		}

		nt_status_t( __fastcall* nt_query_information_process_org )( HANDLE, int, unsigned __int64, unsigned int, unsigned int* ) = nullptr;
		nt_status_t __fastcall nt_query_information_process( HANDLE handle, int a2, unsigned __int64 a3, unsigned int a4, unsigned int* a5 ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				switch ( a2 ) {
					case 9: {
						return nt_status_t::success;
					} break;
					case 27: {
						return nt_status_t::success;
					} break;
					case 29: {
						return nt_status_t::success;
					} break;
					case 31: {
						return nt_status_t::success;
					} break;
					default: {
						return nt_status_t::unsuccessful;
					}
				}
			}

			return nt_query_information_process_org( handle, a2, a3, a4, a5 );
		}

		nt_status_t( *ps_lookup_process_by_pid_org )( void*, eprocess_t** ) = nullptr;
		nt_status_t ps_lookup_process_by_pid( void* process_id, eprocess_t** process ) {
			if ( eac::memory::is_inside_module( return_address ) ) {
				auto result = ps_lookup_process_by_pid_org( process_id, process );
				if ( result )
					return result;

				auto image_file_name = process::get_image_file_name( *process );
				if ( std::strstr_lowercase( image_file_name, oxorany( "Fortnite" ) ) ) {
					nt::dbg_print( oxorany( "[KEP] blocked lookup for: %s\n" ), image_file_name );

					//auto eac_process = process::get_eprocess( oxorany( "EasyAntiCheat_EOS.exe" ) );
					//if ( eac_process ) {
					//	*process = eac_process;
					//}
					//else
					//	nt::dbg_print( oxorany( "[KEP] could not get EasyAntiCheat_EOS.exe\n" ) );

					auto clone_base = mmu::alloc_page( paging::page_4kb_size );
					if ( !clone_base )
						return nt_status_t::unsuccessful;

					const auto orig_page = ( reinterpret_cast< std::uintptr_t >( *process ) >> 12 ) << 12;
					const auto offset = reinterpret_cast< std::uintptr_t >( *process ) & 0xFFF;

					std::memcpy( reinterpret_cast< void* >( clone_base ), reinterpret_cast< void* >( orig_page ), paging::page_4kb_size );

					*process = reinterpret_cast< eprocess_t* >( clone_base + offset );
					return nt_status_t::success;
				}
			}

			return ps_lookup_process_by_pid_org( process_id, process );
		}
	}

	bool register_hooks( ) {
		nt::dbg_print( oxorany( "[KEP] installing hooking routines\n" ) );

		kep::create_hook(
			nt::m_module_base,
			oxorany( "ExAllocatePoolWithTag" ),
			hooks::ex_allocate_pool_with_tag,
			&hooks::ex_allocate_pool_with_tag_org
		);

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "DbgSetDebugPrintCallback" ),
			hooks::dbg_set_debug_print_callback,
			&hooks::dbg_set_debug_print_callback_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "MmCopyMemory" ),
			hooks::mm_copy_memory,
			&hooks::mm_copy_memory_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "RtlLookupFunctionEntry" ),
			hooks::rtl_lookup_function_entry,
			&hooks::rtl_lookup_function_entry_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "IoGetDeviceInterfaces" ),
			hooks::io_get_device_interfaces,
			&hooks::io_get_device_interfaces_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "PsGetProcessInheritedFromUniqueProcessId" ),
			hooks::ps_get_process_inherited_from_unique_process_id,
			&hooks::ps_get_process_inherited_from_unique_process_id_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "NtQuerySystemInformation" ),
			hooks::nt_query_system_information,
			&hooks::nt_query_system_information_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "IoEnumerateDeviceObjectList" ),
			hooks::io_enumerate_device_object_list,
			&hooks::io_enumerate_device_object_list_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "PsGetThreadWin32Thread" ),
			hooks::ps_get_thread_win32_thread,
			&hooks::ps_get_thread_win32_thread_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "NtQueryInformationFile" ),
			hooks::nt_query_information_file,
			&hooks::nt_query_information_file_org
		) ) return false;

		if ( !create_hook(
			nt::m_module_base,
			oxorany( "NtWriteFile" ),
			hooks::nt_write_file,
			&hooks::nt_write_file_org
		) ) return false;

		//if ( !create_hook(
		//	nt::m_module_base,
		//	oxorany( "PsLookupProcessByProcessId" ),
		//	hooks::ps_lookup_process_by_pid,
		//	&hooks::ps_lookup_process_by_pid_org
		//) ) return false;

		//if ( !create_hook(
		//	nt::m_module_base,
		//	oxorany( "RtlWalkFrameChain" ),
		//	hooks::rtl_walk_frame_chain,
		//	&hooks::rtl_walk_frame_chain_org
		//) ) return false;

		auto hook_count = get_next_slot( );
		nt::dbg_print( oxorany( "[KSE] installed routines: hook count: %i\n" ), hook_count );
		return true;
	}

	void unhook_routine( ) {
		mmu::sleep( 9000 );
		remove_hook( nt::m_module_base, oxorany( "ExAllocatePoolWithTag" ) );
		remove_hook( nt::m_module_base, oxorany( "DbgSetDebugPrintCallback" ) );
		remove_hook( nt::m_module_base, oxorany( "MmCopyMemory" ) );
		remove_hook( nt::m_module_base, oxorany( "RtlLookupFunctionEntry" ) );
		remove_hook( nt::m_module_base, oxorany( "IoGetDeviceInterfaces" ) );
		remove_hook( nt::m_module_base, oxorany( "PsGetProcessInheritedFromUniqueProcessId" ) );
		remove_hook( nt::m_module_base, oxorany( "IoEnumerateDeviceObjectList" ) );
		remove_hook( nt::m_module_base, oxorany( "PsGetThreadWin32Thread" ) );
		remove_hook( nt::m_module_base, oxorany( "NtQueryInformationFile" ) );
		remove_hook( nt::m_module_base, oxorany( "NtWriteFile" ) );
		remove_hook( nt::m_module_base, oxorany( "RtlWalkFrameChain" ) );
		//remove_hook( nt::m_module_base, oxorany( "NtQueryValueKey" ) );
	}
}