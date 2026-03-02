#pragma once

#define image_dos_signature     0x5A4D
#define image_nt_signature      0x4550
#define image_nt_optional_hdr32	0x10B
#define image_nt_optional_hdr64	0x20B

#define image_dir_export		0
#define image_dir_import		1
#define image_dir_resource		2
#define image_dir_exception		3
#define image_dir_security		4
#define image_dir_relocs		5
#define image_dir_debug			6
#define image_dir_arch			7
#define image_dir_globalptr		8
#define image_dir_tls			9
#define image_dir_load_config	10
#define image_dir_bound_import	11
#define image_dir_iat			12
#define image_dir_delay_import	13
#define image_dir_com_desc		14
#define image_dir_entries		16

#define image_reloc_absolute	0
#define image_reloc_high		1
#define image_reloc_low			2
#define image_reloc_highlow		3
#define image_reloc_highadj		4
#define image_reloc_specific_5	5
#define image_reloc_reserved	6
#define image_reloc_specific_7	7
#define image_reloc_specific_8	8
#define image_reloc_specific_9	9
#define image_reloc_dir64		10
#define image_reloc_offset(x)	(x & 0xFFF)

typedef struct {

	uint16_t machine, number_of_sections;
	uint32_t time_stamp;
	uint32_t symbol_table, number_of_symbols;
	uint16_t optional_head_sz, characts;

} nt_image_file_head_t;

typedef struct {

	uint8_t name[8];

	union {

		uint32_t physical;
		uint32_t size;

	} misc;

	uint32_t va;
	uint32_t raw_size, raw_data;
	uint32_t relocs_data;
	uint32_t linenumbers;
	uint16_t relocs_cnt;
	uint16_t linenumbers_cnt;
	uint32_t characts;

} nt_image_section_head_t;

typedef struct {

	uint32_t va, size;

} nt_image_data_dir_t;

typedef struct {

	uint16_t	magic;
	uint8_t     major_linker_ver;
	uint8_t     minor_linker_ver;
	uint32_t    size_of_code;
	uint32_t    size_of_init_data;
	uint32_t    size_of_uninit_data;
	uint32_t    entry_point;
	uint32_t    base_of_code;
	uint64_t    image_base;
	uint32_t    section_align;
	uint32_t    file_align;
	uint16_t    major_os_ver;
	uint16_t    minor_os_ver;
	uint16_t    major_image_ver;
	uint16_t    minor_image_ver;
	uint16_t    major_subsystem_ver;
	uint16_t    minor_subsystem_ver;
	uint32_t    win32_ver;
	uint32_t    size_of_image;
	uint32_t    size_of_headers;
	uint32_t    checksum;
	uint16_t    subsystem;
	uint16_t    dll_characts;
	uint64_t    size_of_stack_reserve;
	uint64_t    size_of_stack_commit;
	uint64_t    size_of_heap_reserve;
	uint64_t    size_of_heap_commit;
	uint32_t    loader_flags;
	uint32_t    number_of_rva_and_sizes;

	nt_image_data_dir_t data_directory[image_dir_entries];

} nt_image_optional_head_t;

typedef struct {

	uint16_t magic;
	uint16_t last_page_bytes;
	uint16_t pages_cnt;
	uint16_t relocs;
	uint16_t header_sz;
	uint16_t min_alloc;
	uint16_t max_alloc;
	uint16_t ss;
	uint16_t sp;
	uint16_t checksum;
	uint16_t ip;
	uint16_t cs;
	uint16_t lfarlc;
	uint16_t overlays;
	uint16_t reserved[4];
	uint16_t oem_id;
	uint16_t oem_info;
	uint16_t reserved2[10];
	int32_t  lfanew;

} nt_image_dos_head_t;

typedef struct {

	uint32_t characts;
	uint32_t time_stamp_date;
	uint16_t major_ver;
	uint16_t minor_ver;
	uint32_t name;
	uint32_t base;
	uint32_t funcs_num;
	uint32_t names_num;
	uint32_t funcs_addr;
	uint32_t names_addr;
	uint32_t ordinals_addr;

} nt_image_export_dir_t;

typedef struct {

	union {

		uint32_t characts;
		uint32_t original;

	} thunk;

	uint32_t time_date_stamp;
	uint32_t chain;
	uint32_t name;
	uint32_t first_thunk;

} nt_image_import_desc_t;

typedef struct {

	union {
		uint64_t forwarder_string;
		uint64_t function;
		uint64_t ordinal;
		uint64_t address_of_data;
	} u1;

} nt_image_thunk_data_t;

typedef struct {

	uint16_t	hint;
	char		name[1];

} nt_image_import_name_t;

typedef struct {

	uint32_t					signature;
	nt_image_file_head_t		file;
	nt_image_optional_head_t	optional;

} nt_image_headers_t;

typedef struct {

	uint32_t va, size;

} nt_image_base_reloc_t;

static nt_image_headers_t* nt_get_base(uint8_t* base) {

	nt_image_dos_head_t* dos = (nt_image_dos_head_t*)base;

	return (nt_image_headers_t*)(base + dos->lfanew);
}

static uint8_t* get_export(uint8_t* base, const char* name) {

	nt_image_export_dir_t* nt = (nt_image_export_dir_t*)(base + nt_get_base(base)->optional.data_directory[image_dir_export].va);
	uint32_t* names = (uint32_t*)(base + nt->names_addr);

	const auto string_compare = [](char* left, char* right) {

		while (*left == *right++)
			if (*left++ == 0) return (0);

		return (*(char*)left - *(char*) --right);

		};

	for (uint64_t idx = 0; idx < nt->names_num; ++idx) {

		char* symbol = (char*)(base + names[idx]);

		if (string_compare(symbol, (char*)name) == 0) {

			uint32_t* rva = (uint32_t*)(base + nt->funcs_addr);
			uint16_t* ordinal = (uint16_t*)(base + nt->ordinals_addr);

			return (uint8_t*)base + rva[ordinal[idx]];
		}

	}

	return 0;
}

static nt_image_section_head_t* first_section(uint8_t* base) {

#define crt_offsetof(st, m) ((uint64_t) &(((st*) 0)->m))
	nt_image_headers_t* head = nt_get_base(base);

	return ((nt_image_section_head_t*)((uint64_t)(head)+crt_offsetof(nt_image_headers_t, optional) + (head)->file.optional_head_sz));
}

static uint64_t rel_translate(uint8_t* base, uint64_t relative) {

	nt_image_section_head_t* section = first_section(base);

	for (uint16_t idx = 0; idx < nt_get_base(base)->file.number_of_sections; idx++, section++) {

		if (relative >= section->va && relative < section->va + section->misc.size) {
			return (((uint64_t)base) + section->raw_data + (relative - section->va));
		}

	}

	return 0;
}