#pragma once

namespace paging {
	class c_hyperspace {
	public:
		c_hyperspace( ) { }
		~c_hyperspace( ) { cleanup( ); }

		eprocess_t* m_orig_process = nullptr;
		eprocess_t* m_clone_process = nullptr;
		std::uint64_t m_clone_base = 0;

		bool create( eprocess_t* target_process ) {
			if ( !target_process ) {
				logging::print( oxorany( "[c_hyperspace] Invalid target process" ) );
				return false;
			}

			auto clone_base = reinterpret_cast< std::uint64_t >(
				nt::mm_allocate_independent_pages( page_4kb_size )
				);

			if ( !clone_base ) {
				logging::print( oxorany( "[hyperspace] Could not allocate clone page" ) );
				return false;
			}

			const auto orig_page = ( reinterpret_cast< std::uintptr_t >( target_process ) >> 12 ) << 12;
			const auto offset = reinterpret_cast< std::uintptr_t >( target_process ) & 0xFFF;

			nt::memcpy( reinterpret_cast< void* >( clone_base ), reinterpret_cast< void* >( orig_page ), page_4kb_size );

			auto clone_process = reinterpret_cast< eprocess_t* >( clone_base + offset );
			nt::initialize_list_head( reinterpret_cast< void* >( reinterpret_cast< std::uint64_t >( clone_process ) + 0x5E0 ) );
			nt::initialize_list_head( reinterpret_cast< void* >( reinterpret_cast< std::uint64_t >( clone_process ) + 0x30 ) );

			nt::interlocked_exchange64( reinterpret_cast< void* >( reinterpret_cast< std::uint64_t >( clone_process ) + 0x6D8 ), 0 );
			nt::interlocked_exchange_pointer( reinterpret_cast< void** >( reinterpret_cast< std::uint64_t >( clone_process ) + 0x6E0 ), nullptr );
			nt::interlocked_exchange_pointer( reinterpret_cast< void** >( reinterpret_cast< std::uint64_t >( clone_process ) + 0x768 ), nullptr );

			auto current = g_dpm->read_physical<std::uint32_t>(
				phys::virtual_to_physical( reinterpret_cast< std::uint64_t >( clone_process ) + 0x87C ) );
			g_dpm->write_physical<std::uint64_t>(
				phys::virtual_to_physical( reinterpret_cast< std::uint64_t >( clone_process ) + 0x87C ),
				current | 0x10 );

			if ( !nt::mm_initialize_process_address_space( clone_process, target_process ) ) {
				logging::print( oxorany( "[hyperspace] Failed to initialize address space" ) );
				return false;
			}

			this->m_clone_base = clone_base;
			this->m_clone_process = clone_process;
			this->m_orig_process = target_process;

			logging::print( oxorany( "[c_hyperspace] Created hyperspace for process 0x%llx" ), target_process );
			return true;
		}

		std::uint64_t allocate_virtual( std::size_t size, bool high_address = false ) {
			auto hyperspace_cr3 = this->get_directory_table_base( );
			if ( !hyperspace_cr3 ) {
				logging::print( oxorany( "[c_hyperspace] allocate_virtual: Could not get hyperspace CR3" ) );
				return 0;
			}

			std::uint64_t orig_cr3{ };
			g_paging->swap_context( hyperspace_cr3, &orig_cr3 );

			auto pml4_index = g_paging->find_non_present_pml4e( high_address );
			if ( pml4_index == static_cast< std::uint32_t >( -1 ) ) {
				logging::print( oxorany( "[c_hyperspace] allocate_virtual: Could not find non-present PML4E" ) );
				g_paging->swap_context( orig_cr3 );
				return 0;
			}

			auto base_va = paging::create_virtual_address( pml4_index, high_address );
			const auto page_mask = page_4kb_size - 1;
			const auto aligned_size = ( size + page_mask ) & ~page_mask;
			const auto page_count = aligned_size >> 12;

			if ( !g_paging->map_page_tables( base_va, page_count ) ) {
				logging::print( oxorany( "[c_hyperspace] allocate_virtual: Could not map page tables" ) );
				g_paging->swap_context( hyperspace_cr3 );
				return 0;
			}

			logging::print( oxorany( "[c_hyperspace] Allocated VA: 0x%llx (size: %i)\n" ), base_va, aligned_size );

			g_paging->swap_context( hyperspace_cr3 );
			return base_va;
		}
		
		std::uint64_t get_directory_table_base( ) const {
			return process::get_directory_table_base( m_clone_process );
		}

		bool is_thread_in_hyperspace( std::uint32_t tid ) {
			auto thread = nt::ps_lookup_thread_by_tid( tid );
			if ( !thread )
				return false;

			eprocess_t* apc_process = nullptr;
			auto apc_process_addr = reinterpret_cast< std::uint64_t >( thread ) + 0x220;

			nt::memcpy( &apc_process, reinterpret_cast< void* >( apc_process_addr ), sizeof( eprocess_t* ) );
			nt::ob_dereference_object( thread );

			return apc_process == m_clone_process;
		}

		bool switch_thread_to_hyperspace( std::uint32_t tid ) {
			return swap_thread_process( tid, m_clone_process, true );
		}

		bool switch_thread_from_hyperspace( std::uint32_t tid ) {
			return swap_thread_process( tid, m_orig_process, false );
		}

		bool switch_all_threads_to_hyperspace( ) {
			if ( !m_orig_process || !m_clone_process ) {
				logging::print( oxorany( "[c_hyperspace] Invalid process state" ) );
				return false;
			}

			list_entry_t thread_list_head{};
			auto thread_list_addr = reinterpret_cast< std::uint64_t >( m_orig_process ) + 0x5E0;

			nt::memcpy( &thread_list_head, reinterpret_cast< void* >( thread_list_addr ), sizeof( list_entry_t ) );

			auto entry_addr = reinterpret_cast< std::uint64_t >( thread_list_head.m_flink );
			int count = 0;

			while ( entry_addr != thread_list_addr ) {
				auto thread_addr = entry_addr - 0x6F8;
				auto apc_ptr_addr = thread_addr + 0x220;

				nt::memcpy( reinterpret_cast< void* >( apc_ptr_addr ), &m_clone_process, sizeof( eprocess_t* ) );

				count++;

				list_entry_t current_entry{};
				nt::memcpy( &current_entry, reinterpret_cast< void* >( entry_addr ), sizeof( list_entry_t ) );
				entry_addr = reinterpret_cast< std::uint64_t >( current_entry.m_flink );
			}

			logging::print( oxorany( "[c_hyperspace] Switched %d threads to hyperspace" ), count );
			return count > 0;
		}

		bool switch_all_threads_from_hyperspace( ) {
			if ( !m_orig_process || !m_clone_process ) {
				logging::print( oxorany( "[c_hyperspace] Invalid process state" ) );
				return false;
			}

			list_entry_t thread_list_head{};
			auto thread_list_addr = reinterpret_cast< std::uint64_t >( m_orig_process ) + 0x5E0;

			nt::memcpy( &thread_list_head, reinterpret_cast< void* >( thread_list_addr ), sizeof( list_entry_t ) );

			auto entry_addr = reinterpret_cast< std::uint64_t >( thread_list_head.m_flink );
			int count = 0;

			while ( entry_addr != thread_list_addr ) {
				auto thread_addr = entry_addr - 0x6F8;
				auto apc_ptr_addr = thread_addr + 0x220;

				nt::memcpy( reinterpret_cast< void* >( apc_ptr_addr ), &m_orig_process, sizeof( eprocess_t* ) );

				count++;

				list_entry_t current_entry{};
				nt::memcpy( &current_entry, reinterpret_cast< void* >( entry_addr ), sizeof( list_entry_t ) );
				entry_addr = reinterpret_cast< std::uint64_t >( current_entry.m_flink );
			}

			logging::print( oxorany( "[c_hyperspace] Switched %d threads from hyperspace" ), count );
			return count > 0;
		}

		void cleanup( ) {
			if ( m_clone_base ) {
				logging::print( oxorany( "[c_hyperspace] Cleaning up hyperspace for process 0x%llx" ), m_orig_process );
				//nt::mm_free_independent_pages( m_clone_base, page_4kb_size );
			}

			//clear_clone_vad( m_orig_process );
			clear_clone_vad( m_clone_process );

			m_orig_process = nullptr;
			m_clone_process = nullptr;
			m_clone_base = 0;
			std::getchar( );
		}

	private:
		bool swap_thread_process( std::uint32_t tid, eprocess_t* target_process, bool to_hyperspace ) {
			if ( !target_process ) {
				logging::print( oxorany( "[c_hyperspace] Invalid target process for swap" ) );
				return false;
			}

			auto thread = nt::ps_lookup_thread_by_tid( tid );
			if ( !thread ) {
				logging::print( oxorany( "[c_hyperspace] Failed to lookup thread %u" ), tid );
				return false;
			}

			auto apc_ptr_addr = reinterpret_cast< std::uint64_t >( thread ) + 0x220;
			nt::memcpy( reinterpret_cast< void* >( apc_ptr_addr ), &target_process, sizeof( eprocess_t* ) );

			nt::ob_dereference_object( thread );

			logging::print( oxorany( "[c_hyperspace] Swapped thread %u %s hyperspace" ),
				tid, to_hyperspace ? "to" : "from" );

			return true;
		}

		bool clear_clone_vad( eprocess_t* process ) {
			if ( !process || !nt::is_address_valid( process ) ) {
				return false;
			}

			auto clone_root_offset = reinterpret_cast< std::uint64_t >( process ) + 0x4F0;
			void* clone_root = nullptr;
			nt::memcpy( &clone_root, reinterpret_cast< void* >( clone_root_offset ), sizeof( void* ) );

			if ( clone_root && nt::is_address_valid( clone_root ) ) {
				nt::mi_delete_inserted_clone_vads( process );
				nt::mi_delete_vad_bitmap( process );
				//nt::mi_lock_down_working_set( process, 0 );
				//nt::mi_clone_discard_vad_commit( clone_root );
				return true;
			}

			return false;
		}
	};
}