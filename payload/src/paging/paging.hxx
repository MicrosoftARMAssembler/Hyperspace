#pragma once 

namespace paging {
	void attach_to_process( std::uint64_t cr3, std::uint64_t* old_cr3 = nullptr ) {
		if ( old_cr3 )
			*old_cr3 = m_target_cr3;
		m_target_cr3 = cr3;
	}

	bool is_attached_to_process( std::uint64_t cr3 ) {
		return m_target_cr3 == cr3;
	}

	bool hyperspace_entries( pt_entries_t& entries, std::uint64_t addr ) {
		virt_addr_t va{ addr };
		if ( !dpm::read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
								  &entries.m_pml4e, sizeof( pml4e ) ) )
			return false;

		if ( !entries.m_pml4e.hard.present )
			return false;

		if ( !dpm::read_physical( ( entries.m_pml4e.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
								  &entries.m_pdpte, sizeof( pdpte ) ) )
			return false;

		if ( !entries.m_pdpte.hard.present )
			return false;

		if ( entries.m_pdpte.hard.page_size ) {
			return true;
		}

		if ( !dpm::read_physical( ( entries.m_pdpte.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
								  &entries.m_pde, sizeof( pde ) ) )
			return false;

		if ( !entries.m_pde.hard.present )
			return false;

		if ( entries.m_pde.hard.page_size ) {
			return true;
		}

		if ( !dpm::read_physical( ( entries.m_pde.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
								  &entries.m_pte, sizeof( pte ) ) )
			return false;

		if ( !entries.m_pte.hard.present )
			return false;

		return true;
	}

	bool translate_linear( std::uint64_t va, std::uint64_t* pa = nullptr, std::uint32_t* page_size = nullptr ) {
		pt_entries_t pt_entries;
		if ( !hyperspace_entries( pt_entries, va ) )
			return false;

		if ( !pt_entries.m_pml4e.hard.present )
			return false;

		if ( !pt_entries.m_pdpte.hard.present )
			return false;

		if ( pt_entries.m_pdpte.hard.page_size ) {
			if ( page_size ) *page_size = page_1gb_size;
			if ( pa ) *pa = ( pt_entries.m_pdpte.hard.pfn << page_shift ) + ( va & page_1gb_mask );
			return true;
		}

		if ( !pt_entries.m_pde.hard.present )
			return false;

		if ( pt_entries.m_pde.hard.page_size ) {
			if ( page_size ) *page_size = page_2mb_size;
			if ( pa ) *pa = ( pt_entries.m_pde.hard.pfn << page_shift ) + ( va & page_2mb_mask );
			return true;
		}

		if ( !pt_entries.m_pte.hard.present )
			return false;

		if ( page_size ) *page_size = page_4kb_size;
		if ( pa ) *pa = ( pt_entries.m_pte.hard.pfn << page_shift ) + ( va & page_4kb_mask );
		return true;
	}

	std::uint64_t create_virtual_address( std::uint32_t pml4_index, bool use_high_address ) {
		auto additional_offset = static_cast< std::uint64_t >( nt::rand( ) % 512 ) << 21;

		auto get_pml4e = [ &pml4_index ] ( ) {
			return static_cast< std::uint64_t >( pml4_index ) << 39;
			};

		std::uint64_t virtual_address;
		if ( use_high_address ) {
			virtual_address = 0xFFFF000000000000ULL | get_pml4e( ) | additional_offset;
		}
		else {
			virtual_address = get_pml4e( ) | additional_offset;
		}

		return virtual_address;
	}

	std::uint32_t find_non_present_pml4e( bool use_high_address ) {
		std::uint32_t start_idx = 0;
		std::uint32_t end_idx = 0;

		if ( use_high_address ) {
			start_idx = 256;
			end_idx = 512;
		}
		else {
			start_idx = 1;
			end_idx = 256;
		}

		for ( auto idx = start_idx; idx < end_idx; idx++ ) {
			pml4e pml4_entry{};
			if ( !dpm::read_physical( m_target_cr3 + ( idx * sizeof( pml4e ) ), &pml4_entry, sizeof( pml4e ) ) )
				continue;

			if ( !pml4_entry.hard.present ) {
				nt::dbg_print( oxorany( "[paging] Found non-present PML4E at index %d (%s space)\n" ),
							   idx, use_high_address ? oxorany( "kernel" ) : oxorany( "user" ) );
				return idx;
			}
		}

		nt::dbg_print( oxorany( "[paging] No non-present PML4E found in %s space\n" ),
					   use_high_address ? oxorany( "kernel" ) : oxorany( "user" ) );
		return -1;
	}

	bool split_2mb_to_4kb( std::uint64_t address ) {
		_virt_addr_t va{ address };

		pt_entries_t entries;
		if ( !hyperspace_entries( entries, address ) )
			return false;

		if ( !entries.m_pde.hard.present || !entries.m_pde.hard.page_size )
			return false;

		auto new_pte_va = mmu::alloc_page( paging::page_4kb_size );
		if ( !new_pte_va ) {
			return false;
		}

		auto new_pte = reinterpret_cast< pte* >( new_pte_va );
		auto pde_pfn = entries.m_pde.hard.pfn;
		for ( auto idx = 0; idx < 512; idx++ ) {
			new_pte[ idx ].value = 0;
			new_pte[ idx ].hard.present = 1;
			new_pte[ idx ].hard.read_write = 1;
			new_pte[ idx ].hard.user_supervisor = entries.m_pde.hard.user_supervisor;
			new_pte[ idx ].hard.page_write_through = entries.m_pde.hard.page_write_through;
			new_pte[ idx ].hard.cached_disable = entries.m_pde.hard.cached_disable;
			new_pte[ idx ].hard.accessed = 0;
			new_pte[ idx ].hard.dirty = 0;
			new_pte[ idx ].hard.pat = 0;
			new_pte[ idx ].hard.global = entries.m_pde.hard.global;
			new_pte[ idx ].hard.no_execute = 0;
			new_pte[ idx ].hard.pfn = pde_pfn + idx;
		}

		auto new_pt_phys = phys::virtual_to_physical( new_pte_va );
		if ( !new_pt_phys )
			return false;

		pde new_pde;
		new_pde.value = 0;
		new_pde.hard.present = 1;
		new_pde.hard.read_write = 1;
		new_pde.hard.user_supervisor = entries.m_pde.hard.user_supervisor;
		new_pde.hard.page_write_through = entries.m_pde.hard.page_write_through;
		new_pde.hard.cached_disable = entries.m_pde.hard.cached_disable;
		new_pde.hard.accessed = 0;
		new_pde.hard.page_size = 0;
		new_pde.hard.no_execute = 0;
		new_pde.hard.pfn = new_pt_phys >> 12;

		pml4e pml4_entry;
		if ( !dpm::read_physical( m_system_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
								  &pml4_entry, sizeof( pml4e ) ) )
			return false;

		pdpte pdpt_entry;
		if ( !dpm::read_physical( ( pml4_entry.hard.pfn << 12 ) + ( va.pdpte_index * sizeof( pdpte ) ),
								  &pdpt_entry, sizeof( pdpte ) ) )
			return false;

		if ( !dpm::write_physical( ( pdpt_entry.hard.pfn << 12 ) + ( va.pde_index * sizeof( pde ) ),
								   &new_pde, sizeof( pde ) ) )
			return false;

		tlb::flush_caches( address );
		return true;
	}

	bool split_1gb_to_4kb( std::uint64_t address ) {
		_virt_addr_t va{ address };

		pt_entries_t entries;
		if ( !hyperspace_entries( entries, address ) )
			return false;

		if ( !entries.m_pdpte.hard.present || !entries.m_pdpte.hard.page_size )
			return false;

		auto new_pd_va = mmu::alloc_page( paging::page_4kb_size );
		if ( !new_pd_va )
			return false;

		auto new_pd = reinterpret_cast< pde* >( new_pd_va );
		auto pdpte_pfn = entries.m_pdpte.hard.pfn;
		for ( auto pd_idx = 0; pd_idx < 512; pd_idx++ ) {
			auto new_pt_va = mmu::alloc_page( paging::page_4kb_size );
			if ( !new_pt_va )
				return false;

			auto new_pt = reinterpret_cast< pte* >( new_pt_va );
			auto pd_pfn = pdpte_pfn + ( pd_idx * 512 );
			for ( auto pt_idx = 0; pt_idx < 512; pt_idx++ ) {
				new_pt[ pt_idx ].hard.present = 1;
				new_pt[ pt_idx ].hard.read_write = 1;
				new_pt[ pt_idx ].hard.user_supervisor = entries.m_pdpte.hard.user_supervisor;
				new_pt[ pt_idx ].hard.page_write_through = entries.m_pdpte.hard.page_write_through;
				new_pt[ pt_idx ].hard.cached_disable = entries.m_pdpte.hard.cached_disable;
				new_pt[ pt_idx ].hard.no_execute = 0;
				new_pt[ pt_idx ].hard.accessed = 0;
				new_pt[ pt_idx ].hard.dirty = 0;
				new_pt[ pt_idx ].hard.pat = 0;
				new_pt[ pt_idx ].hard.pfn = pd_pfn + pt_idx;
			}

			auto new_pt_phys = phys::virtual_to_physical( new_pt_va );
			if ( !new_pt_phys )
				return false;

			new_pd[ pd_idx ].hard.present = 1;
			new_pd[ pd_idx ].hard.read_write = 1;
			new_pd[ pd_idx ].hard.user_supervisor = entries.m_pdpte.hard.user_supervisor;
			new_pd[ pd_idx ].hard.page_write_through = entries.m_pdpte.hard.page_write_through;
			new_pd[ pd_idx ].hard.cached_disable = entries.m_pdpte.hard.cached_disable;
			new_pd[ pd_idx ].hard.accessed = 0;
			new_pd[ pd_idx ].hard.page_size = 0;
			new_pd[ pd_idx ].hard.no_execute = 0;
			new_pd[ pd_idx ].hard.pfn = new_pt_phys >> 12;
		}

		auto new_pd_phys = phys::virtual_to_physical( new_pd_va );
		if ( !new_pd_phys )
			return false;

		pdpte new_pdpte{ };
		new_pdpte.hard.present = 1;
		new_pdpte.hard.read_write = 1;
		new_pdpte.hard.user_supervisor = entries.m_pdpte.hard.user_supervisor;
		new_pdpte.hard.page_write_through = entries.m_pdpte.hard.page_write_through;
		new_pdpte.hard.cached_disable = entries.m_pdpte.hard.cached_disable;
		new_pdpte.hard.no_execute = 0;
		new_pdpte.hard.accessed = 0;
		new_pdpte.hard.page_size = 0;
		new_pdpte.hard.pfn = new_pd_phys >> 12;

		pml4e pml4_entry;
		if ( !dpm::read_physical( m_system_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
								  &pml4_entry, sizeof( pml4e ) ) )
			return false;

		if ( !dpm::write_physical( ( pml4_entry.hard.pfn << 12 ) + ( va.pdpte_index * sizeof( pdpte ) ),
								   &new_pdpte, sizeof( pdpte ) ) )
			return false;

		tlb::flush_caches( address );
		return true;
	}

	bool map_page_tables( std::uint64_t base_va, std::size_t size ) {
		const auto page_mask = page_4kb_size - 1;
		const auto aligned_size = ( size + page_mask ) & ~page_mask;
		if ( !aligned_size ) {
			return false;
		}

		const auto page_size = page_4kb_size;
		for ( std::uint64_t current_va = base_va; current_va < base_va + aligned_size; current_va += page_size ) {
			virt_addr_t va{ current_va };

			pml4e pml4_entry{ };
			if ( !dpm::read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
									  &pml4_entry, sizeof( pml4e ) ) )
				continue;

			if ( !pml4_entry.hard.present ) {
				auto pdpt_va = mmu::alloc_page( page_4kb_size );
				if ( !pdpt_va ) {
					continue;
				}

				auto pdpt_pa = phys::virtual_to_physical( pdpt_va );
				if ( !pdpt_pa ) {
					continue;
				}

				if ( !hide_pages( pdpt_va, page_4kb_size, true ) ) {
					return false;
				}

				pml4e new_pml4e{ 0 };
				new_pml4e.hard.present = 1;
				new_pml4e.hard.read_write = 1;
				new_pml4e.hard.user_supervisor = 1;
				new_pml4e.hard.no_execute = 0;
				new_pml4e.hard.pfn = pdpt_pa >> page_shift;

				auto pml4_phys = m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) );
				if ( !dpm::write_physical( pml4_phys, &new_pml4e, sizeof( pml4e ) ) ) {
					continue;
				}

				tlb::flush_caches( pdpt_va );
				pml4_entry = new_pml4e;
			}

			pdpte pdpt_entry{ };
			if ( !dpm::read_physical( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
									  &pdpt_entry, sizeof( pdpte ) ) )
				continue;

			if ( !pdpt_entry.hard.present ) {
				auto pd_va = mmu::alloc_page( page_4kb_size );
				if ( !pd_va ) {
					continue;
				}

				auto pd_pa = phys::virtual_to_physical( pd_va );
				if ( !pd_pa ) {
					continue;
				}

				if ( !hide_pages( pd_va, page_4kb_size, true ) ) {
					return false;
				}

				pdpte new_pdpte{ 0 };
				new_pdpte.hard.present = 1;
				new_pdpte.hard.read_write = 1;
				new_pdpte.hard.user_supervisor = 1;
				new_pdpte.hard.no_execute = 0;
				new_pdpte.hard.pfn = pd_pa >> page_shift;

				auto pdpt_phys = ( pml4_entry.hard.pfn << 12 ) + ( va.pdpte_index * sizeof( pdpte ) );
				if ( !dpm::write_physical( pdpt_phys, &new_pdpte, sizeof( pdpte ) ) ) {
					continue;
				}

				tlb::flush_caches( pd_va );
				pdpt_entry = new_pdpte;
			}

			pde pd_entry{ };
			if ( !dpm::read_physical( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
									  &pd_entry, sizeof( pde ) ) )
				continue;

			if ( !pd_entry.hard.present ) {
				auto pt_va = mmu::alloc_page( page_4kb_size );
				if ( !pt_va ) {
					continue;
				}

				auto pt_pa = phys::virtual_to_physical( pt_va );
				if ( !pt_pa ) {
					continue;
				}

				if ( !hide_pages( pt_va, page_4kb_size, true ) ) {
					return false;
				}

				pde new_pde{ 0 };
				new_pde.hard.present = 1;
				new_pde.hard.read_write = 1;
				new_pde.hard.user_supervisor = 1;
				new_pde.hard.no_execute = 0;
				new_pde.hard.pfn = pt_pa >> page_shift;

				auto pd_phys = ( pdpt_entry.hard.pfn << 12 ) + ( va.pde_index * sizeof( pde ) );
				if ( !dpm::write_physical( pd_phys, &new_pde, sizeof( pde ) ) ) {
					continue;
				}

				tlb::flush_caches( pt_va );
				pd_entry = new_pde;
			}

			pte pt_entry{ };
			if ( !dpm::read_physical( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
									  &pt_entry, sizeof( pte ) ) )
				continue;

			auto page_va = mmu::alloc_page( page_4kb_size );
			if ( !page_va ) {
				continue;
			}

			auto page_pa = phys::virtual_to_physical( page_va );
			if ( !page_pa ) {
				continue;
			}

			if ( !hide_pages( page_va, page_4kb_size, true ) ) {
				return false;
			}

			pte new_pte{ 0 };
			new_pte.hard.present = 1;
			new_pte.hard.read_write = 1;
			new_pte.hard.user_supervisor = 1;
			new_pte.hard.no_execute = 0;
			new_pte.hard.pfn = page_pa >> page_shift;

			auto pt_phys = ( pd_entry.hard.pfn << 12 ) + ( va.pte_index * sizeof( pte ) );
			if ( !dpm::write_physical( pt_phys, &new_pte, sizeof( pte ) ) ) {
				continue;
			}

			tlb::flush_caches( page_va );
		}

		return true;
	}

	bool map_pte_entry( std::uint64_t base_va, std::size_t size ) {
		const auto page_mask = page_4kb_size - 1;
		const auto aligned_size = ( size + page_mask ) & ~page_mask;
		if ( !aligned_size ) {
			return false;
		}

		const auto page_size = page_4kb_size;
		for ( std::uint64_t current_va = base_va; current_va < base_va + aligned_size; current_va += page_size ) {
			virt_addr_t va{ current_va };

			pml4e pml4_entry{ };
			if ( !dpm::read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
									  &pml4_entry, sizeof( pml4e ) ) )
				continue;

			pdpte pdpt_entry{ };
			if ( !dpm::read_physical( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
									  &pdpt_entry, sizeof( pdpte ) ) )
				continue;

			pde pd_entry{ };
			if ( !dpm::read_physical( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
									  &pd_entry, sizeof( pde ) ) )
				continue;

			pte pt_entry{ };
			if ( !dpm::read_physical( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
									  &pt_entry, sizeof( pte ) ) )
				continue;

			auto page_va = mmu::alloc_page( page_4kb_size );
			if ( !page_va ) {
				continue;
			}

			auto page_pa = phys::virtual_to_physical( page_va );
			if ( !page_pa ) {
				continue;
			}

			if ( !hide_pages( page_va, page_4kb_size, true ) ) {
				return false;
			}

			pte new_pte{ 0 };
			new_pte.hard.present = 1;
			new_pte.hard.read_write = 1;
			new_pte.hard.user_supervisor = 1;
			new_pte.hard.no_execute = 0;
			new_pte.hard.pfn = page_pa >> page_shift;

			auto pt_phys = ( pd_entry.hard.pfn << 12 ) + ( va.pte_index * sizeof( pte ) );
			if ( !dpm::write_physical( pt_phys, &new_pte, sizeof( pte ) ) ) {
				continue;
			}

			tlb::flush_caches( page_va );
		}

		return true;
	}

	bool spoof_pte_range( std::uint64_t address, std::size_t size, bool execute_disable = false ) {
		const auto page_mask = paging::page_4kb_size - 1;
		const auto aligned_size = ( size + page_mask ) & ~page_mask;

		const auto page_count = aligned_size >> paging::page_shift;
		for ( auto idx = 0; idx < page_count; ++idx ) {
			const auto current_va = address + ( idx << paging::page_shift );
			virt_addr_t va{ current_va };

			pml4e pml4_entry{ };
			if ( !dpm::read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
									  &pml4_entry, sizeof( pml4e ) ) )
				continue;

			if ( !pml4_entry.hard.present )
				return false;

			pdpte pdpt_entry{ };
			if ( !dpm::read_physical( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
									  &pdpt_entry, sizeof( pdpte ) ) )
				continue;

			if ( !pdpt_entry.hard.present )
				return false;

			if ( pdpt_entry.hard.page_size ) {
				pdpt_entry.hard.user_supervisor = 1;
				pdpt_entry.hard.no_execute = execute_disable ? 1 : 0;

				auto pdpt_phys = ( pml4_entry.hard.pfn << 12 ) + ( va.pdpte_index * sizeof( pdpte ) );
				if ( !dpm::write_physical( pdpt_phys, &pdpt_entry, sizeof( pdpte ) ) ) {
					return false;
				}

				tlb::flush_caches( current_va );
				continue;
			}

			pde pd_entry{ };
			if ( !dpm::read_physical( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
									  &pd_entry, sizeof( pde ) ) )
				return false;

			if ( !pd_entry.hard.present )
				return false;

			if ( pd_entry.hard.page_size ) {
				pdpt_entry.hard.user_supervisor = 1;
				pd_entry.hard.no_execute = execute_disable ? 1 : 0;

				auto pd_phys = ( pdpt_entry.hard.pfn << paging::page_shift ) + ( va.pde_index * sizeof( pde ) );
				if ( !dpm::write_physical( pd_phys, &pd_entry, sizeof( pde ) ) ) {
					return false;
				}

				tlb::flush_caches( current_va );
				continue;
			}

			pte pt_entry{ };
			if ( !dpm::read_physical( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
									  &pt_entry, sizeof( pte ) ) )
				return false;

			pt_entry.hard.user_supervisor = 1;
			pt_entry.hard.no_execute = execute_disable ? 1 : 0;

			auto pt_phys = ( pd_entry.hard.pfn << paging::page_shift ) + ( va.pte_index * sizeof( pte ) );
			if ( !dpm::write_physical( pt_phys, &pt_entry, sizeof( pte ) ) ) {
				return false;
			}

			tlb::flush_caches( current_va );
		}

		return true;
	}

	std::uint64_t remap_physical_page( std::uint64_t new_cr3, std::uint64_t base_va, bool high_address = false ) {
		virt_addr_t va{ base_va };

		pml4e pml4_entry{ };
		if ( !dpm::read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
								  &pml4_entry, sizeof( pml4e ) ) )
			return 0;

		std::uint64_t orig_cr3;
		attach_to_process( new_cr3, &orig_cr3 );

		auto pml4_index = find_non_present_pml4e( high_address );
		if ( pml4_index == -1 ) {
			attach_to_process( orig_cr3 );
			return 0;
		}

		if ( !dpm::write_physical( m_target_cr3 + ( pml4_index * sizeof( pml4e ) ),
								   &pml4_entry, sizeof( pml4e ) ) ) {
			attach_to_process( orig_cr3 );
			return 0;
		}

		attach_to_process( orig_cr3 );

		auto new_va = va;
		new_va.pml4e_index = pml4_index;
		tlb::flush_caches( new_va.value );
		return new_va.value;
	}

	bool change_page_protection( std::uint64_t va, page_protection protection, bool user_accessible = false ) {
		pt_entries_t entries;
		if ( !hyperspace_entries( entries, va ) )
			return false;

		virt_addr_t virt_addr{ va };

		if ( entries.m_pdpte.hard.page_size ) {
			if ( !entries.m_pdpte.hard.present )
				return false;

			pdpte pdpt_entry = entries.m_pdpte;
			pdpt_entry.hard.user_supervisor = user_accessible ? 1 : 0;

			switch ( protection ) {
				case page_protection::readwrite_execute:
					pdpt_entry.hard.read_write = 1;
					pdpt_entry.hard.no_execute = 0;
					break;
				case page_protection::readwrite:
					pdpt_entry.hard.read_write = 1;
					pdpt_entry.hard.no_execute = 1;
					break;
				case page_protection::read_execute:
					pdpt_entry.hard.read_write = 0;
					pdpt_entry.hard.no_execute = 0;
					break;
				case page_protection::readonly:
					pdpt_entry.hard.read_write = 0;
					pdpt_entry.hard.no_execute = 1;
					break;
				case page_protection::inaccessible:
					pdpt_entry.hard.present = 0;
					break;
			}

			auto pdpt_phys = ( entries.m_pml4e.hard.pfn << page_shift ) + ( virt_addr.pdpte_index * sizeof( pdpte ) );
			if ( !dpm::write_physical( pdpt_phys, &pdpt_entry, sizeof( pdpte ) ) )
				return false;

			tlb::flush_caches( va );
			return true;
		}

		if ( entries.m_pde.hard.page_size ) {
			if ( !entries.m_pde.hard.present )
				return false;

			pde pd_entry = entries.m_pde;
			pd_entry.hard.user_supervisor = user_accessible ? 1 : 0;

			switch ( protection ) {
				case page_protection::readwrite_execute:
					pd_entry.hard.read_write = 1;
					pd_entry.hard.no_execute = 0;
					break;
				case page_protection::readwrite:
					pd_entry.hard.read_write = 1;
					pd_entry.hard.no_execute = 1;
					break;
				case page_protection::read_execute:
					pd_entry.hard.read_write = 0;
					pd_entry.hard.no_execute = 0;
					break;
				case page_protection::readonly:
					pd_entry.hard.read_write = 0;
					pd_entry.hard.no_execute = 1;
					break;
				case page_protection::inaccessible:
					pd_entry.hard.present = 0;
					break;
			}

			auto pd_phys = ( entries.m_pdpte.hard.pfn << page_shift ) + ( virt_addr.pde_index * sizeof( pde ) );
			if ( !dpm::write_physical( pd_phys, &pd_entry, sizeof( pde ) ) )
				return false;

			tlb::flush_caches( va );
			return true;
		}

		if ( !entries.m_pte.hard.present )
			return false;

		pte pt_entry = entries.m_pte;
		pt_entry.hard.user_supervisor = user_accessible ? 1 : 0;

		switch ( protection ) {
			case page_protection::readwrite_execute:
				pt_entry.hard.read_write = 1;
				pt_entry.hard.no_execute = 0;
				break;
			case page_protection::readwrite:
				pt_entry.hard.read_write = 1;
				pt_entry.hard.no_execute = 1;
				break;
			case page_protection::read_execute:
				pt_entry.hard.read_write = 0;
				pt_entry.hard.no_execute = 0;
				break;
			case page_protection::readonly:
				pt_entry.hard.read_write = 0;
				pt_entry.hard.no_execute = 1;
				break;
			case page_protection::inaccessible:
				pt_entry.hard.present = 0;
				break;
		}

		auto pt_phys = ( entries.m_pde.hard.pfn << page_shift ) + ( virt_addr.pte_index * sizeof( pte ) );
		if ( !dpm::write_physical( pt_phys, &pt_entry, sizeof( pte ) ) )
			return false;

		tlb::flush_caches( va );
		return true;
	}

	std::uint64_t get_pte_address( std::uint64_t virtual_address ) {
		_virt_addr_t va{ virtual_address };

		pt_entries_t pt_entries;
		if ( !hyperspace_entries( pt_entries, virtual_address ) )
			return 0;

		if ( pt_entries.m_pdpte.hard.page_size ) {
			nt::dbg_print( oxorany( "[paging] 1GB Large Pages are not supported!\n" ) );
			return 0;
		}

		if ( pt_entries.m_pde.hard.page_size ) {
			nt::dbg_print( oxorany( "[paging] 2MB Large Pages are not supported!\n" ) );
			return 0;
		}

		auto pte_address = phys::physical_to_virtual(
			( pt_entries.m_pde.hard.pfn << 12 ) + ( va.pte_index * sizeof( pte ) ) );
		if ( !pte_address )
			return 0;

		return pte_address;
	}

	bool get_process_cr3( std::uint64_t base_address, std::uint64_t* process_cr3 ) {
		virt_addr_t va{ base_address };
		if ( !base_address )
			return false;

		const auto ranges = nt::mm_get_physical_memory_ranges( );
		for ( auto i = 0; ; i++ ) {
			auto memory_range = ranges[ i ];
			if ( !memory_range.m_base_page.m_quad_part ||
				 !memory_range.m_page_count.m_quad_part )
				break;

			auto current_pa = memory_range.m_base_page.m_quad_part;
			for ( auto current_page = 0;
				  current_page < memory_range.m_page_count.m_quad_part;
				  current_page += page_4kb_size, current_pa += page_4kb_size ) {
				cr3 current_dtb{ .dirbase = current_pa >> page_shift };
				if ( !current_dtb.flags )
					continue;

				auto pfn_entry = phys::get_pfn_entry( current_dtb.dirbase );
				if ( !pfn_entry ||
					 pfn_entry->m_u4.m_pte_frame != current_dtb.dirbase )
					continue;

				pml4e pml4_entry{};
				if ( !dpm::read_physical( current_pa + ( va.pml4e_index * sizeof( pml4e ) ),
										  &pml4_entry, sizeof( pml4e ) ) )
					continue;

				if ( !pml4_entry.hard.present )
					continue;

				pdpte pdpt_entry{};
				if ( !dpm::read_physical( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
										  &pdpt_entry, sizeof( pdpte ) ) )
					continue;

				if ( !pdpt_entry.hard.present )
					continue;

				pde pd_entry{};
				if ( !dpm::read_physical( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
										  &pd_entry, sizeof( pde ) ) )
					continue;

				if ( !pd_entry.hard.present )
					continue;

				pte pt_entry{};
				if ( !dpm::read_physical( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
										  &pt_entry, sizeof( pte ) ) )
					continue;

				if ( !pt_entry.hard.present )
					continue;

				if ( process_cr3 )
					*process_cr3 = current_dtb.flags;

				nt::dbg_print( oxorany( "[paging] get_process_cr3: Found Process CR3 at: 0x%llx\n" ), current_dtb.flags );
				return true;
			}
		}

		nt::dbg_print( oxorany( "[paging] get_process_cr3: Could not find CR3\n" ) );
		return false;
	}
}