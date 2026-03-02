#pragma once 

namespace map {
	class c_map {
	public:
		c_map( ) { }
		~c_map( ) { }
		c_map( std::shared_ptr< c_dependency > dependency ) : m_dependency( dependency ) { }

		bool create( ) {
			auto dependency_size = m_dependency->get_size( );
			auto dependency_name = m_dependency->get_file_name( );

			m_mapped_image.resize( dependency_size );
			std::memset( m_mapped_image.data( ), 0, dependency_size );

			this->m_dependency_base = reinterpret_cast< std::uint64_t >(
				nt::mm_allocate_independent_pages( dependency_size ) );
			if ( !m_dependency_base )
				return false;

			nt::memset( reinterpret_cast< void* >( m_dependency_base ), 0, dependency_size );
			if ( !nt::mm_set_page_protection( m_dependency_base, dependency_size, 0x40 ) )
				return false;

			logging::print( oxorany( "[c_map] create: Module size: 0x%i" ), dependency_size );
			logging::print( oxorany( "[c_map] create: Allocated module: 0x%llx" ), m_dependency_base );

			if ( !relocate( ) ) {
				logging::print( oxorany( "[c_map] create: Could not map relocations" ) );
				return false;
			}

			map_delayed_imports( );
			map_imports( );

			if ( !map_sections( ) ) {
				logging::print( oxorany( "[c_map] create: Could not map sections" ) );
				return false;
			}

			nt::memcpy(
				reinterpret_cast< void* >( m_dependency_base ),
				m_mapped_image.data( ),
				m_mapped_image.size( )
			);

			logging::print( oxorany( "[c_map] create: Created module mapping of %ls\n" ), dependency_name.c_str( ) );
			return true;
		}

		bool execute( ) {
			this->m_entry_point = m_dependency_base + m_dependency->get_entry_point( );
			if ( !m_entry_point ) {
				logging::print( oxorany( "[c_map] execute: Could not find EntryPoint" ) );
				return false;
			}

			if ( !map_pdb_symbols( ) ) {
				logging::print( oxorany( "[c_map] execute: Could not map PDB Symbols" ) );
				return false;
			}

			auto pte_address = g_paging->get_pte_address( m_entry_point );
			if ( pte_address ) {
				pte pt_entry{};
				if ( !g_dpm->read_physical_memory(
					pte_address,
					&pt_entry,
					sizeof( pte )
				) ) return false;

				logging::print( oxorany( "[c_map] execute: Present=%d Execute=%d Write=%d" ),
								pt_entry.hard.present,
								!pt_entry.hard.no_execute,
								pt_entry.hard.read_write
				);

				if ( !pt_entry.hard.present || pt_entry.hard.no_execute ) {
					logging::print( oxorany( "[c_map] execute: Unexpectedly could not page dependency base (%d,%d)" ),
									pt_entry.hard.present, pt_entry.hard.no_execute );
					return false;
				}
			}

			auto result = g_hook->call_kernel< bool >( 
				m_entry_point,
				m_dependency_base,
				m_mapped_image.size( ),
				m_pdb_symbols );
			if ( result ) {
				logging::print( oxorany( "[c_map] execute: Successfully executed payload\n" ) );
				return result;
			}

			logging::print( oxorany( "[c_map] execute: Payload failed to execute." ) );
			return result;
		}

	private:
		eprocess_t* m_target_process;
		std::shared_ptr< c_dependency > m_dependency;
		std::uint64_t m_dependency_base{ };
		std::uint64_t m_entry_point{ };
		std::uint64_t m_pdb_symbols{ };
		std::vector< std::uint8_t > m_mapped_image;

		bool map_pdb_symbols( ) {
			pdb_symbols_t pdb_symbols{
				.m_mi_copy_on_write = g_ntoskrnl->get_export( oxorany( "MiCopyOnWrite" ) ),
				.m_mm_set_page_protection = g_ntoskrnl->get_export( oxorany( "MmSetPageProtection" ) ),
				.m_mi_is_page_on_bad_list = g_ntoskrnl->get_export( oxorany( "MiIsPageOnBadList" ) ),
				.m_mi_make_page_bad = g_ntoskrnl->get_export( oxorany( "MiMakePageBad" ) ),
				.m_rtlp_debug_print_callback_list = g_ntoskrnl->get_export( oxorany( "RtlpDebugPrintCallbackList" ) ),
				.m_psp_load_image_notify_routine = g_ntoskrnl->get_export( oxorany( "PspLoadImageNotifyRoutine" ) ),
				.m_ki_is_process_termination_requested = g_ntoskrnl->get_export( oxorany( "KiIsProcessTerminationRequested" ) ),
				.m_ps_query_thread_start_address = g_ntoskrnl->get_export( oxorany( "PsQueryThreadStartAddress" ) ),
				.m_mi_lock_page_table_page = g_ntoskrnl->get_export( oxorany( "MiLockPageTablePage" ) ),
				.m_mm_initialize_process_address_space = g_ntoskrnl->get_export( oxorany( "MmInitializeProcessAddressSpace" ) ),
				.m_ke_get_prcb = g_ntoskrnl->get_export( oxorany( "KeGetPrcb" ) ),
				.m_ki_nmi_callback_list_head = g_ntoskrnl->get_export( oxorany( "KiNmiCallbackList" ) ),
				.m_po_idle = g_ntoskrnl->get_export( oxorany( "PoIdle" ) ),
				.m_etwp_max_pmc_counter = g_ntoskrnl->get_export( oxorany( "EtwpMaxPmcCounter" ) ),
				.m_etwp_debugger_data = g_ntoskrnl->get_export( oxorany( "EtwpDebuggerData" ) ),
				.m_mi_obtain_section_for_driver = g_ntoskrnl->get_export( oxorany( "MiObtainSectionForDriver" ) ),
				.m_mm_allocate_independent_pages = g_ntoskrnl->get_export( oxorany( "MmAllocateIndependentPagesEx" ) ),
				.m_mm_free_independent_pages = g_ntoskrnl->get_export( oxorany( "MmFreeIndependentPages" ) ),
				.m_ki_dynamic_trace_mask = g_ntoskrnl->get_export( oxorany( "KiDynamicTraceMask" ) ),
				.m_ps_enum_process_threads = g_ntoskrnl->get_export( oxorany( "PsEnumProcessThreads" ) ),
				.m_ps_integrity_check_enabled = g_ntoskrnl->get_export( oxorany( "PsIntegrityCheckEnabled" ) ),
				.m_exp_license_watch_init_worker = g_ntoskrnl->get_export( oxorany( "ExpLicenseWatchInitWorker" ) ),
				.m_ki_filter_fiber_context = g_ntoskrnl->get_export( oxorany( "KiFilterFiberContext" ) ),
				.m_ki_sw_interrupt_dispatch = g_ntoskrnl->get_export( oxorany( "KiSwInterruptDispatch" ) ),
				.m_fs_rtl_uninitialize_small_mcb = g_ntoskrnl->get_export( oxorany( "FsRtlUninitializeSmallMcb" ) ),
				.m_ki_dispatch_exception = g_ntoskrnl->get_export( oxorany( "KiDispatchException" ) ),
				.m_ki_expand_kernel_stack_and_callout = g_ntoskrnl->get_export( oxorany( "KeExpandKernelStackAndCallout" ) ),
				.m_fs_rtl_mdl_read_complete_dev_ex = g_ntoskrnl->get_export( oxorany( "FsRtlMdlReadCompleteDevEx" ) ),
				.m_ki_initialize_boot_structures = g_ntoskrnl->get_export( oxorany( "KiInitializeBootStructures" ) ),
				.m_halp_stall_counter = g_ntoskrnl->get_export( oxorany( "HalpStallCounter" ) ),
				.m_kd_trap = g_ntoskrnl->get_export( oxorany( "KdTrap" ) ),
				.m_ke_freeze_execution = g_ntoskrnl->get_export( oxorany( "KeFreezeExecution" ) ),
				.m_ke_stall_execution_processor = g_ntoskrnl->get_export( oxorany( "KeStallExecutionProcessor" ) ),
				.m_kdp_report = g_ntoskrnl->get_export( oxorany( "KdpReport" ) ),
				.m_kdp_trap = g_ntoskrnl->get_export( oxorany( "KdpTrap" ) ),
				.m_kd_enter_debugger = g_ntoskrnl->get_export( oxorany( "KdEnterDebugger" ) ),
				.m_nt_global_flag = g_ntoskrnl->get_export( oxorany( "NtGlobalFlag" ) ),
				.m_kdp_debug_routine_select = g_ntoskrnl->get_export( oxorany( "KdpDebugRoutineSelect" ) ),
				.m_kd_debugger_lock = g_ntoskrnl->get_export( oxorany( "KdDebuggerLock" ) ),
				.m_kd_ignore_um_exceptions = g_ntoskrnl->get_export( oxorany( "KdIgnoreUmExceptions" ) )
			};

			this->m_pdb_symbols = reinterpret_cast< std::uint64_t >(
				nt::mm_allocate_independent_pages( sizeof( pdb_symbols ) )
				);

			if ( !m_pdb_symbols )
				return false;

			nt::memcpy(
				reinterpret_cast< void* >( m_pdb_symbols ),
				&pdb_symbols, sizeof( pdb_symbols )
			);

			return true;
		}

		bool relocate( ) {
			auto delta_offset = m_dependency_base - m_dependency->get_image_base( );
			if ( m_dependency->is_reloc_stripped( ) ) {
				logging::print( oxorany( "[c_map] relocate: Depedency relocations stripped" ) );
				return false;
			}

			auto reloc_dir = m_dependency->get_reloc_directory( );
			if ( !reloc_dir ) {
				logging::print( oxorany( "[c_map] relocate: Could not find relocation directory" ) );
				return false;
			}

			auto reloc_entry = m_dependency->rva_to_va<reloc_entry_t*>( reloc_dir->m_virtual_address );
			if ( !reloc_entry ) {
				logging::print( oxorany( "[c_map] relocate: Could not find relocation entry" ) );
				return false;
			}

			auto reloc_end = reinterpret_cast< reloc_entry_t* >(
				reinterpret_cast< std::uint8_t* >( reloc_entry ) + reloc_dir->m_size
				);

			while ( reloc_entry < reloc_end && reloc_entry->m_size ) {
				auto record_count = ( reloc_entry->m_size - 8 ) >> 1;
				logging::print( oxorany( "[c_map] relocate: Relocating block with size: 0x%x" ), reloc_entry->m_size );

				for ( auto idx = 0u; idx < record_count; idx++ ) {
					auto offset = reloc_entry->m_item[ idx ].m_offset % 4096;
					auto type = reloc_entry->m_item[ idx ].m_type;
					if ( type == IMAGE_REL_BASED_ABSOLUTE )
						continue;

					auto reloc_addr = m_dependency->rva_to_va< std::uint64_t >( reloc_entry->m_to_rva );
					if ( !reloc_addr ) {
						logging::print( oxorany( "[c_map] relocate: Could not resolve relocation" ) );
						continue;
					}

					auto reloc_va = reinterpret_cast< std::uint64_t* >( reloc_addr + offset );
					auto original_va = *reloc_va;
					*reloc_va += delta_offset;
				}

				reloc_entry = reinterpret_cast< reloc_entry_t* >(
					reinterpret_cast< uint8_t* >( reloc_entry ) + reloc_entry->m_size
					);
			}

			return true;
		}

		bool map_imports( ) {
			auto import_table = m_dependency->get_import_table( );
			if ( !import_table->m_virtual_address || !import_table->m_size ) {
				logging::print( oxorany( "[c_map] map_imports: Could not find import table" ) );
				return false;
			}

			auto import_desc = m_dependency->rva_to_va< import_descriptor_t* >( import_table->m_virtual_address );
			while ( import_desc->m_name ) {
				auto module_name = m_dependency->rva_to_va< LPSTR >( import_desc->m_name );
				if ( !module_name ) {
					logging::print( oxorany( "[c_map] map_imports: Could not resolve import name" ) );
					break;
				}

				auto import_module = utility::get_kernel_module( module_name );
				if ( !import_module ) {
					logging::print( oxorany( "[c_map] map_imports: Could not resolve module" ) );
					return false;
				}

				auto first_thunk = m_dependency->rva_to_va< nt_image_thunk_data_t* >( import_desc->m_first_thunk );
				auto orig_first_thunk = m_dependency->rva_to_va< nt_image_thunk_data_t* >( import_desc->m_original_first_thunk );
				if ( !orig_first_thunk || !first_thunk ) {
					logging::print( oxorany( "[c_map] map_imports: Could not resolve thunk data" ) );
					break;
				}

				while ( first_thunk->u1.address_of_data ) {
					if ( IMAGE_SNAP_BY_ORDINAL64( orig_first_thunk->u1.ordinal ) ) {
						auto ordinal = IMAGE_ORDINAL64( orig_first_thunk->u1.ordinal );
						first_thunk->u1.function = import_module->get_export( ( const char* )ordinal );
					}
					else {
						auto ibn = m_dependency->rva_to_va< nt_image_import_name_t* >( orig_first_thunk->u1.address_of_data );
						if ( !ibn ) {
							logging::print( oxorany( "[c_map] map_imports: Could not get IBN" ) );
							return false;
						}

						first_thunk->u1.function = import_module->get_export( ibn->name );
					}

					if ( !first_thunk->u1.function ) {
						logging::print( oxorany( "[c_map] map_imports: Couldnt not resolve function" ) );
						return false;
					}

					orig_first_thunk++;
					first_thunk++;
				}

				crt::mem_zero( module_name, crt::str_len( module_name ) );
				import_desc++;
			}

			return true;
		}

		bool map_delayed_imports( ) {
			auto delay_descriptor = m_dependency->get_delay_import_descriptor( );
			if ( !delay_descriptor->m_virtual_address || !delay_descriptor->m_size ) {
				logging::print( oxorany( "[c_map] map_delayed_imports: Could not find delay import descriptor" ) );
				return false;
			}

			auto delay_desc = m_dependency->rva_to_va< image_delayload_descriptor_t* >( delay_descriptor->m_virtual_address );
			while ( delay_desc->m_dll_name_rva ) {
				auto module_name = m_dependency->rva_to_va< LPSTR >( delay_desc->m_dll_name_rva );
				if ( !module_name ) {
					logging::print( oxorany( "[c_map] map_delayed_imports: Could not resolve import name" ) );
					break;
				}

				auto import_module = utility::get_kernel_module( module_name );
				if ( !import_module ) {
					logging::print( oxorany( "[c_map] map_delayed_imports: Could not resolve module" ) );
					return false;
				}

				auto first_thunk = m_dependency->rva_to_va< nt_image_thunk_data_t* >( delay_desc->m_import_address_table_rva );
				auto orig_first_thunk = m_dependency->rva_to_va< nt_image_thunk_data_t* >( delay_desc->m_import_name_table_rva );
				if ( !orig_first_thunk || !first_thunk ) {
					logging::print( oxorany( "[c_map] map_delayed_imports: Could not resolve thunk data" ) );
					break;
				}

				while ( first_thunk->u1.address_of_data ) {
					if ( IMAGE_SNAP_BY_ORDINAL64( orig_first_thunk->u1.ordinal ) ) {
						auto ordinal = IMAGE_ORDINAL64( orig_first_thunk->u1.ordinal );
						first_thunk->u1.function = import_module->get_export( ( const char* )ordinal );
					}
					else {
						auto ibn = m_dependency->rva_to_va< nt_image_import_name_t* >( orig_first_thunk->u1.address_of_data );
						if ( !ibn ) {
							logging::print( oxorany( "[c_map] map_delayed_imports: Could not get IBN" ) );
							return false;
						}

						first_thunk->u1.function = import_module->get_export( ibn->name );
					}

					if ( !first_thunk->u1.function ) {
						logging::print( oxorany( "[c_map] map_delayed_imports: Couldnt not resolve function" ) );
						return false;
					}

					orig_first_thunk++;
					first_thunk++;
				}
			}

			return true;
		}

		bool map_sections( ) {
			auto section_count = m_dependency->get_section_count( );
			for ( auto idx = 0ul; idx < section_count; idx++ ) {
				auto section = m_dependency->get_section( idx );
				if ( !section->m_virtual_address || !section->m_size_of_raw_data )
					continue;

				auto section_buffer = m_mapped_image.data( ) + section->m_virtual_address;
				auto source_data = m_dependency->get_raw_image( ).data( ) + section->m_pointer_to_raw_data;
				std::memcpy( section_buffer, source_data, section->m_size_of_raw_data );

				logging::print( oxorany( "[c_map] map_sections: Mapped section %s" ), section->m_name );
			}

			return true;
		}
	};
}