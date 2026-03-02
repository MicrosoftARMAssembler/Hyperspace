#pragma once

namespace paging {
	namespace hyperspace {
		struct hyperspace_entry_t {
			eprocess_t* m_orig_process;
			eprocess_t* m_clone_process;
			std::uint64_t m_clone_base;
			bool m_active;
		};

		constexpr std::uint32_t max_entries = 512;
		hyperspace_entry_t m_entries[ max_entries ] = {};

		std::int32_t find_entry( eprocess_t* process ) {
			for ( auto i = 0; i < max_entries; i++ ) {
				if ( m_entries[ i ].m_active && m_entries[ i ].m_orig_process == process )
					return i;
			}
			return -1;
		}

		std::int32_t find_by_clone( eprocess_t* clone ) {
			for ( auto i = 0; i < max_entries; i++ ) {
				if ( m_entries[ i ].m_active && m_entries[ i ].m_clone_process == clone )
					return i;
			}
			return -1;
		}

		std::int32_t find_free_slot( ) {
			for ( auto i = 0; i < max_entries; i++ ) {
				if ( !m_entries[ i ].m_active )
					return i;
			}
			return -1;
		}

		bool clone_eprocess( eprocess_t* target_process ) {
			auto slot = find_free_slot( );
			if ( slot == -1 ) {
				nt::dbg_print( oxorany( "[hyperspace] No free slots\n" ) );
				return false;
			}

			auto clone_base = mmu::alloc_page( page_4kb_size );
			if ( !clone_base )
				return false;

			if ( !hide_pages( clone_base, page_4kb_size ) )
				return false;

			const auto orig_page = ( reinterpret_cast< std::uintptr_t >( target_process ) >> 12 ) << 12;
			const auto offset = reinterpret_cast< std::uintptr_t >( target_process ) & 0xFFF;

			std::memcpy( reinterpret_cast< void* >( clone_base ), reinterpret_cast< void* >( orig_page ), page_4kb_size );

			auto clone_process = reinterpret_cast< eprocess_t* >( clone_base + offset );

			InitializeListHead( reinterpret_cast< LIST_ENTRY* >( reinterpret_cast< std::uintptr_t >( clone_process ) + 0x5E0 ) );
			InitializeListHead( reinterpret_cast< LIST_ENTRY* >( reinterpret_cast< std::uintptr_t >( clone_process ) + 0x30 ) );

			auto rundown = reinterpret_cast< ex_rundown_ref_t* >( reinterpret_cast< std::uintptr_t >( clone_process ) + 0x6D8 );
			InterlockedExchange64( reinterpret_cast< long long* >( &rundown->m_count ), 0 );
			InterlockedExchangePointer( reinterpret_cast< void* volatile* >( &rundown->m_ptr ), nullptr );

			InterlockedExchangePointer( reinterpret_cast< void* volatile* >( reinterpret_cast< std::uintptr_t >( clone_process ) + 0x680 + 0xE8 ), nullptr );

			InterlockedOr( reinterpret_cast< long* >( reinterpret_cast< std::uintptr_t >( clone_process ) + 0x87C ), 0x10 );
			InterlockedOr( reinterpret_cast< long* >( reinterpret_cast< std::uintptr_t >( target_process ) + 0x87C ), 0x10 );

			_mm_mfence( );

			if ( !nt::mm_initialize_process_address_space( clone_process, target_process ) ) {
				mmu::free_page( clone_base, page_4kb_size );
				return false;
			}

			m_entries[ slot ].m_clone_base = clone_base;
			m_entries[ slot ].m_clone_process = clone_process;
			m_entries[ slot ].m_orig_process = target_process;
			m_entries[ slot ].m_active = true;

			return true;
		}

		bool alloc_page_table( std::uint64_t* out_pa ) {
			auto pt_va = mmu::alloc_page( page_4kb_size );
			if ( !pt_va )
				return false;

			auto pt_pa = phys::virtual_to_physical( pt_va );
			if ( !pt_pa )
				return false;

			if ( !hide_pages( pt_va, page_4kb_size, true ) )
				return false;

			if ( out_pa )
				*out_pa = pt_pa;

			return true;
		}

		bool alloc_pml4e( std::uint32_t pml4_index, pml4e* out_entry ) {
			pml4e pml4_entry{};
			if ( !dpm::read_physical( m_target_cr3 + ( pml4_index * sizeof( pml4e ) ),
				&pml4_entry, sizeof( pml4e ) ) )
				return false;

			if ( !pml4_entry.hard.present ) {
				std::uint64_t pdpt_pa = 0;
				if ( !alloc_page_table( &pdpt_pa ) )
					return false;

				pml4_entry.value = 0;
				pml4_entry.hard.present = 1;
				pml4_entry.hard.read_write = 1;
				pml4_entry.hard.user_supervisor = 1;
				pml4_entry.hard.no_execute = 0;
				pml4_entry.hard.pfn = pdpt_pa >> page_shift;

				if ( !dpm::write_physical( m_target_cr3 + ( pml4_index * sizeof( pml4e ) ),
					&pml4_entry, sizeof( pml4e ) ) )
					return false;
			}

			if ( out_entry )
				*out_entry = pml4_entry;

			return true;
		}

		bool alloc_pdpte( std::uint32_t pml4_index, std::uint32_t pdpt_index, pdpte* out_entry ) {
			pml4e pml4_entry{};
			if ( !alloc_pml4e( pml4_index, &pml4_entry ) )
				return false;

			pdpte pdpt_entry{};
			if ( !dpm::read_physical( ( pml4_entry.hard.pfn << page_shift ) + ( pdpt_index * sizeof( pdpte ) ),
				&pdpt_entry, sizeof( pdpte ) ) )
				return false;

			if ( !pdpt_entry.hard.present ) {
				std::uint64_t pd_pa = 0;
				if ( !alloc_page_table( &pd_pa ) )
					return false;

				pdpt_entry.value = 0;
				pdpt_entry.hard.present = 1;
				pdpt_entry.hard.read_write = 1;
				pdpt_entry.hard.user_supervisor = 1;
				pdpt_entry.hard.no_execute = 0;
				pdpt_entry.hard.pfn = pd_pa >> page_shift;

				if ( !dpm::write_physical( ( pml4_entry.hard.pfn << page_shift ) + ( pdpt_index * sizeof( pdpte ) ),
					&pdpt_entry, sizeof( pdpte ) ) )
					return false;

				tlb::flush_caches( ( pml4_entry.hard.pfn << page_shift ) + ( pdpt_index * sizeof( pdpte ) ) );
			}

			if ( out_entry )
				*out_entry = pdpt_entry;

			return true;
		}

		bool alloc_pde( std::uint32_t pml4_index, std::uint32_t pdpt_index, std::uint32_t pd_index, pde* out_entry ) {
			pdpte pdpt_entry{};
			if ( !alloc_pdpte( pml4_index, pdpt_index, &pdpt_entry ) )
				return false;

			pde pd_entry{};
			if ( !dpm::read_physical( ( pdpt_entry.hard.pfn << page_shift ) + ( pd_index * sizeof( pde ) ),
				&pd_entry, sizeof( pde ) ) )
				return false;

			if ( !pd_entry.hard.present ) {
				std::uint64_t pt_pa = 0;
				if ( !alloc_page_table( &pt_pa ) )
					return false;

				pd_entry.value = 0;
				pd_entry.hard.present = 1;
				pd_entry.hard.read_write = 1;
				pd_entry.hard.user_supervisor = 1;
				pd_entry.hard.no_execute = 0;
				pd_entry.hard.pfn = pt_pa >> page_shift;

				if ( !dpm::write_physical( ( pdpt_entry.hard.pfn << page_shift ) + ( pd_index * sizeof( pde ) ),
					&pd_entry, sizeof( pde ) ) )
					return false;

				tlb::flush_caches( ( pdpt_entry.hard.pfn << page_shift ) + ( pd_index * sizeof( pde ) ) );
			}

			if ( out_entry )
				*out_entry = pd_entry;

			return true;
		}

		bool remap_to_hyperspace( eprocess_t* process, std::uint64_t base_va, std::size_t size) {
			auto idx = find_entry( process );
			if ( idx == -1 )
				return false;

			auto clone_cr3 = process::get_directory_table_base( m_entries[ idx ].m_clone_process );
			if ( !clone_cr3 )
				return false;

			const auto aligned_base = base_va & ~page_4kb_mask;
			const auto aligned_size = ( ( base_va + size ) - aligned_base + page_4kb_mask ) & ~page_4kb_mask;
			const auto page_count = aligned_size >> page_shift;

			for ( auto i = 0ull; i < page_count; i++ ) {
				const auto current_va = aligned_base + ( i << page_shift );
				virt_addr_t va{ current_va };

				pt_entries_t pt_entries;
				if ( !hyperspace_entries( pt_entries, current_va ) ) {
					nt::dbg_print( oxorany( "[hyperspace] Failed to get entries for VA: 0x%llx\n" ), current_va );
					continue;
				}

				if ( !pt_entries.m_pte.hard.present )
					continue;

				if ( pt_entries.m_pdpte.hard.page_size ) {
					nt::dbg_print( oxorany( "[hyperspace] Skipping 1GB large page at VA: 0x%llx\n" ), current_va );
					continue;
				}

				if ( pt_entries.m_pde.hard.page_size ) {
					nt::dbg_print( oxorany( "[hyperspace] Skipping 2MB large page at VA: 0x%llx\n" ), current_va );
					continue;
				}

				std::uint64_t orig_cr3{ };
				attach_to_process( clone_cr3, &orig_cr3 );

				pde clone_pde{ };
				if ( !alloc_pde( va.pml4e_index, va.pdpte_index, va.pde_index, &clone_pde ) ) {
					nt::dbg_print( oxorany( "[hyperspace] Failed to ensure PDE in clone for VA: 0x%llx\n" ), current_va );
					attach_to_process( orig_cr3 );
					continue;
				}

				auto clone_pt_phys = ( clone_pde.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) );
				if ( !dpm::write_physical( clone_pt_phys, &pt_entries.m_pte, sizeof( pte ) ) ) {
					nt::dbg_print( oxorany( "[hyperspace] Failed to write PTE to clone for VA: 0x%llx\n" ), current_va );
					attach_to_process( orig_cr3 );
					continue;
				}

				tlb::flush_caches( current_va );
				attach_to_process( orig_cr3 );
			}

			nt::dbg_print( oxorany( "[hyperspace] created shadow mapping of %llu pages in clone [0x%llx]\n" ),
				page_count, clone_cr3 );
			return true;
		}

		std::uint64_t allocate_in_hyperspace( eprocess_t* process, std::size_t size, bool high_address = false ) {
			auto idx = find_entry( process );
			if ( idx == -1 )
				return 0;

			auto clone_cr3 = process::get_directory_table_base( m_entries[ idx ].m_clone_process );
			if ( !clone_cr3 )
				return 0;

			std::uint64_t orig_cr3{};
			attach_to_process( clone_cr3, &orig_cr3 );

			auto pml4_index = find_non_present_pml4e( high_address );
			if ( pml4_index == static_cast< std::uint32_t >( -1 ) ) {
				attach_to_process( orig_cr3 );
				return 0;
			}

			auto base_va = create_virtual_address( pml4_index, high_address );
			const auto aligned_size = ( size + page_4kb_mask ) & ~page_4kb_mask;
			const auto page_count = aligned_size >> page_shift;

			if ( !map_page_tables( base_va, page_count ) ) {
				attach_to_process( orig_cr3 );
				return 0;
			}

			attach_to_process( orig_cr3 );
			return base_va;
		}

		bool is_thread_in_hyperspace( std::uint32_t tid ) {
			auto thread = nt::ps_lookup_thread_by_tid( tid );
			if ( !thread )
				return false;

			auto apc_process = *reinterpret_cast< eprocess_t** >( reinterpret_cast< std::uint64_t >( thread ) + 0x220 );
			nt::ob_dereference_object( thread );

			return find_by_clone( apc_process ) != -1;
		}

		bool switch_from_hyperspace( std::uint32_t tid, eprocess_t* process ) {
			auto thread = nt::ps_lookup_thread_by_tid( tid );
			if ( !thread )
				return false;

			auto idx = find_entry( process );
			if ( idx == -1 ) {
				nt::ob_dereference_object( thread );
				return false;
			}

			auto apc_ptr = reinterpret_cast< eprocess_t** >( reinterpret_cast< std::uint64_t >( thread ) + 0x220 );
			InterlockedExchangePointer( reinterpret_cast< void** >( apc_ptr ), m_entries[ idx ].m_orig_process );
			nt::ob_dereference_object( thread );
			return true;
		}

		bool switch_to_hyperspace( std::uint32_t tid, eprocess_t* process ) {
			auto thread = nt::ps_lookup_thread_by_tid( tid );
			if ( !thread )
				return false;

			auto idx = find_entry( process );
			if ( idx == -1 ) {
				nt::ob_dereference_object( thread );
				return false;
			}

			auto apc_ptr = reinterpret_cast< eprocess_t** >( reinterpret_cast< std::uint64_t >( thread ) + 0x220 );
			InterlockedExchangePointer( reinterpret_cast< void** >( apc_ptr ), m_entries[ idx ].m_clone_process );
			nt::ob_dereference_object( thread );
			return true;
		}

		bool switch_all_threads_to_hyperspace( eprocess_t* process ) {
			auto idx = find_entry( process );
			if ( idx == -1 )
				return false;

			auto thread_list = reinterpret_cast< list_entry_t* >( reinterpret_cast< std::uint64_t >( process ) + 0x5E0 );
			auto entry = thread_list->m_flink;
			int count = 0;

			while ( entry != thread_list ) {
				auto thread = reinterpret_cast< ethread_t* >( reinterpret_cast< std::uint64_t >( entry ) - 0x6F8 );
				auto apc_ptr = reinterpret_cast< eprocess_t** >( reinterpret_cast< std::uint64_t >( thread ) + 0x220 );
				InterlockedExchangePointer( reinterpret_cast< void** >( apc_ptr ), m_entries[ idx ].m_clone_process );
				count++;
				entry = entry->m_flink;
			}

			return count > 0;
		}

		bool switch_all_threads_from_hyperspace( eprocess_t* process ) {
			auto idx = find_entry( process );
			if ( idx == -1 )
				return false;

			auto thread_list = reinterpret_cast< list_entry_t* >( reinterpret_cast< std::uint64_t >( process ) + 0x5E0 );
			auto entry = thread_list->m_flink;
			int count = 0;

			while ( entry != thread_list ) {
				auto thread = reinterpret_cast< ethread_t* >( reinterpret_cast< std::uint64_t >( entry ) - 0x6F8 );
				auto apc_ptr = reinterpret_cast< eprocess_t** >( reinterpret_cast< std::uint64_t >( thread ) + 0x220 );
				InterlockedExchangePointer( reinterpret_cast< void** >( apc_ptr ), m_entries[ idx ].m_orig_process );
				count++;
				entry = entry->m_flink;
			}

			return count > 0;
		}

		bool create_hyperspace( eprocess_t* process ) {
			if ( find_entry( process ) != -1 )
				return true;

			return clone_eprocess( process );
		}

		void cleanup_hyperspace( eprocess_t* process ) {
			auto idx = find_entry( process );
			if ( idx == -1 )
				return;

			if ( nt::mm_is_address_valid( reinterpret_cast< void* >( m_entries[ idx ].m_clone_base ) ) )
				mmu::free_page( m_entries[ idx ].m_clone_base, page_4kb_size );

			m_entries[ idx ].m_active = false;
			m_entries[ idx ].m_orig_process = nullptr;
			m_entries[ idx ].m_clone_process = nullptr;
			m_entries[ idx ].m_clone_base = 0;
		}
	}
}