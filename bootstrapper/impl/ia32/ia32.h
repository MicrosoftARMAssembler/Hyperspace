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


struct eprocess_t;
struct peb_t; 
struct ethread_t;

typedef union _virt_addr_t
{
    std::uintptr_t value;
    struct
    {
        std::uint64_t offset : 12;        // 0:11
        std::uint64_t pte_index : 9;      // 12:20
        std::uint64_t pde_index : 9;      // 21:29
        std::uint64_t pdpte_index : 9;    // 30:38
        std::uint64_t pml4e_index : 9;    // 39:47
        std::uint64_t reserved : 16;      // 48:63
    };
    struct
    {
        std::uint64_t offset_4kb : 12;    // 4KB page offset
        std::uint64_t pt_offset : 9;
        std::uint64_t pd_offset : 9;
        std::uint64_t pdpt_offset : 9;
        std::uint64_t pml4_offset : 9;
        std::uint64_t reserved2 : 16;
    };
    struct
    {
        std::uint64_t offset_2mb : 21;    // 2MB page offset
        std::uint64_t pd_offset2 : 9;
        std::uint64_t pdpt_offset2 : 9;
        std::uint64_t pml4_offset2 : 9;
        std::uint64_t reserved3 : 16;
    };
    struct
    {
        std::uint64_t offset_1gb : 30;    // 1GB page offset
        std::uint64_t pdpt_offset3 : 9;
        std::uint64_t pml4_offset3 : 9;
        std::uint64_t reserved4 : 16;
    };
} virt_addr_t, *pvirt_addr_t;

typedef union _pml4e
{
    std::uint64_t value;
    struct
    {
        std::uint64_t present : 1;                   // Must be 1 if valid
        std::uint64_t read_write : 1;               // Write access control
        std::uint64_t user_supervisor : 1;           // User/supervisor access control
        std::uint64_t page_write_through : 1;        // Write-through caching
        std::uint64_t cached_disable : 1;            // Cache disable
        std::uint64_t accessed : 1;                  // Set when accessed
        std::uint64_t ignored0 : 1;                  // Ignored
        std::uint64_t large_page : 1;               // Reserved (must be 0)
        std::uint64_t ignored1 : 4;                 // Ignored
        std::uint64_t pfn : 36;                     // Physical frame number
        std::uint64_t reserved : 4;                 // Reserved for software
        std::uint64_t ignored2 : 11;                // Ignored
        std::uint64_t no_execute : 1;               // No-execute bit
    } hard;
} pml4e, * ppml4e;

typedef union _pdpte
{
    std::uint64_t value;
    struct
    {
        std::uint64_t present : 1;                   // Must be 1 if valid
        std::uint64_t read_write : 1;               // Write access control
        std::uint64_t user_supervisor : 1;           // User/supervisor access control
        std::uint64_t page_write_through : 1;        // Write-through caching
        std::uint64_t cached_disable : 1;            // Cache disable
        std::uint64_t accessed : 1;                  // Set when accessed
        std::uint64_t dirty : 1;                    // Set when written to (1GB pages)
        std::uint64_t page_size : 1;                // 1=1GB page, 0=points to page directory
        std::uint64_t ignored1 : 4;                 // Ignored
        std::uint64_t pfn : 36;                     // Physical frame number
        std::uint64_t reserved : 4;                 // Reserved for software
        std::uint64_t ignored2 : 11;                // Ignored
        std::uint64_t no_execute : 1;               // No-execute bit
    } hard;
} pdpte, * ppdpte;

typedef union _pde
{
    std::uint64_t value;
    struct
    {
        std::uint64_t present : 1;                   // Must be 1 if valid
        std::uint64_t read_write : 1;               // Write access control
        std::uint64_t user_supervisor : 1;           // User/supervisor access control
        std::uint64_t page_write_through : 1;        // Write-through caching
        std::uint64_t cached_disable : 1;            // Cache disable
        std::uint64_t accessed : 1;                  // Set when accessed
        std::uint64_t dirty : 1;                    // Set when written to (2MB pages)
        std::uint64_t page_size : 1;                // 1=2MB page, 0=points to page table
        std::uint64_t global : 1;                   // Global page (if CR4.PGE=1)
        std::uint64_t ignored1 : 3;                 // Ignored
        std::uint64_t pfn : 36;                     // Physical frame number
        std::uint64_t reserved : 4;                 // Reserved for software
        std::uint64_t ignored2 : 11;                // Ignored
        std::uint64_t no_execute : 1;               // No-execute bit
    } hard;
} pde, * ppde;

typedef union _pte
{
    std::uint64_t value;
    struct
    {
        std::uint64_t present : 1;                   // Must be 1 if valid
        std::uint64_t read_write : 1;               // Write access control
        std::uint64_t user_supervisor : 1;           // User/supervisor access control
        std::uint64_t page_write_through : 1;        // Write-through caching
        std::uint64_t cached_disable : 1;            // Cache disable
        std::uint64_t accessed : 1;                  // Set when accessed
        std::uint64_t dirty : 1;                    // Set when written to
        std::uint64_t pat : 1;                      // Page Attribute Table bit
        std::uint64_t global : 1;                   // Global page
        std::uint64_t ignored1 : 3;                 // Ignored
        std::uint64_t pfn : 36;                     // Physical frame number
        std::uint64_t reserved : 4;                 // Reserved for software
        std::uint64_t ignored2 : 7;                 // Ignored
        std::uint64_t protection_key : 4;           // Protection key
        std::uint64_t no_execute : 1;               // No-execute bit
    } hard;
} pte, * ppte;

typedef union _cr3 {
    std::uint64_t flags;

    struct {
        std::uint64_t pcid : 12;
        std::uint64_t pfn : 36;
        std::uint64_t reserved_1 : 12;
        std::uint64_t reserved_2 : 3;
        std::uint64_t pcid_invalidate : 1;
    };
} cr3 , * pcr3;

enum pe_magic_t {
    dos_header = 0x5a4d,
    nt_headers = 0x4550,
    opt_header = 0x020b
};

enum memory_caching_type_t {
    MmNonCached = 0,
    MmCached = 1,
    MmWriteCombined = 2,
    MmHardwareCoherentCached = 3,
    MmNonCachedUnordered = 4,
    MmUSWCCached = 5,
    MmMaximumCacheType = 6
};

struct dos_header_t {
    std::int16_t m_magic;
    std::int16_t m_cblp;
    std::int16_t m_cp;
    std::int16_t m_crlc;
    std::int16_t m_cparhdr;
    std::int16_t m_minalloc;
    std::int16_t m_maxalloc;
    std::int16_t m_ss;
    std::int16_t m_sp;
    std::int16_t m_csum;
    std::int16_t m_ip;
    std::int16_t m_cs;
    std::int16_t m_lfarlc;
    std::int16_t m_ovno;
    std::int16_t m_res0[ 0x4 ];
    std::int16_t m_oemid;
    std::int16_t m_oeminfo;
    std::int16_t m_res1[ 0xa ];
    std::int32_t m_lfanew;

    
    constexpr bool is_valid( ) {
        return m_magic == pe_magic_t::dos_header;
    }
};

struct data_directory_t {
    std::int32_t m_virtual_address;
    std::int32_t m_size;

    template< class type_t >
    
    type_t as_rva(
        std::uint64_t rva
    ) {
        return reinterpret_cast< type_t >( rva + m_virtual_address );
    }
};

struct import_descriptor_t {
    union {
        std::uint32_t m_characteristics;
        std::uint32_t m_original_first_thunk;
    };
    std::uint32_t m_time_date_stamp;
    std::uint32_t m_forwarder_chain;
    std::uint32_t m_name;
    std::uint32_t m_first_thunk;
};

struct image_delayload_descriptor_t {
    union {
        std::uint32_t all_attributes;
        struct {
            std::uint32_t m_rva_based : 1;
            std::uint32_t m_reverved_attributes : 1;
        };
    };

    std::uint32_t m_dll_name_rva;
    std::uint32_t m_module_handle_rva;
    std::uint32_t m_import_address_table_rva;
    std::uint32_t m_import_name_table_rva;
    std::uint32_t m_bound_import_address_table_rva;
    std::uint32_t m_unload_information_table_rva;
    std::uint32_t m_time_date_stamp;
};

struct nt_headers_t {
    std::int32_t m_signature;
    std::int16_t m_machine;
    std::int16_t m_number_of_sections;
    std::int32_t m_time_date_stamp;
    std::int32_t m_pointer_to_symbol_table;
    std::int32_t m_number_of_symbols;
    std::int16_t m_size_of_optional_header;
    std::int16_t m_characteristics;

    std::int16_t m_magic;
    std::int8_t m_major_linker_version;
    std::int8_t m_minor_linker_version;
    std::int32_t m_size_of_code;
    std::int32_t m_size_of_initialized_data;
    std::int32_t m_size_of_uninitialized_data;
    std::int32_t m_address_of_entry_point;
    std::int32_t m_base_of_code;
    std::uint64_t m_image_base;
    std::int32_t m_section_alignment;
    std::int32_t m_file_alignment;
    std::int16_t m_major_operating_system_version;
    std::int16_t m_minor_operating_system_version;
    std::int16_t m_major_image_version;
    std::int16_t m_minor_image_version;
    std::int16_t m_major_subsystem_version;
    std::int16_t m_minor_subsystem_version;
    std::int32_t m_win32_version_value;
    std::int32_t m_size_of_image;
    std::int32_t m_size_of_headers;
    std::int32_t m_check_sum;
    std::int16_t m_subsystem;
    std::int16_t m_dll_characteristics;
    std::uint64_t m_size_of_stack_reserve;
    std::uint64_t m_size_of_stack_commit;
    std::uint64_t m_size_of_heap_reserve;
    std::uint64_t m_size_of_heap_commit;
    std::int32_t m_loader_flags;
    std::int32_t m_number_of_rva_and_sizes;

    data_directory_t m_export_table;
    data_directory_t m_import_table;
    data_directory_t m_resource_table;
    data_directory_t m_exception_table;
    data_directory_t m_certificate_table;
    data_directory_t m_base_relocation_table;
    data_directory_t m_debug;
    data_directory_t m_architecture;
    data_directory_t m_global_ptr;
    data_directory_t m_tls_table;
    data_directory_t m_load_config_table;
    data_directory_t m_bound_import;
    data_directory_t m_iat;
    data_directory_t m_delay_import_descriptor;
    data_directory_t m_clr_runtime_header;
    data_directory_t m_reserved;

    
    constexpr bool is_valid( ) {
        return m_signature == pe_magic_t::nt_headers
            && m_magic == pe_magic_t::opt_header;
    }
};

struct export_directory_t {
    std::int32_t m_characteristics;
    std::int32_t m_time_date_stamp;
    std::int16_t m_major_version;
    std::int16_t m_minor_version;
    std::int32_t m_name;
    std::int32_t m_base;
    std::int32_t m_number_of_functions;
    std::int32_t m_number_of_names;
    std::int32_t m_address_of_functions;
    std::int32_t m_address_of_names;
    std::int32_t m_address_of_names_ordinals;
};

struct section_header_t {
    char m_name[ 0x8 ];
    union {
        std::int32_t m_physical_address;
        std::int32_t m_virtual_size;
    };
    std::int32_t m_virtual_address;
    std::int32_t m_size_of_raw_data;
    std::int32_t m_pointer_to_raw_data;
    std::int32_t m_pointer_to_relocations;
    std::int32_t m_pointer_to_line_numbers;
    std::int16_t m_number_of_relocations;
    std::int16_t m_number_of_line_numbers;
    std::int32_t m_characteristics;
};

struct unicode_string_t {
    std::uint16_t m_length;
    std::uint16_t m_maximum_length;
    wchar_t* m_buffer;
};

struct rtl_process_module_information_t {
    HANDLE  m_section;
    void*   m_mapped_base;
    void*   m_image_base;
    uint32_t m_image_size;
    uint32_t m_flags;
    uint16_t m_load_order_index;
    uint16_t m_init_order_index;
    uint16_t m_load_count;
    uint16_t m_offset_to_file_name;
    uint8_t  m_full_path[256];
};

struct rtl_process_modules_t {
    uint32_t    m_count;
    rtl_process_module_information_t m_modules[1];
};

struct mm_unloaded_drivers_t {
    UNICODE_STRING m_name;
    PVOID m_module_start;
    PVOID m_module_end;
    ULONG64 m_unload_time;
};

typedef struct _MM_UNLOADED_DRIVER {
    UNICODE_STRING 	Name;
    PVOID 			ModuleStart;
    PVOID 			ModuleEnd;
    ULONG64 		UnloadTime;
} MM_UNLOADED_DRIVER, * PMM_UNLOADED_DRIVER;

struct ex_push_lock_t {
    union {
        struct {
            std::uint64_t m_locked : 1;
            std::uint64_t m_waiting : 1;
            std::uint64_t m_waking : 1;
            std::uint64_t m_multiple_shared : 1;
            std::uint64_t m_shared : 60;
        };
        std::uint64_t m_value;
        void* m_ptr;
    };
}; // Size: 0x8

struct dispatcher_header_t {
    union {
        struct {
            std::uint8_t m_type;
            union {
                std::uint8_t m_absolute_timer : 1;
                std::uint8_t m_timer_resolution : 1;
                std::uint8_t m_timer_resolution_required : 1;
                std::uint8_t m_timer_resolution_set : 1;
            };
            union {
                std::uint8_t m_inserted : 1;
                std::uint8_t m_large_stack : 1;
                std::uint8_t m_priority_boost : 1;
                std::uint8_t m_thread_control_flags;
            };
            std::uint8_t m_signal_state;
        };
        std::uint32_t m_lock;
    };
    std::uint32_t m_size;
    union {
        std::uint64_t m_reserved1;
        struct {
            std::uint8_t m_hand_size;
            std::uint8_t m_inserted_2;
        };
    };
    union {
        std::uint64_t m_signal_state_2;
        struct {
            std::uint32_t m_signal_state_3;
            std::uint32_t m_thread_apc_disable;
        };
    };
}; // Size: 0x18

struct list_entry_t {
    list_entry_t* m_flink;
    list_entry_t* m_blink;
};

struct single_list_entry_t {
    single_list_entry_t* m_next;
};

struct kwait_status_register_t {
    union {
        std::uint8_t m_flags;
        struct {
            std::uint8_t m_state : 3;
            std::uint8_t m_affinity : 1;
            std::uint8_t m_priority : 1;
            std::uint8_t m_apc : 1;
            std::uint8_t m_user_apc : 1;
            std::uint8_t m_alert : 1;
        };
    };
}; // Size: 0x1

struct large_integer_t {
    union {
        struct {
            std::uint32_t m_low_part;
            std::int32_t m_high_part;
        };
        struct {
            std::uint32_t m_low_part;
            std::int32_t m_high_part;
        } u;
        std::int64_t m_quad_part;
    };
};

struct kdpc_t {
    std::uint8_t m_type;
    std::uint8_t m_importance;
    std::int32_t m_number;
    list_entry_t m_dpc_list_entry;
    void* m_deferred_routine;
    void* m_deferred_context;
    void* m_system_argument_1;
    void* m_system_argument_2;
    void* m_dpc_data;
};

struct ktimer_t {
    dispatcher_header_t m_header;
    std::uint64_t m_due_time;
    list_entry_t m_timer_list_entry;
    kdpc_t* m_dpc;
    std::uint32_t m_period;
    std::uint32_t m_processor;
    std::uint32_t m_timer_type;
}; // Size: 0x40

struct ktimer_table_entry_t {
    ULONG lock;                                                             //0x0
    struct list_entry_t entry;                                               //0x4
    struct large_integer_t time;                                             //0x10
};

struct ktimer_table_state_t {
    ULONGLONG last_timer_expiration[ 1 ];                                       //0x0
    ULONG last_timer_hand[ 1 ];                                                 //0x8
};

struct ktimer_table_t {
    struct ktimer_t* timer_expiry[ 16 ];                                        //0x0
    struct ktimer_table_entry_t timer_entries[ 1 ][ 256 ];                        //0x40
    struct ktimer_table_state_t table_state;                                  //0x1840
};

struct group_affinity_t {
    std::uint64_t m_mask;
    std::uint16_t m_group;
    std::uint16_t m_reserved[ 3 ];
}; // Size: 0x10

struct kevent_t {
    dispatcher_header_t m_header;
}; // Size: 0x18

struct mmsupport_t {
    list_entry_t m_work_set_exp_head;                   // +0x000
    std::uint64_t m_flags;                              // +0x010
    std::uint64_t m_last_trim_time;                     // +0x018
    union {
        std::uint64_t m_page_fault_count;
        std::uint64_t m_peak_virtual_size;
        std::uint64_t m_virtual_size;
    };                                                  // +0x020
    std::uint64_t m_min_ws_size;                       // +0x028
    std::uint64_t m_max_ws_size;                       // +0x030
    std::uint64_t m_virtual_memory_threshold;          // +0x038
    std::uint64_t m_working_set_size;                  // +0x040
    std::uint64_t m_peak_working_set_size;            // +0x048
};

struct ex_fast_ref_t {
    union {
        void* m_object;
        std::uint64_t m_ref_cnt : 4;
        std::uint64_t m_value;
    };
}; // Size: 0x8

struct kprocess_t {
    dispatcher_header_t m_header;                       // +0x000
    list_entry_t m_profile_list_head;                  // +0x018
    std::uint64_t m_directory_table_base;              // +0x028
    std::uint64_t m_flags;                             // +0x030
    std::uint64_t m_flags2;                            // +0x038
    std::uint64_t m_session_id;                        // +0x040
    mmsupport_t m_mm;                                  // +0x048
    list_entry_t m_process_list_entry;                 // +0x0E0
    std::uint64_t m_total_cycle_time;                  // +0x0F0
    std::uint64_t m_create_time;                       // +0x0F8
    std::uint64_t m_user_time;                         // +0x100
    std::uint64_t m_kernel_time;                       // +0x108
    list_entry_t m_active_process_links;               // +0x110
    std::uint64_t m_process_quota_usage[ 2 ];            // +0x120
    std::uint64_t m_process_quota_peak[ 2 ];             // +0x130
    std::uint64_t m_commit_charge;                     // +0x140
    std::uint64_t m_peak_commit_charge;                // +0x148
    std::uint64_t m_peak_virtual_size;                 // +0x150
    std::uint64_t m_virtual_size;                      // +0x158
    std::uint32_t m_exit_status;                       // +0x160
    std::uint32_t m_address_policy;                    // +0x164
};

struct kapc_state_t {
    list_entry_t m_apc_list_head[ 2 ];
    eprocess_t* m_process;
    std::uint8_t m_kernel_apc_in_progress;
    std::uint8_t m_kernel_apc_pending;
    std::uint8_t m_user_apc_pending;
    std::uint8_t m_pad;
}; // Size: 0x40

struct m128a {
    std::uint64_t m_low;                             // +0x000 - Low 64 bits
    std::int64_t m_high;                             // +0x008 - High 64 bits (signed)
}; // Size: 0x010

struct activation_context_stack_t {
    std::uint32_t m_flags;                           // +0x000 - Flags
    std::uint32_t m_next_cookie_sequence_number;     // +0x004 - Next cookie sequence
    void* m_active_frame;                            // +0x008 - Active frame pointer
    list_entry_t m_frame_list_cache;                 // +0x010 - Frame list cache
    std::uint32_t m_flags2;                          // +0x020 - Additional flags
    std::uint32_t m_padding;                         // +0x024 - Padding
    std::uint64_t m_reserved[ 2 ];                     // +0x028 - Reserved
}; // Size: 0x038

struct ejob_t {
    dispatcher_header_t m_header;                    // +0x000 - KOBJECTS_HEADER
    list_entry_t m_job_list_entry;                   // +0x018 - Link in global job list
    std::uint64_t m_process_list_head;               // +0x028 - List of processes in job
    std::uint64_t m_job_lock;                        // +0x030 - Job lock
    std::uint32_t m_total_user_time;                 // +0x038 - Total user time
    std::uint32_t m_total_kernel_time;               // +0x03C - Total kernel time
    std::uint32_t m_total_page_fault_count;          // +0x040 - Total page faults
    std::uint32_t m_total_processes;                 // +0x044 - Number of processes
    std::uint32_t m_active_processes;                // +0x048 - Active processes
    std::uint32_t m_total_terminated_processes;      // +0x04C - Terminated processes
    std::uint64_t m_per_process_user_time_limit;     // +0x050 - Per-process time limit
    std::uint64_t m_per_job_user_time_limit;         // +0x058 - Per-job time limit
    std::uint64_t m_limit_flags;                     // +0x060 - Limit flags
    std::uint64_t m_minimum_working_set_size;        // +0x068 - Min working set
    std::uint64_t m_maximum_working_set_size;        // +0x070 - Max working set
    std::uint64_t m_active_process_limit;            // +0x078 - Active process limit
    std::uint64_t m_affinity;                        // +0x080 - Affinity mask
    std::uint64_t m_priority_class;                  // +0x088 - Priority class
    std::uint64_t m_per_process_memory_limit;        // +0x090 - Per-process memory limit
    std::uint64_t m_per_job_memory_limit;            // +0x098 - Per-job memory limit
    std::uint64_t m_reserved[ 2 ];                     // +0x0A0 - Reserved
    void* m_completion_port;                         // +0x0B0 - Completion port
    std::uint64_t m_completion_key;                  // +0x0B8 - Completion key
    std::uint64_t m_session_id;                      // +0x0C0 - Session ID
    std::uint64_t m_silo_root;                       // +0x0C8 - Silo root
    std::uint64_t m_container;                       // +0x0D0 - Container
    std::uint64_t m_container_silo;                  // +0x0D8 - Container silo
    std::uint64_t m_reserved2[ 4 ];                    // +0x0E0 - Reserved
}; // Size: 0x100

struct pagefault_history_t {
    std::uint64_t m_timestamp;           // +0x000 - System time when fault occurred
    std::uint64_t m_virtual_address;     // +0x008 - Virtual address that caused fault
    std::uint32_t m_flags;               // +0x010 - Fault flags (read/write, user/kernel, etc.)
    std::uint32_t m_reserved;            // +0x014 - Reserved/padding
    std::uint32_t m_process_id;          // +0x018 - Process ID that caused fault
    std::uint32_t m_thread_id;           // +0x01C - Thread ID that caused fault
    std::uint64_t m_instruction_pointer; // +0x020 - IP when fault occurred
    std::uint64_t m_stack_pointer;       // +0x028 - SP when fault occurred
}; // Size: 0x030

struct ktrap_frame_t {
    std::uint64_t m_p1_home;                    // +0x000
    std::uint64_t m_p2_home;                    // +0x008
    std::uint64_t m_p3_home;                    // +0x010
    std::uint64_t m_p4_home;                    // +0x018
    std::uint64_t m_p5;                         // +0x020

    std::uint8_t m_previous_mode;               // +0x028
    std::uint8_t m_previous_irql;               // +0x029
    std::uint8_t m_fault_indicator;             // +0x02A
    std::uint8_t m_exception_active;            // +0x02B
    std::uint32_t m_mxcsr;                      // +0x02C

    std::uint64_t m_rax;                        // +0x030
    std::uint64_t m_rcx;                        // +0x038
    std::uint64_t m_rdx;                        // +0x040
    std::uint64_t m_r8;                         // +0x048
    std::uint64_t m_r9;                         // +0x050
    std::uint64_t m_r10;                        // +0x058
    std::uint64_t m_r11;                        // +0x060

    union {
        std::uint64_t m_gs_base;                // +0x068
        std::uint64_t m_gs_swap;
    };

    m128a m_xmm0;                               // +0x070
    m128a m_xmm1;                               // +0x080
    m128a m_xmm2;                               // +0x090
    m128a m_xmm3;                               // +0x0A0
    m128a m_xmm4;                               // +0x0B0
    m128a m_xmm5;                               // +0x0C0

    union {
        std::uint64_t m_fault_address;          // +0x0D0
        std::uint64_t m_context_record;
    };

    std::uint64_t m_dr0;                        // +0x0D8
    std::uint64_t m_dr1;                        // +0x0E0
    std::uint64_t m_dr2;                        // +0x0E8
    std::uint64_t m_dr3;                        // +0x0F0
    std::uint64_t m_dr6;                        // +0x0F8
    std::uint64_t m_dr7;                        // +0x100

    // Debug registers block
    std::uint64_t m_debug_control;              // +0x108
    std::uint64_t m_last_branch_to_rip;         // +0x110
    std::uint64_t m_last_branch_from_rip;       // +0x118
    std::uint64_t m_last_exception_to_rip;      // +0x120
    std::uint64_t m_last_exception_from_rip;    // +0x128

    std::uint16_t m_seg_ds;                     // +0x130
    std::uint16_t m_seg_es;                     // +0x132
    std::uint16_t m_seg_fs;                     // +0x134
    std::uint16_t m_seg_gs;                     // +0x136

    std::uint64_t m_nested_trap_frame;                 // +0x138
    std::uint64_t m_rbx;                        // +0x140
    std::uint64_t m_rdi;                        // +0x148
    std::uint64_t m_rsi;                        // +0x150
    std::uint64_t m_rbp;                        // +0x158

    union {
        std::uint64_t m_error_code;             // +0x160
        std::uint64_t m_exception_frame;
    };

    std::uint64_t m_rip;                        // +0x168
    std::uint16_t m_seg_cs;                     // +0x170
    std::uint8_t m_fill0;                       // +0x172
    std::uint8_t m_logging;                     // +0x173
    std::uint16_t m_fill1[ 2 ];                   // +0x174
    std::uint32_t m_eflags;                     // +0x178
    std::uint32_t m_fill2;                      // +0x17C
    std::uint64_t m_rsp;                        // +0x180
    std::uint16_t m_seg_ss;                     // +0x188
    std::uint16_t m_fill3;                      // +0x18A
    std::uint32_t m_fill4;                      // +0x18C
}; // Size = 0x190


struct teb_t;
struct nt_tib_t {
    struct _exception_registration_record* m_exception_list;  // 0x000
    std::uint64_t m_stack_base;                                // 0x008
    std::uint64_t m_stack_limit;                              // 0x010
    std::uint64_t m_sub_system_tib;                           // 0x018
    union {
        std::uint64_t m_fiber_data;                           // 0x020
        std::uint32_t m_version;                            // 0x020
    };
    std::uint64_t m_arbitrary_user_pointer;                    // 0x028
    teb_t* m_self;                                          // 0x030
};

struct client_id_t {
    void* m_unique_process;
    void* m_unique_thread;
};

struct se_audit_process_creation_info_t {
    unicode_string_t* m_image_file_name;    // Pointer to UNICODE_STRING
};

struct ex_rundown_ref_t {
    union {
        std::uint64_t m_count;                    // Size=0x8
        void* m_ptr;                              // Size=0x8
    };
};

struct handle_table_t {
    std::uint32_t m_next_handle_needing_pool;       // +0x000
    std::int32_t m_extra_info_pages;                // +0x004
    std::uint64_t m_table_code;                     // +0x008
    eprocess_t* m_quota_process;                    // +0x010
    list_entry_t m_handle_table_list;               // +0x018
    std::uint32_t m_unique_process_id;              // +0x028
}; // Size: 0x02C

struct ps_dynamic_enforced_address_ranges_t {
    void* m_lock;         // EX_PUSH_LOCK or similar
    void* m_list_head;    // LIST_ENTRY*
    std::uint32_t   m_count;
    std::uint32_t   m_maximum;
    void* m_bitmap;       // RTL_BITMAP*
};

struct ps_process_wake_information_t {
    std::uint64_t   m_notification_channel;
    std::uint64_t   m_wake_counters;
    std::uint32_t   m_wake_mask;
    std::uint32_t   m_wake_state;
    std::uint32_t   m_no_wake_reason;
    std::uint32_t   m_padding;
};

struct wnf_state_name_t {
    std::uint32_t   m_data1;
    std::uint32_t   m_data2;
};

struct ps_interlocked_timer_delay_values_t {
    union {
        std::uint64_t m_value;
        struct {
            std::uint64_t m_delay_ms : 30;
            std::uint64_t m_coalescing_window_ms : 30;
            std::uint64_t m_reserved : 1;
            std::uint64_t m_new_timer_wheel : 1;
            std::uint64_t m_retry : 1;
            std::uint64_t m_locked : 1;
        };
    };
};

struct mmsupport_full_t {
    // This is a large structure, here is a simplified version with key fields.
    // You may want to expand it further as needed.
    struct mmsupport_t {
        list_entry_t     m_working_set_expansion_links;
        std::uint64_t    m_last_trim_time;
        std::uint64_t    m_flags;
        std::uint64_t    m_page_fault_count;
        std::uint64_t    m_peak_working_set_size;
        std::uint64_t    m_minimum_working_set_size;
        std::uint64_t    m_maximum_working_set_size;
        std::uint64_t    m_vm_working_set_size;
        // ... add more as needed
    } m_base;
    // Windows 10+ adds more fields, including working set lists, locks, etc.
    std::uint8_t     m_padding[ 0x140 ]; // Adjust size as needed for your use
};

struct file_object_t {
    std::int16_t     m_type;
    std::int16_t     m_size;
    void* m_device_object;
    void* m_vpb;
    void* m_fs_context;
    void* m_fs_context2;
    void* m_section_object_pointer;
    void* m_private_cache_map;
    std::int32_t     m_final_status;
    void* m_related_file_object;
    std::uint8_t     m_lock_operation;
    std::uint8_t     m_delete_pending;
    std::uint8_t     m_read_access;
    std::uint8_t     m_write_access;
    std::uint8_t     m_delete_access;
    std::uint8_t     m_shared_read;
    std::uint8_t     m_shared_write;
    std::uint8_t     m_shared_delete;
    std::uint32_t    m_flags;
    unicode_string_t m_file_name;
    // ... add more as needed
};

struct ewow64process_t {
    void* m_reserved1[ 3 ];
    void* m_wow64;
    void* m_reserved2[ 2 ];
    std::uint32_t    m_flags;
    // ... add more as needed
};

struct eprocess_quota_block_t {
    void* m_quota_entry;
    std::uint64_t    m_quota_limit;
    std::uint64_t    m_quota_peak;
    std::uint64_t    m_quota_usage;
    std::uint32_t    m_flags;
    std::uint32_t    m_reference_count;
    // ... add more as needed
};

struct mm_session_space_t {
    std::uint32_t    m_reference_count;
    std::uint32_t    m_session_id;
    void* m_process_reference_to_session;
    void* m_session_page_directory_index;
    void* m_session_va_start;
    void* m_session_va_end;
    // ... add more as needed
};

struct rtl_avl_tree_t {
    void* m_root;                                 // Size=0x8
};

struct eprocess_t {
    // Pcb and dispatch section (0x000 - 0x438)
    kprocess_t m_pcb;                                // +0x000

    // Process lock and identification section (0x438 - 0x458)
    ex_push_lock_t m_process_lock;                   // +0x438
    void* m_unique_process_id;                       // +0x440
    list_entry_t m_active_process_links;             // +0x448

    // Rundown and flags section (0x458 - 0x468)
    ex_rundown_ref_t m_rundown_protect;              // +0x458
    union {
        std::uint32_t m_flags2;                      // +0x460
        struct {
            std::uint32_t m_job_not_really_active : 1;
            std::uint32_t m_accounting_folded : 1;
            std::uint32_t m_new_process_reported : 1;
            std::uint32_t m_exit_process_reported : 1;
            std::uint32_t m_report_commit_changes : 1;
            std::uint32_t m_last_report_memory : 1;
            std::uint32_t m_force_wake_charge : 1;
            std::uint32_t m_cross_session_create : 1;
            std::uint32_t m_needs_handle_rundown : 1;
            std::uint32_t m_ref_trace_enabled : 1;
            std::uint32_t m_pico_created : 1;
            std::uint32_t m_empty_job_evaluated : 1;
            std::uint32_t m_default_page_priority : 3;
            std::uint32_t m_primary_token_frozen : 1;
            std::uint32_t m_process_verifier_target : 1;
            std::uint32_t m_restrict_set_thread_context : 1;
            std::uint32_t m_affinity_permanent : 1;
            std::uint32_t m_affinity_update_enable : 1;
            std::uint32_t m_propagate_node : 1;
            std::uint32_t m_explicit_affinity : 1;
            std::uint32_t m_process_execution_state : 2;
            std::uint32_t m_enable_read_vm_logging : 1;
            std::uint32_t m_enable_write_vm_logging : 1;
            std::uint32_t m_fatal_access_termination_requested : 1;
            std::uint32_t m_disable_system_allowed_cpu_set : 1;
            std::uint32_t m_process_state_change_request : 2;
            std::uint32_t m_process_state_change_in_progress : 1;
            std::uint32_t m_in_private : 1;
        };
    };
    union {
        std::uint32_t m_flags;                       // +0x464
        struct {
            std::uint32_t m_create_reported : 1;
            std::uint32_t m_no_debug_inherit : 1;
            std::uint32_t m_process_exiting : 1;
            std::uint32_t m_process_delete : 1;
            std::uint32_t m_manage_executable_memory_writes : 1;
            std::uint32_t m_vm_deleted : 1;
            std::uint32_t m_outswap_enabled : 1;
            std::uint32_t m_outswapped : 1;
            std::uint32_t m_fail_fast_on_commit_fail : 1;
            std::uint32_t m_wow64_va_space_4gb : 1;
            std::uint32_t m_address_space_initialized : 2;
            std::uint32_t m_set_timer_resolution : 1;
            std::uint32_t m_break_on_termination : 1;
            std::uint32_t m_deprioritize_views : 1;
            std::uint32_t m_write_watch : 1;
            std::uint32_t m_process_in_session : 1;
            std::uint32_t m_override_address_space : 1;
            std::uint32_t m_has_address_space : 1;
            std::uint32_t m_launch_prefetched : 1;
            std::uint32_t m_background : 1;
            std::uint32_t m_vm_top_down : 1;
            std::uint32_t m_image_notify_done : 1;
            std::uint32_t m_pde_update_needed : 1;
            std::uint32_t m_vdm_allowed : 1;
            std::uint32_t m_process_rundown : 1;
            std::uint32_t m_process_inserted : 1;
            std::uint32_t m_default_io_priority : 3;
            std::uint32_t m_process_self_delete : 1;
            std::uint32_t m_set_timer_resolution_link : 1;
        };
    };

    // Time and quota section (0x468 - 0x490)
    large_integer_t m_create_time;                   // +0x468
    std::uint64_t m_process_quota_usage[ 2 ];        // +0x470
    std::uint64_t m_process_quota_peak[ 2 ];         // +0x480

    // Memory management section (0x490 - 0x4a0)
    std::uint64_t m_peak_virtual_size;               // +0x490
    std::uint64_t m_virtual_size;                    // +0x498

    // Session and exception handling (0x4a0 - 0x4c0)
    list_entry_t m_session_process_links;            // +0x4a0
    union {
        void* m_exception_port_data;                 // +0x4b0
        std::uint64_t m_exception_port_value;
        std::uint64_t m_exception_port_state : 3;
    };
    ex_fast_ref_t m_token;                           // +0x4b8
    std::uint64_t m_mm_reserved;                     // +0x4c0

    // Locks and synchronization (0x4c8 - 0x4e8)
    ex_push_lock_t m_address_creation_lock;          // +0x4c8
    ex_push_lock_t m_page_table_commitment_lock;     // +0x4d0
    ethread_t* m_rotate_in_progress;                 // +0x4d8
    ethread_t* m_fork_in_progress;                   // +0x4e0
    ejob_t* volatile m_commit_charge_job;            // +0x4e8

    // AVL Tree and VAD management (0x4e8 - 0x500)
    rtl_avl_tree_t m_clone_root;                     // +0x4f0
    volatile std::uint64_t m_number_of_private_pages; // +0x4f8

    // Locks and page management (0x500 - 0x518)
    volatile std::uint64_t m_number_of_locked_pages;  // +0x500
    void* m_win32_process;                           // +0x508
    struct _EJOB* volatile m_job;                    // +0x510

    // Section and base address (0x518 - 0x528)
    void* m_section_object;                          // +0x518
    void* m_section_base_address;                    // +0x520
    std::uint32_t m_cookie;                          // +0x528

    // Working set and window station (0x530 - 0x540)
    pagefault_history_t* m_working_set_watch;        // +0x530
    void* m_win32_window_station;                    // +0x538

    // Process identification (0x540 - 0x550)
    void* m_inherited_from_unique_process_id;        // +0x540
    volatile std::uint64_t m_owner_process_id;        // +0x548

    // Process environment block and session (0x550 - 0x560)
    peb_t* m_peb;                                    // +0x550
    mm_session_space_t* m_session;                   // +0x558

    // Spare and quota management (0x560 - 0x570)
    void* m_spare1;                                  // +0x560
    eprocess_quota_block_t* m_quota_block;           // +0x568

    // Handle and debug management (0x570 - 0x590)
    handle_table_t* m_object_table;                  // +0x570
    void* m_debug_port;                              // +0x578
    ewow64process_t* m_wow64_process;                // +0x580
    void* m_device_map;                              // +0x588
    void* m_etw_data_source;                         // +0x590

    // Page directory and image details (0x590 - 0x5a0)
    std::uint64_t m_page_directory_pte;              // +0x590
    file_object_t* m_image_file_pointer;             // +0x5a0

    // Image and process details (0x5a0 - 0x5c0)
    std::uint8_t m_image_file_name[ 15 ];            // +0x5a8
    std::uint8_t m_priority_class;                   // +0x5b7
    void* m_security_port;                           // +0x5b8
    se_audit_process_creation_info_t m_se_audit_process_creation_info; // +0x5c0

    // Job and thread management (0x5c0 - 0x5e0)
    list_entry_t m_job_links;                        // +0x5c8
    void* m_highest_user_address;                    // +0x5d8
    list_entry_t m_thread_list_head;                 // +0x5e0

    // Thread and process state (0x5e0 - 0x600)
    volatile std::uint32_t m_active_threads;         // +0x5f0
    std::uint32_t m_image_path_hash;                 // +0x5f4
    std::uint32_t m_default_hard_error_processing;   // +0x5f8
    std::int32_t m_last_thread_exit_status;          // +0x5fc

    // Prefetch and locked pages (0x600 - 0x610)
    ex_fast_ref_t m_prefetch_trace;                  // +0x600
    void* m_locked_pages_list;                       // +0x608

    // Operation counters (0x610 - 0x640)
    large_integer_t m_read_operation_count;          // +0x610
    large_integer_t m_write_operation_count;         // +0x618
    large_integer_t m_other_operation_count;         // +0x620
    large_integer_t m_read_transfer_count;           // +0x628
    large_integer_t m_write_transfer_count;          // +0x630
    large_integer_t m_other_transfer_count;          // +0x638

    // Commit and charge management (0x640 - 0x680)
    std::uint64_t m_commit_charge_limit;             // +0x640
    volatile std::uint64_t m_commit_charge;          // +0x648
    volatile std::uint64_t m_commit_charge_peak;     // +0x650

    // Memory management support (0x680 - 0x7c0)
    mmsupport_full_t m_vm;                           // +0x680
    list_entry_t m_mm_process_links;                 // +0x7c0

    // Timer and virtual timer management (0x970 - 0x990)
    union {
        ps_interlocked_timer_delay_values_t m_process_timer_delay; // +0x970
    };
    volatile std::uint32_t m_k_timer_sets;           // +0x978
    volatile std::uint32_t m_k_timer2_sets;          // +0x97c
    volatile std::uint32_t m_thread_timer_sets;      // +0x980
    std::uint64_t m_virtual_timer_list_lock;         // +0x988
    list_entry_t m_virtual_timer_list_head;          // +0x990

    // Wake and WNF channel (0x9a0 - 0x9d0)
    union {
        wnf_state_name_t m_wake_channel;             // +0x9a0
        ps_process_wake_information_t m_wake_info;   // +0x9a0
    };

    // Mitigation flags (0x9d0 - 0x9d8)
    union {
        std::uint32_t m_mitigation_flags;            // +0x9d0
        struct {
            std::uint32_t m_control_flow_guard_enabled : 1;
            std::uint32_t m_control_flow_guard_export_suppression_enabled : 1;
            std::uint32_t m_control_flow_guard_strict : 1;
            std::uint32_t m_disallow_stripped_images : 1;
            std::uint32_t m_force_relocate_images : 1;
            std::uint32_t m_high_entropy_aslr_enabled : 1;
            std::uint32_t m_stack_randomization_disabled : 1;
            std::uint32_t m_extension_point_disable : 1;
            std::uint32_t m_disable_dynamic_code : 1;
            std::uint32_t m_disable_dynamic_code_allow_opt_out : 1;
            std::uint32_t m_disable_dynamic_code_allow_remote_downgrade : 1;
            std::uint32_t m_audit_disable_dynamic_code : 1;
            std::uint32_t m_disallow_win32k_system_calls : 1;
            std::uint32_t m_audit_disallow_win32k_system_calls : 1;
            std::uint32_t m_enable_filtered_win32k_apis : 1;
            std::uint32_t m_audit_filtered_win32k_apis : 1;
            std::uint32_t m_disable_non_system_fonts : 1;
            std::uint32_t m_audit_non_system_font_loading : 1;
            std::uint32_t m_prefer_system32_images : 1;
            std::uint32_t m_prohibit_remote_image_map : 1;
            std::uint32_t m_audit_prohibit_remote_image_map : 1;
            std::uint32_t m_prohibit_low_il_image_map : 1;
            std::uint32_t m_audit_prohibit_low_il_image_map : 1;
            std::uint32_t m_signature_mitigation_opt_in : 1;
            std::uint32_t m_audit_block_non_microsoft_binaries : 1;
            std::uint32_t m_audit_block_non_microsoft_binaries_allow_store : 1;
            std::uint32_t m_loader_integrity_continuity_enabled : 1;
            std::uint32_t m_audit_loader_integrity_continuity : 1;
            std::uint32_t m_enable_module_tampering_protection : 1;
            std::uint32_t m_enable_module_tampering_protection_no_inherit : 1;
            std::uint32_t m_restrict_indirect_branch_prediction : 1;
            std::uint32_t m_isolate_security_domain : 1;
        } m_mitigation_flags_values;
    };

    // Mitigation flags 2 (0x9d4 - 0x9d8)
    union {
        std::uint32_t m_mitigation_flags2;           // +0x9d4
        struct {
            std::uint32_t m_enable_export_address_filter : 1;
            std::uint32_t m_audit_export_address_filter : 1;
            std::uint32_t m_enable_export_address_filter_plus : 1;
            std::uint32_t m_audit_export_address_filter_plus : 1;
            std::uint32_t m_enable_rop_stack_pivot : 1;
            std::uint32_t m_audit_rop_stack_pivot : 1;
            std::uint32_t m_enable_rop_caller_check : 1;
            std::uint32_t m_audit_rop_caller_check : 1;
            std::uint32_t m_enable_rop_sim_exec : 1;
            std::uint32_t m_audit_rop_sim_exec : 1;
            std::uint32_t m_enable_import_address_filter : 1;
            std::uint32_t m_audit_import_address_filter : 1;
            std::uint32_t m_disable_page_combine : 1;
            std::uint32_t m_speculative_store_bypass_disable : 1;
            std::uint32_t m_cet_user_shadow_stacks : 1;
            std::uint32_t m_audit_cet_user_shadow_stacks : 1;
            std::uint32_t m_audit_cet_user_shadow_stacks_logged : 1;
            std::uint32_t m_user_cet_set_context_ip_validation : 1;
            std::uint32_t m_audit_user_cet_set_context_ip_validation : 1;
            std::uint32_t m_audit_user_cet_set_context_ip_validation_logged : 1;
            std::uint32_t m_cet_user_shadow_stacks_strict_mode : 1;
            std::uint32_t m_block_non_cet_binaries : 1;
            std::uint32_t m_block_non_cet_binaries_non_ehcont : 1;
            std::uint32_t m_audit_block_non_cet_binaries : 1;
            std::uint32_t m_audit_block_non_cet_binaries_logged : 1;
            std::uint32_t m_reserved1 : 1;
            std::uint32_t m_reserved2 : 1;
            std::uint32_t m_reserved3 : 1;
            std::uint32_t m_reserved4 : 1;
            std::uint32_t m_reserved5 : 1;
            std::uint32_t m_cet_dynamic_apis_out_of_proc_only : 1;
            std::uint32_t m_user_cet_set_context_ip_validation_relaxed_mode : 1;
        } m_mitigation_flags2_values;
    };

    // Partition and security domain (0x9d8 - 0x9f0)
    void* m_partition_object;                        // +0x9d8
    std::uint64_t m_security_domain;                 // +0x9e0
    std::uint64_t m_parent_security_domain;          // +0x9e8

    // Coverage and hot patch (0x9f0 - 0xa10)
    void* m_coverage_sampler_context;                // +0x9f0
    void* m_mm_hot_patch_context;                    // +0x9f8
    rtl_avl_tree_t m_dynamic_eh_continuation_targets_tree; // +0xa00
    ex_push_lock_t m_dynamic_eh_continuation_targets_lock; // +0xa08
    ps_dynamic_enforced_address_ranges_t m_dynamic_enforced_cet_compatible_ranges; // +0xa10

    // Disabled components and path redirection (0xa20 - 0xa30)
    std::uint32_t m_disabled_component_flags;        // +0xa20
    std::uint32_t* volatile m_path_redirection_hashes; // +0xa28
}; // Size: 0xa40

struct teb_t {
    nt_tib_t m_nt_tib;                          // 0x000 Contains exception_list, stack_base, stack_limit, etc.
    std::uint64_t m_environment_pointer;           // 0x038
    client_id_t m_client_id;                    // 0x040
    std::uint64_t m_active_rpc_handle;            // 0x050
    std::uint64_t m_thread_local_storage_pointer; // 0x058
    peb_t* m_process_environment_block;         // 0x060
    std::uint32_t m_last_error_value;           // 0x068
    std::uint32_t m_count_of_owned_critical_sections; // 0x06C
    std::uint64_t m_csr_client_thread;            // 0x070
    std::uint64_t m_win32_thread_info;            // 0x078
    std::uint32_t m_user32_reserved[ 26 ];        // 0x080
    std::uint64_t m_user_reserved[ 5 ];             // 0x0E8
    std::uint64_t m_wow32_reserved;               // 0x100
    std::uint32_t m_current_locale;             // 0x108
    std::uint32_t m_fp_software_status_register; // 0x10C
    std::uint64_t m_system_reserved1[ 54 ];         // 0x110
    std::int32_t m_exception_code;              // 0x2C0

    activation_context_stack_t* m_activation_context_stack_pointer; // 0x2C8
    std::uint8_t m_spare_bytes[ 24 ];             // 0x2D0
    std::uint32_t m_tls_slots[ 64 ];              // 0x2E8
    list_entry_t m_tls_links;                   // 0x4E8
};

struct kthread_t {
    // Base section (0x000 - 0x090)
    dispatcher_header_t m_header;                      // +0x000
    void* m_slist_fault_address;                      // +0x018
    std::uint64_t m_quantum_target;                   // +0x020
    void* m_initial_stack;                            // +0x028
    void* m_stack_limit;                              // +0x030
    void* m_stack_base;                              // +0x038
    std::uint64_t m_thread_lock;                     // +0x040
    std::uint64_t m_cycle_time;                      // +0x048
    std::uint32_t m_current_run_time;                // +0x050
    std::uint32_t m_expected_run_time;               // +0x054
    void* m_kernel_stack;                            // +0x058
    void* m_state_save_area;                         // +0x060
    void* m_scheduling_group;                        // +0x068
    kwait_status_register_t m_wait_register;         // +0x070
    std::uint8_t m_running;                          // +0x071
    std::uint8_t m_alerted[ 2 ];                      // +0x072
    std::uint32_t m_auto_alignment;                  // +0x074
    std::uint8_t m_tag;                              // +0x07C
    std::uint8_t m_system_hetero_cpu_policy;         // +0x07D
    std::uint8_t m_spare_byte;                       // +0x07E
    std::uint32_t m_system_call_number;              // +0x080
    std::uint32_t m_ready_time;                      // +0x084
    void* m_first_argument;                          // +0x088
    ktrap_frame_t* m_trap_frame;                     // +0x090

    // Callback and system section (0x098 - 0x0F8)
    kapc_state_t m_apc_state;                          // +0x098
    std::uint8_t m_apc_queueable;                    // +0x0A0
    std::uint8_t m_apc_queueable_padding[ 7 ];         // +0x0A1
    void* m_service_table;                          // +0x0A8
    std::uint32_t m_kernel_reserve_apc;             // +0x0B0
    std::uint32_t m_kernel_reserve_apc_padding;     // +0x0B4
    void* m_win32_thread;                           // +0x0B8
    void* m_trap_frame_base;                        // +0x0C0
    std::uint64_t m_wait_status;                     // +0x0C8
    void* m_wait_block_list;                         // +0x0D0
    void* m_queue;                                   // +0x0E8
    teb_t* m_teb;                                     // +0x0F0
    std::uint64_t m_relative_timer_bias;             // +0x0F8

    // Timer and flags section (0x100 - 0x177)
    ktimer_t m_timer;                                // +0x100
    std::uint32_t m_misc_flags;                      // +0x170
    std::uint8_t m_kernel_apc_disable;               // +0x174
    std::uint8_t m_kernel_apc_pending;               // +0x175
    std::uint8_t m_io_pending;                       // +0x176
    std::uint8_t m_io_pending_high;                  // +0x177

    // Wait info section (0x178 - 0x1FC)
    std::int64_t m_entropy_count;                    // +0x178
    std::uint32_t m_permission_key;                  // +0x180
    std::uint32_t m_permission_key_non_paged;        // +0x184
    void* m_wait_prcb;                              // +0x188
    void* m_wait_next;                              // +0x190
    void* m_wait_value;                             // +0x198
    void* m_wait_reason;                            // +0x1A0
    std::uint32_t m_wait_irql;                      // +0x1A8
    std::uint8_t m_wait_mode;                       // +0x1AC
    std::uint8_t m_wait_next_flag;                  // +0x1AD
    std::uint8_t m_wait_reason_flag;                // +0x1AE
    std::uint8_t m_wait_response;                   // +0x1AF
    void* m_wait_pointer;                          // +0x1B0
    std::uint32_t m_thread_flags;                    // +0x1B8
    std::uint32_t m_spare0;                         // +0x1BC
    void* m_wait_block_list2;                       // +0x1C0
    std::uint32_t m_wait_block_count;               // +0x1C8
    std::uint32_t m_wait_block_offset;              // +0x1CC
    void* m_wait_blocks;                            // +0x1D0
    // Queue and process section (0x1D8 - 0x250)
    list_entry_t m_wait_list_entry;                 // +0x1D8
    std::uint32_t m_wait_status2;                   // +0x1E8
    std::uint32_t m_wait_state_sequence;            // +0x1EC
    std::uint32_t m_wait_irql_old;                  // +0x1F0
    std::uint32_t m_wait_mode_old;                  // +0x1F4
    std::uint32_t m_wait_queue_timeout;             // +0x1F8
    std::uint32_t m_wait_block_multiple;            // +0x1FC
    void* m_thread_queue_list_entry;                // +0x200
    list_entry_t m_queue_list_entry;                // +0x208
    std::uint16_t m_queue_index;                    // +0x218
    std::uint32_t m_queue_priority;                 // +0x21C
    kprocess_t* m_process;                          // +0x220
    group_affinity_t m_affinity;                    // +0x228
    std::uint64_t m_affinity_version;               // +0x238
    void* m_npx_state;                             // +0x250

    // Performance and kernel stack section (0x258 - 0x2E0)
    void* m_performance_counters;                   // +0x258
    void* m_context_switch_count;                   // +0x260
    void* m_scheduler_assist_thread;                // +0x268
    void* m_kernel_stack_control;                   // +0x270
    void* m_kernel_stack_limit;                     // +0x278
    void* m_kernel_stack_base;                      // +0x280
    void* m_thread_lock_owner;                      // +0x288
    void* m_kernel_wait_always;                     // +0x290
    void* m_user_wait_always;                       // +0x298
    void* m_win32k_thread;                         // +0x2A0
    void* m_worker_routine;                        // +0x2A8
    void* m_worker_context;                        // +0x2B0
    void* m_win32_start_address;                   // +0x2B8
    void* m_lpaccel;                              // +0x2C0
    void* m_lpfnwndproc;                          // +0x2C8
    void* m_win32k_callback;                      // +0x2D0
    void* m_win32k_callback_context;               // +0x2D8

    // Final section (0x2E0 - 0x430)
    kevent_t m_suspend_event;                       // +0x2E0
    list_entry_t m_thread_list_entry;               // +0x2F8
    list_entry_t m_mutant_list_head;                // +0x308
    std::uint8_t m_ab_entry_summary;                // +0x318
    std::uint8_t m_ab_wait_entry_count;             // +0x319
    std::uint8_t m_ab_allocation_region_count;      // +0x31A
    std::uint8_t m_system_priority;                 // +0x31B
    std::uint32_t m_secure_thread_cookie;           // +0x31C
    void* m_lock_entries;                          // +0x320
    single_list_entry_t m_propagate_boosts_entry;   // +0x328
    single_list_entry_t m_io_self_boosts_entry;     // +0x330
    std::uint8_t m_priority_floor_counts[ 16 ];       // +0x338
    std::uint8_t m_priority_floor_counts_reserved[ 16 ]; // +0x348
    std::uint32_t m_priority_floor_summary;         // +0x358
    std::uint32_t m_ab_completed_io_boost_count;    // +0x35C
    std::uint32_t m_ab_completed_io_qos_boost_count; // +0x360
    std::uint16_t m_ke_reference_count;             // +0x364
    std::uint8_t m_ab_orphaned_entry_summary;       // +0x366
    std::uint8_t m_ab_owned_entry_count;            // +0x367
    std::uint32_t m_foreground_loss_time;           // +0x368
    std::uint64_t m_read_operation_count;           // +0x380
    std::uint64_t m_write_operation_count;          // +0x388
    std::uint64_t m_other_operation_count;          // +0x390
    std::uint64_t m_read_transfer_count;            // +0x398
    std::uint64_t m_write_transfer_count;           // +0x3A0
    std::uint64_t m_other_transfer_count;           // +0x3A8
    void* m_queued_scb;                            // +0x3B0
    std::uint32_t m_thread_timer_delay;             // +0x3B8
    void* m_tracing_private;                        // +0x3C0
    void* m_scheduler_assist;                       // +0x3C8
    void* m_ab_wait_object;                        // +0x3D0
    std::uint32_t m_reserved_previous_ready_time_value; // +0x3D8
    std::uint64_t m_kernel_wait_time;               // +0x3E0
    std::uint64_t m_user_wait_time;                 // +0x3E8
    void* m_explicit_scheduling;                    // +0x3F0
    void* m_debug_active;                          // +0x3F8
    std::uint32_t m_scheduler_assist_priority_floor; // +0x400
    std::uint32_t m_spare28;                       // +0x404
    std::uint8_t m_resource_index;                  // +0x408
    std::uint8_t m_spare31[ 3 ];                     // +0x409
    std::uint64_t m_end_padding[ 4 ];                // +0x410
}; // Size: 0x430


struct ethread_t {
    struct kthread_t m_kthread;              // +0x000 - KTHREAD portion
    list_entry_t m_thread_list_entry;                // +0x430
    void* m_create_time;                             // +0x440
    union {
        void* m_exit_time;                           // +0x448
        list_entry_t m_active_execution_list;
    };
    union {
        void* m_exit_status;                         // +0x450
        void* m_post_block_old;
    };
    union {
        void* m_terminate_apc;                       // +0x458
        void* m_terminate_pending;
    };
    union {
        void* m_thread_flags;                        // +0x460
        union {
            std::uint32_t m_thread_flags_value;
            struct {
                std::uint32_t m_terminate_requested : 1;
                std::uint32_t m_dead_thread : 1;
                std::uint32_t m_hide_from_debugger : 1;
                std::uint32_t m_active_impersonation_info : 1;
                std::uint32_t m_system_thread : 1;
                std::uint32_t m_hard_errors_are_disabled : 1;
                std::uint32_t m_break_on_termination : 1;
                std::uint32_t m_skip_creation_msg : 1;
                std::uint32_t m_skip_termination_msg : 1;
            };
        };
        std::uint32_t m_padding;
    };
    union {
        void* m_create_info;                         // +0x468
        void* m_win32_start_address;
    };
    void* m_teb;                                     // +0x470
    client_id_t m_client_id;                       // +0x478
    void* m_security_port;                           // +0x480
    void* m_previous_mode;                           // +0x488
    void* m_resource_index;                          // +0x490
    void* m_large_stack;                             // +0x498
    void* m_fx_save_area;                           // +0x4A0
    void* m_priority_class;                          // +0x4A8
    void* m_desktop;                                 // +0x4B0
    void* m_suspend_semaphore;                       // +0x4B8
    union {
        void* m_win32_thread;                        // +0x4C0
        struct {
            std::uint32_t m_io_priority : 3;
            std::uint32_t m_memory_priority : 3;
            std::uint32_t m_absolute_cpu_priority : 1;
        };
    };
    void* m_working_on_behalf_ticket;                // +0x4C8
    void* m_impersonation_info;                      // +0x4D0
    void* m_io_pending_mr;                           // +0x4D8
    void* m_io_suppress_thread;                      // +0x4E0
    void* m_memory_attribute;                         // +0x4E8
    union {
        void* m_win32_thread_event;                  // +0x4F0
        void* m_running_down;
    };
    void* m_thread_lock;                             // +0x4F8
    std::uint32_t m_read_operation_count;            // +0x500
    std::uint32_t m_write_operation_count;           // +0x504
    std::uint32_t m_other_operation_count;           // +0x508
    std::uint32_t m_io_priority_boost;               // +0x50C
    void* m_io_client_pointer;                       // +0x510
    void* m_file_object;                             // +0x518
    void* m_word_list_head;                          // +0x520
    void* m_process_context;                         // +0x528
    void* m_granted_access;                          // +0x530
    void* m_cross_thread_flags;                      // +0x538
    union {
        std::uint32_t m_cross_thread_flags_uint;     // +0x540
        struct {
            std::uint32_t m_terminated : 1;
            std::uint32_t m_debug_active : 1;
            std::uint32_t m_system_process : 1;
            std::uint32_t m_impersonating : 1;
            std::uint32_t m_break_on_termination : 1;
            std::uint32_t m_reserved : 27;
        };
    };
    std::uint32_t m_cross_thread_flags_padding;      // +0x544
    void* m_start_address;                           // +0x548
    void* m_win32_thread_info;                       // +0x550
    void* m_lpaccel;                                 // +0x558
    void* m_lpfnwndproc;                            // +0x560
    void* m_win32k;                                  // +0x568
}; // Size = 0x570

typedef struct hash_bucket_entry_t {
    struct hash_bucket_entry_t* m_next;
    unicode_string_t m_driver_name;
    std::uint32_t m_cert_mash[ 5 ];
};

struct balanced_links_t {
    void* m_parent;             // 0x00
    void* m_left;               // 0x08
    void* m_right;              // 0x10
    std::uint8_t m_balance;     // 0x18
    std::uint8_t m_reserved[ 3 ]; // 0x19-0x1B
    std::uint32_t m_pad;        // 0x1C
}; // size: 0x20

struct pool_tracker_big_pages_t {
    volatile std::uint64_t m_va;
    std::uint32_t m_key;
    std::uint32_t m_pattern : 8;
    std::uint32_t m_pool_type : 12;
    std::uint32_t m_slush_size : 12;
    std::uint64_t m_number_of_bytes;
};

struct avl_table_t {
    balanced_links_t m_balanced_root;    // 0x00-0x20
    void* m_ordered_pointer;             // 0x20
    std::uint32_t m_which_ordered_element; // 0x28
    std::uint32_t m_number_generic_table_elements; // 0x2C
    std::uint32_t m_depth_of_tree;       // 0x30
    std::uint32_t m_pad1;                // 0x34
    void* m_restart_key;                 // 0x38
    std::uint32_t m_delete_count;        // 0x40
    std::uint32_t m_pad2;                // 0x44
    void* m_compare_routine;             // 0x48
    void* m_allocate_routine;            // 0x50
    void* m_free_routine;                // 0x58
    void* m_table_context;               // 0x60
}; // size: 0x68

struct ldr_data_table_entry_t {
    list_entry_t m_in_load_order_module_list;
    list_entry_t m_in_memory_order_module_list;
    list_entry_t m_in_initialization_order_module_list;
    void* m_dll_base;
    void* m_entry_point;
    std::uint32_t m_size_of_image;
    unicode_string_t m_full_dll_name;
    unicode_string_t m_base_dll_name;
    std::uint32_t m_flags;
    std::uint16_t m_load_count;
    std::uint16_t m_tls_index;
    list_entry_t m_hash_links;
    void* m_section_pointer;
    std::uint32_t m_check_sum;
    std::uint32_t m_time_date_stamp;
};

struct kldr_data_table_entry_t {
    list_entry_t m_in_load_order_links;                  // +0x00
    void* m_exception_table;                            // +0x10
    std::uint32_t m_exception_table_size;               // +0x18
    void* m_gp_value;                                   // +0x20
    void* m_non_paged_debug_info;                       // +0x28
    void* m_dll_base;                                   // +0x30
    void* m_entry_point;                                // +0x38
    std::uint32_t m_size_of_image;                      // +0x40
    unicode_string_t m_full_dll_name;                   // +0x48
    unicode_string_t m_base_dll_name;                   // +0x58
    std::uint32_t m_flags;                              // +0x68
    std::uint16_t m_load_count;                         // +0x6C
    union {
        struct {
            std::uint16_t m_signature_level : 4;        // +0x6E
            std::uint16_t m_signature_type : 3;         // +0x6E
            std::uint16_t m_frozen : 2;                 // +0x6E
            std::uint16_t m_hot_patch : 1;              // +0x6E
            std::uint16_t m_unused : 6;                 // +0x6E
        };
        std::uint16_t m_entire_field;                   // +0x6E
    } u1;                                               // +0x6E
    void* m_section_pointer;                            // +0x70
    std::uint32_t m_check_sum;                          // +0x78
    std::uint32_t m_coverage_section_size;              // +0x7C
    void* m_coverage_section;                           // +0x80
    void* m_loaded_imports;                             // +0x88
    union {
        void* m_spare;                                  // +0x90
        kldr_data_table_entry_t* m_nt_data_table_entry; // +0x90
    };
    std::uint32_t m_size_of_image_not_rounded;          // +0x98
    std::uint32_t m_time_date_stamp;                    // +0x9C
}; // Size: 0xA0

struct apic_icr {
    union {
        uint64_t m_raw; // Full 64-bit ICR value
        struct {
            uint32_t m_vector : 8;            // Interrupt vector (0x00 to 0xFF)
            uint32_t m_delivery_mode : 3;     // Delivery mode (e.g., fixed = 0)
            uint32_t m_destination_mode : 1;  // Destination mode (0 = physical, 1 = logical)
            uint32_t m_delivery_status : 1;   // Delivery status (0 = idle, 1 = send pending)
            uint32_t m_reserved1 : 1;         // Reserved (set to 0)
            uint32_t m_level : 1;             // Level (0 = deassert, 1 = assert)
            uint32_t m_trigger_mode : 1;      // Trigger mode (0 = edge, 1 = level)
            uint32_t m_reserved2 : 2;         // Reserved (set to 0)
            uint32_t m_destination_shorthand : 2; // Destination shorthand (e.g., self = 1)
            uint32_t m_reserved3 : 12;        // Reserved (set to 0)

            uint32_t m_reserved4 : 24;        // Reserved (set to 0)
            uint32_t m_destination : 8;       // Destination processor (physical mode)
        } m_fields;
    };
};

struct idt_entry_t {
    uint16_t m_offset_low;
    uint16_t m_selector;
    uint8_t m_ist;
    uint8_t m_attributes;
    uint16_t m_offset_middle;
    uint32_t m_offset_high;
    uint32_t m_reserved;
};

struct physical_address_t {
    union {
        struct {
            std::uint32_t m_low_part;      // +0x000
            std::int32_t m_high_part;      // +0x004
        };
        struct {
            std::uint64_t m_quad_part;     // +0x000
        };
    };
}; // Size: 0x008

struct mm_copy_address_t {
    union {
        std::uint64_t m_virtual_address;    // +0x000
        physical_address_t m_physical_address;   // +0x000
    };
}; // Size: 0x008

struct eresource_t {
    struct {
        void* m_flink;             // 0x00
        void* m_blink;             // 0x08
    } m_system_resource_list;      // 0x00-0x10

    void* m_owner_table;           // 0x10
    std::uint16_t m_active_count;  // 0x18
    union {
        std::uint16_t m_flag;      // 0x1A
        struct {
            std::uint8_t m_shared_wait_count;   // 0x1A
            std::uint8_t m_exclusive_wait_count; // 0x1B
        };
    };
    std::uint32_t m_pad1;          // 0x1C

    void* m_shared_waiters;        // 0x20
    void* m_exclusive_waiters;     // 0x28

    struct {
        void* m_owner_thread;      // 0x30
        void* m_owner_count;       // 0x38
    } m_owner_entry;               // 0x30-0x40

    std::uint32_t m_active_entries;     // 0x40
    std::uint32_t m_contention_count;   // 0x44
    std::uint32_t m_shared_waiter_count;    // 0x48
    std::uint32_t m_exclusive_waiter_count; // 0x4C

    std::uint8_t m_misc_flags;          // 0x50
    std::uint8_t m_reserved[ 3 ];         // 0x51-0x53
    std::uint32_t m_timeout_count;      // 0x54

    union {
        void* m_descriptor;             // 0x58
        std::uint32_t m_converted_type; // 0x58
    };

    std::uint64_t m_spin_lock;         // 0x60
}; // size: 0x68

struct piddb_cache_entry_t {
    list_entry_t m_list;           // 0x00-0x10
    unicode_string_t m_driver_name;  // 0x10-0x1C
    std::uint32_t m_timestamp;     // 0x20
    std::int32_t m_load_status;    // 0x24
    std::uint8_t m_shim_data[ 16 ];// 0x28-0x38
}; // size: 0x38


struct rtl_critical_section_t {
    void* m_debug_info;
    std::int32_t m_lock_count;
    std::int32_t m_recursion_count;
    void* m_owning_thread;
    void* m_lock_semaphore;
    std::uint32_t m_spin_count;
};

struct peb_ldr_data_t {
    std::uint32_t m_length;
    bool m_initialized;
    void* m_ss_handle;
    list_entry_t m_module_list_load_order;
    list_entry_t m_module_list_memory_order;
    list_entry_t m_module_list_in_it_order;
};

struct peb_t {
    std::uint8_t m_inherited_address_space;
    std::uint8_t m_read_image_file_exec_options;
    std::uint8_t m_being_debugged;
    std::uint8_t m_bit_field;

    struct {
        std::uint32_t m_image_uses_large_pages : 1;
        std::uint32_t m_is_protected_process : 1;
        std::uint32_t m_is_legacy_process : 1;
        std::uint32_t m_is_image_dynamically_relocated : 1;
        std::uint32_t m_spare_bits : 4;
    };

    void* m_mutant;
    void* m_image_base_address;
    peb_ldr_data_t* m_ldr;
    void* m_process_parameters;
    void* m_subsystem_data;
    void* m_process_heap;
    rtl_critical_section_t* m_fast_peb_lock;
    void* m_atl_thunk_slist_ptr;
    void* m_ifeo_key;

    struct {
        std::uint32_t m_process_in_job : 1;
        std::uint32_t m_process_initializing : 1;
        std::uint32_t m_reserved_bits0 : 30;
    } m_cross_process_flags;

    union {
        void* m_kernel_callback_table;
        void* m_user_shared_info_ptr;
    };

    std::uint32_t m_system_reserved[ 1 ];
    std::uint32_t m_spare_ulong;
    void* m_free_list;
    std::uint32_t m_tls_expansion_counter;
    void* m_tls_bitmap;
    std::uint32_t m_tls_bitmap_bits[ 2 ];
    void* m_read_only_shared_memory_base;
    void* m_hotpatch_information;
    void** m_read_only_static_server_data;
    void* m_ansi_code_page_data;
    void* m_oem_code_page_data;
    void* m_unicode_case_table_data;
    std::uint32_t m_number_of_processors;
    std::uint32_t m_nt_global_flag;
    std::int64_t m_critical_section_timeout;
    std::uint32_t m_heap_segment_reserve;
    std::uint32_t m_heap_segment_commit;
    std::uint32_t m_heap_decomit_total_free_threshold;
    std::uint32_t m_heap_decomit_free_block_threshold;
    std::uint32_t m_number_of_heaps;
    std::uint32_t m_maximum_number_of_heaps;
    void** m_process_heaps;
    void* m_gdi_shared_handle_table;
    void* m_process_starter_helper;
    std::uint32_t m_gdi_dc_attribute_list;
    rtl_critical_section_t* m_loader_lock;
    std::uint32_t m_os_major_version;
    std::uint32_t m_os_minor_version;
    std::uint16_t m_os_build_number;
    std::uint16_t m_os_csd_version;
    std::uint32_t m_os_platform_id;
    std::uint32_t m_image_subsystem;
    std::uint32_t m_image_subsystem_major_version;
    std::uint32_t m_image_subsystem_minor_version;
    std::uint32_t m_image_process_affinity_mask;
    std::uint32_t m_gdi_handle_buffer[ 34 ];
    void* m_post_process_init_routine;
    void* m_tls_expansion_bitmap;
    std::uint32_t m_tls_expansion_bitmap_bits[ 32 ];
    std::uint32_t m_session_id;
    std::uint64_t m_app_compat_flags;
    std::uint64_t m_app_compat_flags_user;
    void* m_p_shim_data;
    void* m_app_compat_info;
    unicode_string_t m_csd_version;
    void* m_activation_context_data;
    void* m_process_assembly_storage_map;
    void* m_system_default_activation_context_data;
    void* m_system_assembly_storage_map;
    std::uint32_t m_minimum_stack_commit;
    void* m_fls_callback;
    list_entry_t m_fls_list_head;
    void* m_fls_bitmap;
    std::uint32_t m_fls_bitmap_bits[ 4 ];
    std::uint32_t m_fls_high_index;
    void* m_wer_registration_data;
    void* m_wer_ship_assert_ptr;
    void* m_context_data;
    void* m_image_header_hash;
    union {
        ULONG m_tracking_flags;
        struct {
            ULONG m_heap_tracking_enabled : 1;
            ULONG m_crit_sec_tracking_enabled : 1;
            ULONG m_lib_loader_tracking_enabled : 1;
            ULONG m_spare_tracking_enabled : 29;
        };
    };
    ULONGLONG m_csr_server_read_only_shared_memory_base;
    void* m_tpp_worker_list;
    void* m_api_set_map;
};

struct mmpfnentry1_t {
    std::uint8_t m_page_location : 3;
    std::uint8_t m_write_in_progress : 1;
    std::uint8_t m_modified : 1;
    std::uint8_t m_read_in_progress : 1;
    std::uint8_t m_cache_attribute : 2;
};

struct mmpfnentry3_t {
    std::uint8_t m_priority : 1;
    std::uint8_t m_on_protected_standby : 1;
    std::uint8_t m_in_page_error : 1;
    std::uint8_t m_system_charged_page : 1;
    std::uint8_t m_removal_requested : 1;
    std::uint8_t m_rarity_error : 1;
};

struct mi_pfn_ulong5_t {
    union {
        struct {
            std::uint32_t m_modified_write_count : 16;
            std::uint32_t m_shared_count : 16;
        };
        std::uint32_t m_entire_field;
    };
};

struct mipfnblink_t {
    union {
        struct 
        {
            std::uint64_t m_blink : 36;                                             
            std::uint64_t m_node_blink_high : 20;
            std::uint64_t m_tb_flush_stamp : 4;
            std::uint64_t m_unused : 2;
            std::uint64_t m_sage_blink_delete_bit : 1;
            std::uint64_t m_page_blink_lock_bit : 1;
            std::uint64_t m_share_count : 62;
            std::uint64_t m_page_share_count_delete_bit : 1;
            std::uint64_t m_page_share_count_lock_bit : 1;
        };

        std::uint64_t m_entire_field;                                              
        volatile std::uint64_t m_lock;                                             
        struct {
            std::uint64_t m_lock_not_used : 62;                                       
            std::uint64_t m_delete_bit : 1;                                          
            std::uint64_t m_lock_bit : 1;                                            
        };
    };
};

struct rtl_balanced_node_t {
    union {
        rtl_balanced_node_t* m_children[ 2 ];
        struct {
            rtl_balanced_node_t* m_left;
            rtl_balanced_node_t* m_right;
        };
    };
    union {
        std::uint8_t m_red : 1;
        rtl_balanced_node_t* m_parent;
        std::uint64_t m_value;
    };
};

struct mi_active_pfn_t {
    union {
        struct {
            std::uint64_t m_tradable : 1;                                           //0x0
            std::uint64_t m_non_paged_buddy : 43;                                     //0x0
        } m_leaf;                                                             //0x0
        struct {
            std::uint64_t m_tradable : 1;                                           //0x0
            std::uint64_t m_wsle_sge : 3;                                            //0x0
            std::uint64_t m_oldest_wsle_leaf_entries : 10;                             //0x0
            std::uint64_t m_oldest_wsle_leaf_age : 3;                                  //0x0
            std::uint64_t m_non_paged_buddy : 43;                                     //0x0
        } m_page_table;                                                        //0x0
        std::uint64_t m_entire_active_field;                                        //0x0
    };
};

struct mmpte_hardware_t {
    std::uint64_t m_valid : 1;
    std::uint64_t m_write : 1;
    std::uint64_t m_owner : 1;
    std::uint64_t m_write_through : 1;
    std::uint64_t m_cache_disable : 1;
    std::uint64_t m_accessed : 1;
    std::uint64_t m_dirty : 1;
    std::uint64_t m_large_page : 1;
    std::uint64_t m_global : 1;
    std::uint64_t m_copy_on_write : 1;
    std::uint64_t m_prototype : 1;
    std::uint64_t m_reserved0 : 1;
    std::uint64_t m_page_frame_number : 36;
    std::uint64_t m_reserved1 : 4;
    std::uint64_t m_software_ws_index : 11;
    std::uint64_t m_no_execute : 1;
};

//struct mmpte_prototype_t;
//struct mmpte_software_t;
//struct mmpte_timestamp_t;
//struct mmpte_transition_t;
//struct mmpte_subsection_t;
//struct mmpte_list_t;

struct mmpte_t {
    union {
        std::uint64_t m_long;                                                     
        volatile std::uint64_t m_volatile_long;
        struct mmpte_hardware_t m_hard;
        //struct mmpte_prototype_t m_proto;
        //struct mmpte_software_t m_soft;
        //struct mmpte_timestamp_t m_timeStamp;
        //struct mmpte_transition_t m_trans;
        //struct mmpte_subsection_t m_subsect;
        //struct mmpte_list_t m_list;
    };
};

struct mmpfn_t {
    union {
        list_entry_t m_list_entry;
        rtl_balanced_node_t m_tree_node;
        struct {
            union {
                union {
                    single_list_entry_t m_next_slist_pfn;
                    void* m_next;
                    std::uint64_t m_flink : 38;
                    std::uint64_t m_node_flink_low : 28;
                    mi_active_pfn_t m_active;
                } m_u1;

                union {
                    mmpte_t* m_pte_address;
                    std::uint64_t m_pte_long;
                };

                mmpte_t m_original_pte;
            };
        };
    };

    mipfnblink_t m_u2;

    union {
        struct {
            std::uint16_t m_reference_count;
            mmpfnentry1_t m_e1;
        };
        struct {
            mmpfnentry3_t m_e3;
            struct {
                std::uint16_t m_reference_count;
            } m_e2;
        };
        struct {
            std::uint16_t m_reference_count;
            mmpfnentry1_t m_e1;
        } m_e4;
    } m_u3;

    std::uint16_t m_node_blink_low;
    std::uint16_t m_unused : 4;
    std::uint16_t m_unused2 : 4;

    union {
        std::uint16_t m_view_count;
        std::uint16_t m_node_flink_low;

        struct {
            std::uint16_t m_modified_list_bucket_index : 4;
            std::uint16_t m_anchor_large_page_size : 2;
        } m_e2;
    };

    mi_pfn_ulong5_t m_u5;

    union {
        std::uint64_t m_pte_frame : 36;
        std::uint64_t m_resident_page : 1;
        std::uint64_t m_unused1 : 1;
        std::uint64_t m_unused2 : 1;
        std::uint64_t m_partition : 10;
        std::uint64_t m_file_only : 1;
        std::uint64_t m_pfn_exists : 1;
        std::uint64_t m_node_flink_high : 5;
        std::uint64_t m_page_identity : 3;
        std::uint64_t m_prototype_pte : 1;
        std::uint64_t m_entire_field;
    } m_u4;
};

struct physical_memory_range_t {
    large_integer_t m_base_page;
    large_integer_t m_page_count;
}; // Size: 0x010

struct system_handle_table_entry_info_t {
    void* m_object;
    HANDLE m_unique_process_id;
    HANDLE m_handle_value;
	std::uint32_t m_granted_access;
    std::uint8_t m_creator_back_trace_index;
	std::uint8_t m_object_type_index;
    std::uint64_t m_handle_attributes;
    std::uint64_t m_reserved;
};

struct system_handle_information_t {
	std::uint64_t m_number_of_handles;
	std::uint64_t m_reserved;
    system_handle_table_entry_info_t m_handles[ 1 ];
};

typedef struct {

    uint16_t machine, number_of_sections;
    uint32_t time_stamp;
    uint32_t symbol_table, number_of_symbols;
    uint16_t optional_head_sz, characts;

} nt_image_file_head_t;

typedef struct {

    uint8_t name[ 8 ];

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

    nt_image_data_dir_t data_directory[ image_dir_entries ];

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
    uint16_t reserved[ 4 ];
    uint16_t oem_id;
    uint16_t oem_info;
    uint16_t reserved2[ 10 ];
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
    char		name[ 1 ];

} nt_image_import_name_t;

typedef struct {

    uint32_t					signature;
    nt_image_file_head_t		file;
    nt_image_optional_head_t	optional;

} nt_image_headers_t;

typedef struct {

    uint32_t va, size;

} nt_image_base_reloc_t;

struct rtl_avl_tree_node_t {
    void* left;
    void* right;
    void* parent_and_balance;
};

struct clone_descriptor_t {
    rtl_avl_tree_node_t node;
    std::uint64_t reserved1;
    std::uint64_t generation;
    std::uint64_t count;
    std::uint64_t* data;
    std::uint64_t size;
    std::uint64_t reserved2[ 5 ];
};