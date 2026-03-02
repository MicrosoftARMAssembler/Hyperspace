#pragma once

namespace phys {
    std::uint64_t mm_lowest_physical_page { };
    std::uint64_t mm_highest_physical_page { };
    std::uint64_t m_physical_mask { };

    bool init_ranges( ) {
        auto physical_ranges = nt::mm_get_physical_memory_ranges( );
        if ( !physical_ranges ) {
            return false;
        }

        auto lowest_pfn = MAXULONG_PTR;
        auto highest_pfn = static_cast< std::uint64_t >( 0 );
        for ( auto i = 0ul; physical_ranges[ i ].m_base_page.m_quad_part != 0 ||
            physical_ranges[ i ].m_page_count.m_quad_part != 0;
            i++ ) {
            auto base_address = physical_ranges[ i ].m_base_page;
            auto number_of_bytes = physical_ranges[ i ].m_page_count;
            if ( number_of_bytes.m_quad_part == 0 ) {
                continue;
            }

            auto start_pfn = static_cast< std::uint64_t >( base_address.m_quad_part >> 12 );
            auto end_pfn =
                static_cast< std::uint64_t >( ( base_address.m_quad_part + number_of_bytes.m_quad_part - 1 ) >> 12 );

            if ( start_pfn < lowest_pfn ) {
                lowest_pfn = start_pfn;
            }

            if ( end_pfn > highest_pfn ) {
                highest_pfn = end_pfn;
            }
        }

        mm_lowest_physical_page = lowest_pfn;
        mm_highest_physical_page = highest_pfn;
        return true;
    }

    void init_mask( ) {
        int regs[ 4 ]{};
        __cpuid( regs, 0x80000000 );
        unsigned max_ext = static_cast< unsigned >( regs[ 0 ] );

        unsigned phys_bits = 36;
        if ( max_ext >= 0x80000008 ) {
            __cpuid( regs, 0x80000008 );
            unsigned b = regs[ 0 ] & 0xFF;
            if ( b >= 32 && b <= 52 )
                phys_bits = b;
        }

        m_physical_mask = ( phys_bits >= 63 ) ? ~0ULL : ( ( 1ULL << phys_bits ) - 1ULL );
    }

    bool is_address_valid( std::uint64_t pa, std::size_t size ) {
        if ( ( pa & ~m_physical_mask ) != 0 )
            return false;

        const auto end_pa = pa + ( size - 1 );
        if ( ( end_pa & ~m_physical_mask ) != 0 )
            return false;

        auto pfn = pa >> 12;
        return pfn >= mm_lowest_physical_page && pfn <= mm_highest_physical_page;
    }

    std::uint64_t physical_to_virtual( std::uint64_t pa ) {
        auto va = nt::mm_get_virtual_for_physical( pa );
        return reinterpret_cast< std::uint64_t >( va );
    }

    std::uint64_t virtual_to_physical( std::uint64_t va ) {
        auto pa = nt::mm_get_physical_address( reinterpret_cast< void* >( va ) );
        return pa.m_quad_part;
    }

    mmpfn_t* get_pfn_entry( std::uint64_t pfn ) {
        auto mm_pfn_database = nt::get_mm_pfn_database( );
        if ( !mm_pfn_database ) {
            return nullptr;
        }

        return &mm_pfn_database[ pfn ];
    }
}