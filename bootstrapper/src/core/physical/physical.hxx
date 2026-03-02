#pragma once

namespace phys {
    constexpr std::uint32_t m_copy_physical = 1;
    constexpr std::uint32_t m_copy_virtual = 2;

    std::uint64_t physical_to_virtual( std::uint64_t physical_address ) {
        physical_address_t pa{};
        pa.m_quad_part = physical_address;
        return reinterpret_cast< std::uint64_t >( nt::mm_get_virtual_for_physical( pa ) );
    }

    std::uint64_t virtual_to_physical( std::uint64_t va ) {
        auto pa = nt::mm_get_physical_address( reinterpret_cast< void* >( va ) );
        return pa.m_quad_part;
    }
}