#pragma once 

namespace paging {
    constexpr auto page_4kb_size = 0x1000ull;
    constexpr auto page_2mb_size = 0x200000ull;
    constexpr auto page_1gb_size = 0x40000000ull;

    constexpr auto page_shift = 12ull;
    constexpr auto page_2mb_shift = 21ull;
    constexpr auto page_1gb_shift = 30ull;

    constexpr auto page_4kb_mask = 0xFFFull;
    constexpr auto page_2mb_mask = 0x1FFFFFull;
    constexpr auto page_1gb_mask = 0x3FFFFFFFull;

    enum class e_protection {
        read,
        write,
        read_write,
        read_write_execute
    };

    const char* protection_to_string( e_protection p ) {
        switch ( p ) {
        case e_protection::read: return "Read";
        case e_protection::write: return "Write";
        case e_protection::read_write: return "Read/Write";
        case e_protection::read_write_execute: return "Read/Write/Execute";
        default: return "Unknown";
        }
    }

    enum class e_page_table : std::uint32_t {
        pml4e,
        pdpte,
        pde,
        pte
    };

    using read_func_t = bool( * )( std::uint64_t, void*, std::size_t );
    using write_func_t = bool( * )( std::uint64_t, void*, std::size_t );

    std::uint64_t create_virtual_address( std::uint32_t pml4_index, bool use_high_address ) {
        auto additional_offset = static_cast< std::uint64_t >( rand( ) % 512 ) << 21;

        auto get_pml4e = [ &pml4_index ]( ) {
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
}