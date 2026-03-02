#pragma once

namespace vad {
	std::uint64_t allocate_virtual( eprocess_t* eprocess, std::size_t size, mmu::protection_style protection ) {
		auto vad_root = reinterpret_cast< rtl_avl_tree_t* >( reinterpret_cast< std::uint8_t* >( eprocess ) + 0x7d8 );
		if ( !vad_root ) {
			nt::dbg_print( oxorany( "[vpn] Could not get VAD node\n" ) );
			return {};
		}

		nt::dbg_print( oxorany( "[vpn] vadroot %llx, eproc %llx\n" ), vad_root->m_root, eprocess );

		auto make_vpn = [ ]( std::uint32_t low, std::uint16_t high ) -> std::uint64_t {
			return ( static_cast< std::uint64_t >( high ) << 32 ) | low;
			};
		auto split_vpn = [ ]( std::uint64_t vpn, std::uint32_t& low, std::uint8_t& high ) {
			low = static_cast< std::uint32_t >( vpn & 0xFFFFFFFFull );
			high = static_cast< std::uint8_t >( ( vpn >> 32 ) & 0xFFFFull );
			};
		auto bytes_to_pages = [ ]( std::uint64_t b ) -> std::uint64_t { return ( b + 0xFFF ) >> 12; };

		std::uint64_t allocated_base = 0;
		std::uint64_t cr3_addr = paging::m_target_cr3;

		mmu::traverse_vad_node( eprocess, reinterpret_cast< mmvad_short_t* >( vad_root->m_root ), [ & ]( mmvad_short_t* vad_node ) -> bool {
			const auto start_vpn = make_vpn( vad_node->m_starting_vpn, vad_node->m_starting_vpn_high );
			const auto end_vpn = make_vpn( vad_node->m_ending_vpn, vad_node->m_ending_vpn_high );

			if ( vad_node->u.m_vad_flags.m_protection == protection ) {
				const auto current_pages = end_vpn - start_vpn + 1;
				const auto target_pages = bytes_to_pages( size );
				if ( target_pages <= current_pages ) {
					return true;
				}

				nt::dbg_print( oxorany( "[vpn] page %llx\n" ), target_pages );

				const auto new_end_vpn = start_vpn + target_pages - 1;
				auto original_low = vad_node->m_ending_vpn;
				auto original_high = vad_node->m_ending_vpn_high;
				auto original_end_vpn = end_vpn;

				std::uint32_t low; std::uint8_t high;
				split_vpn( new_end_vpn, low, high );
				vad_node->m_ending_vpn = low;
				vad_node->m_ending_vpn_high = high;

				nt::dbg_print( oxorany( "[vpn] start %llx end %llx -> new end %llx (pages)\n" ),
					start_vpn << 12, end_vpn << 12, new_end_vpn << 12 );

				std::uint64_t pages_to_initialize = new_end_vpn - original_end_vpn;
				allocated_base = start_vpn << 12;

				for ( std::uint64_t page_offset = 1; page_offset <= pages_to_initialize; page_offset++ ) {
					std::uint64_t current_va = ( original_end_vpn + page_offset ) << 12;

					cr3 cr3_struct{ .flags = cr3_addr };
					virt_addr_t va{ .value = current_va };

					pml4e pml4_entry{ };
					auto pml4_addr = reinterpret_cast< pml4e* >( cr3_struct.dirbase << 12 ) + va.pml4e_index;
					paging::dpm::read_physical( reinterpret_cast< std::uint64_t >( pml4_addr ), &pml4_entry, sizeof( pml4e ) );
					nt::dbg_print( oxorany( "[pml4] %d %llx \n" ), pml4_entry.hard.present, pml4_entry.hard.pfn );

					pdpte pdpt_entry{ };
					auto pdpte_addr = reinterpret_cast< pdpte* >( pml4_entry.hard.pfn << 12 ) + va.pdpte_index;
					paging::dpm::read_physical( reinterpret_cast< std::uint64_t >( pdpte_addr ), &pdpt_entry, sizeof( pdpte ) );
					nt::dbg_print( oxorany( "[pdpte] %d %llx \n" ), pdpt_entry.hard.present, pdpt_entry.hard.pfn );

					pde pd_entry{ };
					auto pde_addr = reinterpret_cast< pde* >( pdpt_entry.hard.pfn << 12 ) + va.pde_index;
					paging::dpm::read_physical( reinterpret_cast< std::uint64_t >( pde_addr ), &pd_entry, sizeof( pde ) );
					nt::dbg_print( oxorany( "[pde] %d %llx \n" ), pd_entry.hard.present, pd_entry.hard.pfn );

					physical_address_t lowest_pa = { };
					physical_address_t highest_pa;
					highest_pa.m_quad_part = -1LL;
					physical_address_t skip_bytes = { };

					auto mdl = nt::mm_allocate_pages_for_mdl(
						lowest_pa,
						highest_pa,
						skip_bytes,
						paging::page_4kb_size
					);
					if ( !mdl || mdl->m_byte_count < paging::page_4kb_size ) {
						nt::dbg_print( oxorany( "[-] fail to alloc a mdl\n" ) );
						if ( mdl ) nt::io_free_mdl( mdl );
						return true;
					}

					auto pfn_array = reinterpret_cast< std::uint64_t* >( mdl + 1 );
					physical_address_t phys_addr;
					phys_addr.m_quad_part = pfn_array[ 0 ] << 12;

					nt::dbg_print( oxorany( "[physical page] pa %llx -> va %llx\n" ),
						phys_addr.m_quad_part, current_va );

					pte new_pte{ };
					new_pte.hard.present = 1;
					new_pte.hard.read_write = 1;
					new_pte.hard.no_execute = 0;
					new_pte.hard.user_supervisor = 1;
					new_pte.hard.pfn = phys_addr.m_quad_part >> 12;

					auto pte_addr = reinterpret_cast< pte* >( pd_entry.hard.pfn << 12 );
					const auto sys_va = paging::dpm::map_physical( reinterpret_cast< std::uint64_t >( pte_addr ) );
					reinterpret_cast< pte* >( sys_va )[ va.pte_index ].value = new_pte.value;

					nt::ke_invalidate_all_caches( );
					nt::io_free_mdl( mdl );

					std::uint64_t pa;
					if ( paging::translate_linear( current_va, &pa ) )
						nt::dbg_print( oxorany( "[test] mapping: va %llx -> pa %llx\n" ), current_va, pa );
				}

				nt::dbg_print( oxorany( "[restore] end : %llx\n" ), original_end_vpn << 12 );

				vad_node->m_ending_vpn = original_low;
				vad_node->m_ending_vpn_high = original_high;
				return false;
			}

			return true;
			} );

		return allocated_base;
	}
}