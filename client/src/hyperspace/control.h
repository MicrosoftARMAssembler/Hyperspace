#pragma once

namespace hyperspace {
	const std::uint32_t m_magic_code = 0xceedbeef;

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

		struct pt_entries_t {
			pml4e m_pml4e;
			pdpte m_pdpte;
			pde m_pde;
			pte m_pte;
		};

		enum class page_protection : std::uint8_t {
			readwrite_execute = 0,
			readwrite,
			inaccessible
		};
	}
}