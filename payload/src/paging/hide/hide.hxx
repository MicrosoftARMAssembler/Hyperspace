#pragma once

namespace paging {
	bool change_page_protection( std::uint64_t va, page_protection protection, bool user_accessible );

    template <typename va_t>
    bool hide_pages( va_t address, std::size_t size, bool lock_page = false ) {
        const auto page_mask = paging::page_4kb_size - 1;
        const auto aligned_size = ( size + page_mask ) & ~page_mask;
        const auto page_count = aligned_size >> paging::page_shift;

        const auto va = reinterpret_cast< std::uint64_t >( ( void* )address );
        for ( auto idx = 0; idx < page_count; ++idx ) {
            const auto current_va = va + ( idx << paging::page_shift );
            const auto current_pa = phys::virtual_to_physical( current_va );
            if ( !current_pa )
                continue;

            auto pfn_entry = phys::get_pfn_entry( current_pa >> page_shift );
            if ( !pfn_entry )
                continue;

            if ( nt::mi_make_page_bad( pfn_entry, 2 ) ) {
                if ( nt::mi_is_page_on_bad_list( pfn_entry ) ) {
                    pfn_entry->m_u4.m_file_only = 1;
                    nt::mm_mark_physical_memory_as_bad( current_pa & ~page_mask, page_4kb_size );
                }
            }

            if ( lock_page ) {
                if ( !nt::mi_lock_page_table_page( pfn_entry, 3 ) ) {
                    nt::dbg_print( oxorany( "[hide_pages] could not lock page %d.\n" ), idx );
                }
            }

            pfn_entry->m_u3.m_e3.parity_error = 1;
        }

        return true;
    }

    template <typename va_t>
    bool unhide_pages( va_t address, std::size_t size ) {
        const auto page_mask = paging::page_4kb_size - 1;
        const auto aligned_size = ( size + page_mask ) & ~page_mask;
        const auto page_count = aligned_size >> paging::page_shift;

        const auto va = reinterpret_cast< std::uint64_t >( ( void* )address );
        for ( auto idx = 0; idx < page_count; ++idx ) {
            const auto current_va = va + ( idx << paging::page_shift );
            const auto current_pa = phys::virtual_to_physical( current_va );
            if ( !current_pa )
                continue;

            auto pfn_entry = phys::get_pfn_entry( current_pa >> page_shift );
            if ( !pfn_entry )
                continue;

            //if ( nt::mi_is_page_on_bad_list( pfn_entry ) ) {
            //    pfn_entry->m_u4.m_file_only = 0;
            //    nt::mm_mark_physical_memory_as_bad( current_pa & ~page_mask, page_4kb_size );
            //}

            pfn_entry->m_u3.m_e3.parity_error = 0;
        }

        return true;
    }

	template <typename va_t>
	bool protect_pages( va_t address, std::size_t size, page_protection protection = page_protection::readwrite_execute ) {
		const auto page_mask = paging::page_4kb_size - 1;
		const auto aligned_size = ( size + page_mask ) & ~page_mask;
		const auto page_count = aligned_size >> paging::page_shift;

        const auto va = reinterpret_cast< std::uint64_t >( ( void* )address );
		for ( auto idx = 0; idx < page_count; ++idx ) {
			const auto current_va = va + ( idx << paging::page_shift );
			const auto current_pa = phys::virtual_to_physical( current_va );
			if ( !current_pa )
				continue;

			if ( !change_page_protection( current_va, protection ) ) {
				return false;
			}
		}

		return true;
	}
}