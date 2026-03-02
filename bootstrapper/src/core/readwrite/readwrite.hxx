#pragma once

namespace rw {
    template <typename ret_t = std::uint64_t, typename addr_t = ret_t>
    ret_t read_virt( addr_t va ) {
        ret_t buffer{ };
        nt::memcpy( &buffer, reinterpret_cast< void* >( va ), sizeof( ret_t ) );
        return buffer;
    }

    template <typename addr_t = std::uint64_t, typename value_t>
    bool write_virt( addr_t va, value_t value ) {
        nt::memcpy( reinterpret_cast< void* >( va ), &value, sizeof( value_t ) );
        return true;
    }
}