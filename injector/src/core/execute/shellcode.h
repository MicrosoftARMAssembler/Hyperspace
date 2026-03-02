#pragma once

namespace execute {
	const std::uint32_t m_executed = 1;
	const std::uint32_t m_preparing = 2;
	const std::uint32_t m_called = 2;

	enum shellcode_status : std::uint8_t {
		executing = 1,
		preparing = 2,
		finished = 3
	};

	struct shellcode_t {
		std::uint64_t m_original_fn;
		std::vector <std::uint8_t> m_code{ };
		std::uint32_t m_size{ };
	};

#pragma pack(push, 8) 
	struct shellcode_data_t {
		std::uint32_t m_status;
		std::uint32_t m_padding;
		std::uint64_t m_entry_point;
		std::uint64_t m_dll_base;
	};
#pragma pack(pop)
}