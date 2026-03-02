#pragma once

namespace paging {
	namespace ptm {
		std::uint64_t m_new_pdpt{ 0 };
		std::uint64_t m_new_pd{ 0 };
		std::uint64_t m_new_pt{ 0 };

		std::uint64_t m_new_pdpt_pfn{ 0 };
		std::uint64_t m_new_pd_pfn{ 0 };
		std::uint64_t m_new_pt_pfn{ 0 };

		std::uint16_t m_pml4e_index,
			m_pdpte_index,
			m_pde_index,
			m_pte_index,
			m_page_offset;

		bool initialize( ) {
			cr3 target_cr3{ m_target_cr3 };
			const auto current_pml4 = reinterpret_cast< pml4e* >(
				phys::physical_to_virtual( target_cr3.dirbase << 12 )
				);

			for ( auto idx = 100u; idx < 256u; ++idx ) {
				if ( !current_pml4[ idx ].hard.present ) {
					m_pml4e_index = idx;
					break;
				}
			}

			if ( m_pml4e_index == 0 ) {
				nt::dbg_print( oxorany( "[ptm] initialize: No free PML4 entry found\n" ) );
				return false;
			}

			m_new_pdpt = mmu::alloc_page( page_4kb_size );
			if ( !m_new_pdpt )
				return false;

			if ( !hide_pages( m_new_pdpt, page_4kb_size ) ) {
				return false;
			}

			pt_entries_t pt_entries;
			if ( !hyperspace_entries( pt_entries, m_new_pdpt ) )
				return false;

			m_new_pdpt_pfn = pt_entries.m_pte.hard.pfn << 12;

			pml4e new_pml4e = { 0 };
			new_pml4e.hard.present = true;
			new_pml4e.hard.read_write = true;
			new_pml4e.hard.user_supervisor = true;
			new_pml4e.hard.pfn = pt_entries.m_pte.hard.pfn;

			auto pml4_entry_phys = ( target_cr3.dirbase << 12 ) + ( m_pml4e_index * sizeof( pml4e ) );
			std::memcpy(
				reinterpret_cast< void* >( phys::physical_to_virtual( pml4_entry_phys ) ),
				&new_pml4e,
				sizeof( pml4e )
			);

			m_new_pd = mmu::alloc_page( page_4kb_size );
			if ( !m_new_pd )
				return false;

			if ( !hide_pages( m_new_pd, page_4kb_size ) ) {
				return false;
			}

			if ( !hyperspace_entries( pt_entries, m_new_pd ) )
				return false;

			m_new_pd_pfn = pt_entries.m_pte.hard.pfn << 12;

			m_new_pt = mmu::alloc_page( page_4kb_size );
			if ( !m_new_pt )
				return false;

			if ( !hide_pages( m_new_pt, page_4kb_size ) ) {
				return false;
			}

			if ( !hyperspace_entries( pt_entries, m_new_pt ) )
				return false;

			m_new_pt_pfn = pt_entries.m_pte.hard.pfn << 12;
			return true;
		}

		void* map_page( std::uint64_t physical_address, page_protection protection = page_protection::readwrite, bool supervisor = false ) {
			if ( !physical_address || !phys::is_address_valid( physical_address, page_4kb_size ) )
				return nullptr;

			const auto page_offset = physical_address & page_4kb_mask;
			const auto page_base = physical_address & ~page_4kb_mask;
			const auto pfn = page_base >> 12;

			++m_pte_index;
			if ( m_pte_index > 511 ) {
				++m_pde_index;
				m_pte_index = 0;
			}

			if ( m_pde_index > 511 ) {
				++m_pdpte_index;
				m_pde_index = 0;
			}

			if ( m_pdpte_index > 511 ) {
				return nullptr;
			}

			pdpte new_pdpte = { 0 };
			new_pdpte.hard.present = true;
			new_pdpte.hard.read_write = true;
			new_pdpte.hard.pfn = m_new_pd_pfn >> 12;
			new_pdpte.hard.user_supervisor = supervisor;

			auto pdpte_addr = m_new_pdpt + ( m_pdpte_index * sizeof( pdpte ) );
			*reinterpret_cast< pdpte* >( pdpte_addr ) = new_pdpte;

			pde new_pde = { 0 };
			new_pde.hard.present = true;
			new_pde.hard.read_write = ( protection != page_protection::inaccessible );
			new_pde.hard.pfn = m_new_pt_pfn >> 12;
			new_pde.hard.user_supervisor = supervisor;
			new_pde.hard.no_execute = ( protection != page_protection::readwrite_execute );

			auto pde_addr = m_new_pd + ( m_pde_index * sizeof( pde ) );
			*reinterpret_cast< pde* >( pde_addr ) = new_pde;

			pte new_pte = { 0 };
			new_pte.hard.present = true;
			new_pte.hard.read_write = ( protection != page_protection::inaccessible );
			new_pte.hard.pfn = pfn;
			new_pte.hard.user_supervisor = supervisor;
			new_pte.hard.no_execute = ( protection != page_protection::readwrite_execute );

			auto pte_addr = m_new_pt + ( m_pte_index * sizeof( pte ) );
			*reinterpret_cast< pte* >( pte_addr ) = new_pte;

			virt_addr_t new_addr{ };
			new_addr.pml4e_index = m_pml4e_index;
			new_addr.pdpte_index = m_pdpte_index;
			new_addr.pde_index = m_pde_index;
			new_addr.pte_index = m_pte_index;
			new_addr.offset = page_offset;

			__invlpg( reinterpret_cast< void* >( new_addr.value ) );
			return reinterpret_cast< void* >( new_addr.value );
		}
	}
}