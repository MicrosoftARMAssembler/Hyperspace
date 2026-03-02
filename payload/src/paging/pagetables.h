#pragma once

namespace paging {
	std::uint64_t m_system_cr3{ };
	std::uint64_t m_target_cr3{ };

	constexpr auto page_4kb_size = 0x1000ull;
	constexpr auto page_2mb_size = 0x200000ull;
	constexpr auto page_1gb_size = 0x40000000ull;

	constexpr auto page_shift = 12ull;
	constexpr auto page_2mb_shift = 21ull;
	constexpr auto page_1gb_shift = 30ull;

	constexpr auto page_4kb_mask = 0xFFFull;
	constexpr auto page_2mb_mask = 0x1FFFFFull;
	constexpr auto page_1gb_mask = 0x3FFFFFFFull;

	constexpr std::uint8_t detour_code[ ]{
		0x48, 0xB8,              // mov rax, imm64  ; Load stub address
		0x00, 0x00, 0x00, 0x00,  // placeholder for lower 32 bits of stub address
		0x00, 0x00, 0x00, 0x00,  // placeholder for upper 32 bits of stub address
		0xFF, 0xE0               // jmp rax         ; Jump to stub (preserves stack)
	};

    struct pt_entries_t {
        pml4e m_pml4e;
        pdpte m_pdpte;
        pde m_pde;
        pte m_pte;
    };

	enum class pt_type : std::uint8_t {
		pml4 = 0,
		pdpt,
		pd,
		pt
	};

	enum class page_protection : std::uint8_t {
		readwrite_execute = 0,
		readwrite,
		read_execute,
		readonly,
		inaccessible
	};

	std::uint64_t generate_va( std::uint64_t pml4e_index, std::uint64_t pdpte_index,
		std::uint64_t pde_index, std::uint64_t pte_index, std::uint64_t offset ) {
		return ( pml4e_index << 39 ) |
			( pdpte_index << 30 ) |
			( pde_index << 21 ) |
			( pte_index << 12 ) |
			offset;
	}
}

#define page_align(va) ((std::uint64_t)(va) & ~(paging::page_4kb_size - 1))