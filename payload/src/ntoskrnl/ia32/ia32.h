#pragma once
struct driver_object_t;
struct device_object_t;
struct _ACCESS_STATE;
typedef struct _ACCESS_STATE* PACCESS_STATE;

typedef
_IRQL_requires_same_
_Function_class_( kstart_routine )
void
kstart_routine(
    _In_ void* start_context
);
typedef kstart_routine* pkstart_routine;

typedef union segment_selector_t {
    std::uint16_t value;

    struct {
        std::uint16_t rpl : 2;
        std::uint16_t ti : 1;
        std::uint16_t index : 13;
    };
};

typedef union cr4_t {
    std::uint64_t value;
    struct {
        std::uint64_t vme : 1;
        std::uint64_t pvi : 1;
        std::uint64_t tsd : 1;
        std::uint64_t de : 1;
        std::uint64_t pse : 1;
        std::uint64_t pae : 1;
        std::uint64_t mce : 1;
        std::uint64_t pge : 1;
        std::uint64_t pce : 1;
        std::uint64_t osfxsr : 1;
        std::uint64_t osxmmexcpt : 1;
        std::uint64_t umip : 1;
        std::uint64_t la57 : 1;
        std::uint64_t vmxe : 1;
        std::uint64_t smxe : 1;
        std::uint64_t reserved1 : 1;
        std::uint64_t fsgsbase : 1;
        std::uint64_t pcide : 1;
        std::uint64_t osxsave : 1;
        std::uint64_t kl : 1;
        std::uint64_t smep : 1;
        std::uint64_t smap : 1;
        std::uint64_t pke : 1;
        std::uint64_t cet : 1;
        std::uint64_t pks : 1;
        std::uint64_t reserved2 : 39;
    };
};

typedef union rflags_t {
    std::uint64_t value;

    struct {
        std::uint64_t cf : 1;
        std::uint64_t reserved1 : 1; /* Always 1 */
        std::uint64_t pf : 1;
        std::uint64_t reserved2 : 1; /* Always 0 */
        std::uint64_t af : 1;
        std::uint64_t reserved3 : 1; /* Always 0 */
        std::uint64_t zf : 1;
        std::uint64_t sf : 1;
        std::uint64_t tf : 1;
        std::uint64_t intf : 1;
        std::uint64_t df : 1;
        std::uint64_t of : 1;
        std::uint64_t iopl : 2;
        std::uint64_t nt : 1;
        std::uint64_t reserved4 : 1; /* Always 0 */
        std::uint64_t rf : 1;
        std::uint64_t vm : 1;
        std::uint64_t ac : 1;
        std::uint64_t vif : 1;
        std::uint64_t vip : 1;
        std::uint64_t id : 1;
        std::uint64_t reserved5 : 42; /* Always 0 */
    };
};

struct idtr_t {
    std::uint16_t limit;
    std::uint64_t base;
};
typedef idtr_t gdtr_t;

typedef union segment_descriptor32_t {
    std::uint64_t value;
    struct {
        std::uint64_t segment_limit_low : 16;
        std::uint64_t base_low : 16;
        std::uint64_t base_mid : 8;
        std::uint64_t type : 4;
        std::uint64_t descriptor_type : 1;
        std::uint64_t dpl : 2;
        std::uint64_t p : 1;
        std::uint64_t segment_limit_high : 4;
        std::uint64_t system : 1;
        std::uint64_t long_mode : 1;
        std::uint64_t default_big : 1;
        std::uint64_t granularity : 1;
        std::uint64_t base_high : 8;
    };
};

typedef union segment_descriptor64_t {
    std::uint64_t value[ 2 ];
    struct {
        segment_descriptor32_t desc;
        std::uint32_t base_upper;
        std::uint32_t set_zero;
    };
};

#pragma pack(push, 1)
typedef struct {
    uint16_t limit;
    uint64_t base_address;
} segment_descriptor_register64_t;
#pragma pack(pop)

typedef struct {
    LONG* m_service_table;
    void* m_counter_table;
    std::uint64_t m_number_of_services;
    PCHAR m_argument_table;
} service_descriptor_table_t;

union idt_entry_t {
    std::uint32_t value[ 4 ];

    struct {
        std::uint16_t base_low;
        std::uint16_t segment_selector;
        std::uint8_t ist : 3;
        std::uint8_t reserved1 : 5;
        std::uint8_t type : 4;
        std::uint8_t reserved2 : 1;
        std::uint8_t dpl : 2;
        std::uint8_t present : 1;
        std::uint16_t base_middle;
        std::uint32_t base_high;
        std::uint32_t reserved3;
    };
};

typedef union _virt_addr_t {
    std::uintptr_t value;
    struct {
        std::uint64_t offset : 12;        // 0:11
        std::uint64_t pte_index : 9;      // 12:20
        std::uint64_t pde_index : 9;      // 21:29
        std::uint64_t pdpte_index : 9;    // 30:38
        std::uint64_t pml4e_index : 9;    // 39:47
        std::uint64_t reserved : 16;      // 48:63
    };
} virt_addr_t, * pvirt_addr_t;

typedef union _pfn_t {
    std::uintptr_t value;
    std::uintptr_t offset;
} pfn_t, * ppfn_t;

typedef union _pml4e {
    struct {
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
    std::uint64_t value;
} pml4e, * ppml4e;

typedef union _pdpte {
    struct {
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
    std::uint64_t value;
} pdpte, * ppdpte;

typedef union _pde {
    struct {
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
    std::uint64_t value;
} pde, * ppde;

typedef union _pte {
    struct {
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
    std::uint64_t value;
} pte, * ppte;

typedef union cr3 {
    std::uint64_t flags;
    struct {
        std::uint64_t reserved1 : 3;
        std::uint64_t page_level_write_through : 1;
        std::uint64_t page_level_cache_disable : 1;
        std::uint64_t reserved2 : 7;
        std::uint64_t dirbase : 36;
        std::uint64_t reserved3 : 16;
    };
};

struct pg_scan_worker_context {
    std::uint64_t m_encoded_routine;
    std::uint64_t m_encoded_pg_context;
    std::uint64_t m_xor_key;
};

struct pg_context_t {
    std::uint8_t m_cmp_append_dll_section[ 192 ];           // +0x000
    std::uint32_t m_unknown;                              // +0x0C0
    std::uint32_t m_context_size_in_qword;                // +0x0C4
    std::uint64_t m_ex_acquire_resource_shared_lite;      // +0x0C8
    std::uint64_t m_ex_acquire_resource_exclusive_lite;   // +0x0D0
    std::uint64_t m_ex_allocate_pool_with_tag;            // +0x0D8
    std::uint64_t m_ex_free_pool;                         // +0x0E0
    std::uint64_t m_ex_map_handle_to_pointer;             // +0x0E8
    std::uint64_t m_ex_queue_work_item;                   // +0x0F0
    std::uint64_t m_ex_release_resource_lite;             // +0x0F8
    std::uint64_t m_ex_unlock_handle_table_entry;         // +0x100
    std::uint64_t m_exf_acquire_push_lock_exclusive;      // +0x108
    std::uint64_t m_exf_release_push_lock_exclusive;      // +0x110
    std::uint64_t m_exf_acquire_push_lock_shared;         // +0x118
    std::uint64_t m_exf_release_push_lock_shared;         // +0x120
    std::uint64_t m_ke_acquire_in_stack_queued_spin_lock_at_dpc_level; // +0x128
    std::uint64_t m_ex_acquire_spin_lock_shared_at_dpc_level;          // +0x130
    std::uint64_t m_ke_bug_check_ex;                      // +0x138
    std::uint64_t m_ke_delay_execution_thread;            // +0x140
    std::uint64_t m_ke_enter_critical_region_thread;      // +0x148
    std::uint64_t m_ke_leave_critical_region;             // +0x150
    std::uint64_t m_ke_enter_guarded_region;              // +0x158
    std::uint64_t m_ke_leave_guarded_region;              // +0x160
    std::uint64_t m_ke_release_in_stack_queued_spin_lock_from_dpc_level; // +0x168
    std::uint64_t m_ex_release_spin_lock_shared_from_dpc_level;         // +0x170
    std::uint64_t m_ke_revert_to_user_affinity_thread;    // +0x178
    std::uint64_t m_ke_processor_group_affinity;          // +0x180
    std::uint64_t m_ke_set_system_group_affinity_thread;  // +0x188
    std::uint64_t m_ke_set_coalescable_timer;             // +0x190
    std::uint64_t m_obf_dereference_object;               // +0x198
    std::uint64_t m_ob_reference_object_by_name;          // +0x1A0
    std::uint64_t m_rtl_image_directory_entry_to_data;    // +0x1A8
    std::uint64_t m_rtl_image_nt_header;                  // +0x1B0
    std::uint64_t m_rtl_lookup_function_table;            // +0x1B8
    std::uint64_t m_rtl_pc_to_file_header;                // +0x1C0
    std::uint64_t m_rtl_section_table_from_virtual_address; // +0x1C8
    std::uint64_t m_dbg_print;                            // +0x1D0
    std::uint64_t m_mm_allocate_independent_pages;        // +0x1D8
    std::uint64_t m_mm_free_independent_pages;            // +0x1E0
    std::uint64_t m_mm_set_page_protection;               // +0x1E8
    std::uint64_t m_unknown1;                             // +0x1F0
    std::uint64_t m_unknown2;                             // +0x1F8
    std::uint64_t m_unknown3;                             // +0x200
    std::uint64_t m_unknown4;                             // +0x208
    std::uint64_t m_rtl_lookup_function_entry;            // +0x210
    std::uint64_t m_ke_acquire_spin_lock_raise_to_dpc;    // +0x218
    std::uint64_t m_ke_release_spin_lock;                 // +0x220
    std::uint64_t m_mm_get_session_by_id;                 // +0x228
    std::uint64_t m_mm_get_next_session;                  // +0x230
    std::uint64_t m_mm_quit_next_session;                 // +0x238
    std::uint64_t m_mm_attach_session;                    // +0x240
    std::uint64_t m_mm_detach_session;                    // +0x248
    std::uint64_t m_mm_get_session_id_ex;                 // +0x250
    std::uint64_t m_mm_is_session_address;                // +0x258
    std::uint64_t m_ke_insert_queue_apc;                  // +0x260
    std::uint64_t m_ke_wait_for_single_object;            // +0x268
    std::uint64_t m_ps_create_system_thread;              // +0x270
    std::uint64_t m_ex_reference_call_back_block;         // +0x278
    std::uint64_t m_ex_get_call_back_block_routine;       // +0x280
    std::uint64_t m_ex_dereference_call_back_block;       // +0x288
    std::uint64_t m_ki_scb_queue_scan_worker;             // +0x290
    std::uint64_t m_psp_enumerate_callback;               // +0x298
    std::uint64_t m_cmp_enumerate_callback;               // +0x2A0
    std::uint64_t m_dbg_enumerate_callback;               // +0x2A8
    std::uint64_t m_exp_enumerate_callback;               // +0x2B0
    std::uint64_t m_exp_get_next_callback;                // +0x2B8
    std::uint64_t m_pop_po_coalescing_callback_;          // +0x2C0
    std::uint64_t m_ki_scheduler_apc_terminate;           // +0x2C8
    std::uint64_t m_ki_scheduler_apc;                     // +0x2D0
    std::uint64_t m_pop_po_coalescing_callback;           // +0x2D8
    std::uint64_t m_pg_self_encrypt_wait_and_decrypt;     // +0x2E0
    std::uint64_t m_ki_wait_always;                       // +0x2E8
    std::uint64_t m_ki_entropy_timing_routine;            // +0x2F0
    std::uint64_t m_ki_process_list_head;                 // +0x2F8
    std::uint64_t m_ki_process_list_lock;                 // +0x300
    std::uint64_t m_obp_type_object_type;                 // +0x308
    std::uint64_t m_io_driver_object_type;                // +0x310
    std::uint64_t m_ps_active_process_head;               // +0x318
    std::uint64_t m_ps_inverted_function_table;           // +0x320
    std::uint64_t m_ps_loaded_module_list;                // +0x328
    std::uint64_t m_ps_loaded_module_resource;            // +0x330
    std::uint64_t m_ps_loaded_module_spin_lock;           // +0x338
    std::uint64_t m_psp_active_process_lock;              // +0x340
    std::uint64_t m_psp_cid_table;                        // +0x348
    std::uint64_t m_exp_uuid_lock;                        // +0x350
    std::uint64_t m_alpc_port_list_lock;                  // +0x358
    std::uint64_t m_ke_service_descriptor_table;          // +0x360
    std::uint64_t m_ke_service_descriptor_table_shadow;   // +0x368
    std::uint64_t m_vf_thunks_extended;                   // +0x370
    std::uint64_t m_ps_win32_call_back;                   // +0x378
    std::uint64_t m_triage_image_page_size_0x28;          // +0x380
    std::uint64_t m_ki_table_information;                 // +0x388
    std::uint64_t m_handle_table_list_head;               // +0x390
    std::uint64_t m_handle_table_list_lock;               // +0x398
    std::uint64_t m_obp_kernel_handle_table;              // +0x3A0
    std::uint64_t m_hyper_space;                          // +0x3A8
    std::uint64_t m_ki_wait_never;                        // +0x3B0
    std::uint64_t m_kx_unexpected_interrupt0;             // +0x3B8
    std::uint64_t m_pg_context_end_field_to_be_cached;    // +0x3C0
    std::uint64_t m_unknown13;                            // +0x3C8
    std::uint64_t m_worker_queue_item;                    // +0x3D0
    std::uint64_t m_ex_node0_0x198;                       // +0x3D8
    std::uint64_t m_worker_routine;                       // +0x3E0
    std::uint64_t m_worker_queue_context;                 // +0x3E8
    std::uint64_t m_unknown15;                            // +0x3F0
    std::uint64_t m_prcb;                                 // +0x3F8
    std::uint64_t m_page_base;                            // +0x400
    std::uint64_t m_second_param_of_end_of_uninitialize;  // +0x408
    std::uint64_t m_dcp_routine_to_be_scheduled;          // +0x410
    std::uint32_t m_number_of_chunks_to_be_validated;     // +0x418
    std::uint32_t m_field_41c;                            // +0x41C
    std::uint32_t m_offset_to_pg_self_validation_in_bytes; // +0x420
    std::uint32_t m_field_424;                            // +0x424
    std::uint32_t m_offset_to_fs_uninitialize_small_mcb_in_bytes; // +0x428
    std::uint32_t m_field_42c;                            // +0x42C
    std::uint64_t m_spin_lock;                            // +0x430
    std::uint32_t m_offset_to_validation_info_in_bytes;   // +0x438
    std::uint32_t m_field_43c;                            // +0x43C
    std::uint32_t m_unknown22;                            // +0x440
    std::uint32_t m_hash_shift;                           // +0x444
    std::uint64_t m_hash_seed;                            // +0x448
    std::uint64_t m_unknown24;                            // +0x450
    std::uint32_t m_compared_size_for_hash;               // +0x458
    std::uint32_t m_field_45c;                            // +0x45C
    std::uint32_t m_unknown26;                            // +0x460
    std::uint32_t m_field_464;                            // +0x464
    std::uint64_t m_scheduler_type;                       // +0x468
    std::uint64_t m_unknown28;                            // +0x470
    std::uint64_t m_unknown29;                            // +0x478
    std::uint64_t m_unknown30;                            // +0x480
    std::uint64_t m_unknown31;                            // +0x488
    std::uint64_t m_unknown32;                            // +0x490
    std::uint64_t m_unknown33;                            // +0x498
    std::uint64_t m_unknown34;                            // +0x4A0
    std::uint64_t m_unknown35;                            // +0x4A8
    std::uint64_t m_unknown36;                            // +0x4B0
    std::uint64_t m_guard_check_icall_fptr;               // +0x4B8
    std::uint64_t m_hal_guard_check_icall_fptr;           // +0x4C0
    std::uint64_t m_guard_check_icall_fptr_108;           // +0x4C8
    std::uint64_t m_is_error_found;                       // +0x4D0
    std::uint64_t m_bug_chk_param1;                       // +0x4D8
    std::uint64_t m_bug_chk_param2;                       // +0x4E0
    std::uint64_t m_bug_chk_param4_type;                  // +0x4E8
    std::uint64_t m_bug_chk_param3;                       // +0x4F0
    std::uint32_t m_unknown42;                            // +0x4F8
    std::uint32_t m_should_mm_allocate_independent_pages_be_used; // +0x4FC
    std::uint32_t m_lock_type;                            // +0x500
    std::uint32_t m_field_504;                            // +0x504
    std::uint64_t m_pagevrf;                              // +0x508
    std::uint64_t m_pagespec;                             // +0x510
    std::uint64_t m_init;                                 // +0x518
    std::uint64_t m_pagekd;                               // +0x520
    std::uint64_t m_unknown44;                            // +0x528
    std::uint64_t m_triage_image_page_size_0x48;          // +0x530
    std::uint64_t m_unknown45;                            // +0x538
    std::uint32_t m_check_win32k_if_not_negative_one;     // +0x540
    std::uint32_t m_field_544;                            // +0x544
    std::uint64_t m_win32k_base;                          // +0x548
    std::uint64_t m_session_pointer;                      // +0x550
    std::uint32_t m_on_the_fly_en_decryption_flag;        // +0x558
    std::uint32_t m_dispatcher_header_should_be_masked;   // +0x55C
    std::uint32_t m_used_to_determine_error_as_flag;      // +0x560
    std::uint32_t m_field_564;                            // +0x564
    std::uint64_t m_thread_for_apc;                       // +0x568
    std::uint64_t m_unknown51;                            // +0x570
    std::uint64_t m_unknown52;                            // +0x578
    std::uint64_t m_unknown53;                            // +0x580
    std::uint64_t m_unknown54;                            // +0x588
    std::uint64_t m_unknown55;                            // +0x590
    std::uint64_t m_unknown56;                            // +0x598
    std::uint64_t m_unknown57;                            // +0x5A0
    std::uint64_t m_unknown58;                            // +0x5A8
    std::uint64_t m_apc;                                  // +0x5B0
    std::uint64_t m_ki_dispatch_callout;                  // +0x5B8
    std::uint64_t m_emp_check_errata_list_pop_po_coalescing_callback; // +0x5C0
    std::uint64_t m_should_ke_wait_for_single_object_be_used; // +0x5C8
    std::uint64_t m_should_ki_scb_queue_worker_be_used;   // +0x5D0
    std::uint64_t m_unknown62;                            // +0x5D8
    std::uint64_t m_unknown63;                            // +0x5E0
    std::uint64_t m_pg_original_hash;                     // +0x5E8
    std::uint64_t m_unknown65;                            // +0x5F0
    std::uint64_t m_should_pg_self_encrypt_wait_and_decrypt_be_used; // +0x5F8
    std::uint32_t m_offset_to_pte_restore_info_array_in_numbers; // +0x600
    std::uint32_t m_number_of_pte_to_be_restored;         // +0x604
    std::uint64_t m_hal_hali_halt_system;                 // +0x608
    std::uint64_t m_unknown68;                            // +0x610
    std::uint64_t m_ke_bug_check_ex_;                     // +0x618
    std::uint64_t m_unknown69;                            // +0x620
    std::uint64_t m_ke_bug_check2;                        // +0x628
    std::uint64_t m_unknown70;                            // +0x630
    std::uint64_t m_ki_bug_check_debug_break;             // +0x638
    std::uint64_t m_unknown71;                            // +0x640
    std::uint64_t m_ki_debug_trap_or_fault;               // +0x648
    std::uint64_t m_unknown72;                            // +0x650
    std::uint64_t m_dbg_break_point_with_status;          // +0x658
    std::uint64_t m_unknown73;                            // +0x660
    std::uint64_t m_rtl_capture_context;                  // +0x668
    std::uint64_t m_unknown74;                            // +0x670
    std::uint64_t m_ke_query_current_stack_information;   // +0x678
    std::uint64_t m_unknown75;                            // +0x680
    std::uint64_t m_ke_query_current_stack_information_chunk; // +0x688
    std::uint64_t m_unknown76;                            // +0x690
    std::uint64_t m_ki_save_processor_control_state;      // +0x698
    std::uint64_t m_unknown77;                            // +0x6A0
    std::uint64_t m_hal_private_dispatch_table_0x48;      // +0x6A8
    std::uint64_t m_unknown78;                            // +0x6B0
    std::uint64_t m_unknown79;                            // +0x6B8
    std::uint64_t m_unknown80;                            // +0x6C0
    std::uint64_t m_unknown81;                            // +0x6C8
    std::uint64_t m_unknown82;                            // +0x6D0
    std::uint64_t m_unknown83;                            // +0x6D8
    std::uint64_t m_unknown84;                            // +0x6E0
    std::uint64_t m_unknown85;                            // +0x6E8
    std::uint64_t m_unknown86;                            // +0x6F0
    std::uint64_t m_unknown87;                            // +0x6F8
    std::uint64_t m_unknown88;                            // +0x700
    std::uint64_t m_unknown89;                            // +0x708
    std::uint64_t m_unknown90;                            // +0x710
    std::uint64_t m_unknown91;                            // +0x718
    std::uint64_t m_unknown92;                            // +0x720
    std::uint64_t m_unknown93;                            // +0x728
    std::uint64_t m_unknown94;                            // +0x730
    std::uint64_t m_unknown95;                            // +0x738
    std::uint64_t m_unknown96;                            // +0x740
    std::uint64_t m_unknown97;                            // +0x748
    std::uint64_t m_unknown98;                            // +0x750
    std::uint64_t m_unknown99;                            // +0x758
    std::uint64_t m_unknown100;                           // +0x760
    std::uint64_t m_unknown101;                           // +0x768
    std::uint64_t m_unknown102;                           // +0x770
    std::uint64_t m_unknown103;                           // +0x778
    std::uint64_t m_unknown104;                           // +0x780
    std::uint64_t m_unknown105;                           // +0x788
    std::uint64_t m_unknown106;                           // +0x790
    std::uint64_t m_unknown107;                           // +0x798
    std::uint64_t m_unknown108;                           // +0x7A0
}; // Size = 0x7A8
static_assert( sizeof( pg_context_t ) == 0x7A8, "pg_context_t size mismatch" );

enum class device_type_t : unsigned long {
    beep = 0x00000001,
    cd_rom = 0x00000002,
    cd_rom_file_system = 0x00000003,
    controller = 0x00000004,
    datalink = 0x00000005,
    dfs = 0x00000006,
    disk = 0x00000007,
    disk_file_system = 0x00000008,
    file_system = 0x00000009,
    inport_port = 0x0000000a,
    keyboard = 0x0000000b,
    mailslot = 0x0000000c,
    midi_in = 0x0000000d,
    midi_out = 0x0000000e,
    mouse = 0x0000000f,
    multi_unc_provider = 0x00000010,
    named_pipe = 0x00000011,
    network = 0x00000012,
    network_browser = 0x00000013,
    network_file_system = 0x00000014,
    null = 0x00000015,
    parallel_port = 0x00000016,
    physical_netcard = 0x00000017,
    printer = 0x00000018,
    scanner = 0x00000019,
    serial_mouse_port = 0x0000001a,
    serial_port = 0x0000001b,
    screen = 0x0000001c,
    sound = 0x0000001d,
    streams = 0x0000001e,
    tape = 0x0000001f,
    tape_file_system = 0x00000020,
    transport = 0x00000021,
    unknown = 0x00000022,
    video = 0x00000023,
    virtual_disk = 0x00000024,
    wave_in = 0x00000025,
    wave_out = 0x00000026,
    port_8042 = 0x00000027,
    network_redirector = 0x00000028,
    battery = 0x00000029,
    bus_extender = 0x0000002a,
    modem = 0x0000002b,
    vdm = 0x0000002c,
    mass_storage = 0x0000002d,
    smb = 0x0000002e,
    ks = 0x0000002f,
    changer = 0x00000030,
    smartcard = 0x00000031,
    acpi = 0x00000032,
    dvd = 0x00000033,
    full_text_index = 0x00000034,
    dfs_file_system = 0x00000035,
    dfs_volume = 0x00000036,
    serenum = 0x00000037,
    termsrv = 0x00000038,
    ksec = 0x00000039,
    fips = 0x0000003a,
    infiniband = 0x0000003b
};

typedef struct rtl_balanced_node_t {
    union {
        struct rtl_balanced_node_t* m_children[ 2 ];
        struct {
            struct rtl_balanced_node_t* m_left;
            struct rtl_balanced_node_t* m_right;
        };
    };

#define RTL_BALANCED_NODE_RESERVED_PARENT_MASK 3

    union {
        std::uint8_t m_red : 1;
        std::uint8_t m_balance : 2;
        std::uint64_t m_parent_value;
    };
};

struct rtl_avl_tree_t {
    rtl_balanced_node_t* m_root;
};

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

struct mmvad_flags_t {
    std::uint32_t m_lock : 1;                                                           //0x0
    std::uint32_t m_lock_contended : 1;                                                  //0x0
    std::uint32_t m_delete_in_progress : 1;                                               //0x0
    std::uint32_t m_no_change : 1;                                                       //0x0
    std::uint32_t m_vad_type : 3;                                                        //0x0
    std::uint32_t m_protection : 5;                                                     //0x0
    std::uint32_t m_preferred_node : 6;                                                  //0x0
    std::uint32_t m_page_size : 2;                                                       //0x0
    std::uint32_t m_private_memory : 1;                                                  //0x0
};

struct mmvad_flags1_t {
    std::uint32_t m_commit_charge : 31;                                                  //0x0
    std::uint32_t m_mem_commit : 1;                                                      //0x0
};

struct private_vad_flags_t {
    std::uint32_t m_lock : 1;                                                           //0x0
    std::uint32_t m_lock_contended : 1;                                                  //0x0
    std::uint32_t m_delete_in_progress : 1;                                               //0x0
    std::uint32_t m_no_change : 1;                                                       //0x0
    std::uint32_t m_vad_type : 3;                                                        //0x0
    std::uint32_t m_protection : 5;                                                     //0x0
    std::uint32_t m_preferred_node : 6;                                                  //0x0
    std::uint32_t m_page_size : 2;                                                       //0x0
    std::uint32_t m_private_memory_always_set : 1;                                         //0x0
    std::uint32_t m_write_watch : 1;                                                     //0x0
    std::uint32_t m_fixed_large_page_size : 1;                                             //0x0
    std::uint32_t m_zero_fill_pages_optional : 1;                                          //0x0
    std::uint32_t m_graphics : 1;                                                       //0x0
    std::uint32_t m_enclave : 1;                                                        //0x0
    std::uint32_t m_shadow_stack : 1;                                                    //0x0
    std::uint32_t m_physical_memory_pfns_referenced : 1;                                   //0x0
};

struct graphics_vad_node_t {
    std::uint32_t m_lock : 1;
    std::uint32_t m_lock_contended : 1;                                                //0x0
    std::uint32_t DeleteInProgress : 1;                                               //0x0
    std::uint32_t NoChange : 1;                                                       //0x0
    std::uint32_t VadType : 3;                                                        //0x0
    std::uint32_t Protection : 5;                                                     //0x0
    std::uint32_t PreferredNode : 6;                                                  //0x0
    std::uint32_t PageSize : 2;                                                       //0x0
    std::uint32_t PrivateMemoryAlwaysSet : 1;                                         //0x0
    std::uint32_t WriteWatch : 1;                                                     //0x0
    std::uint32_t FixedLargePageSize : 1;                                             //0x0
    std::uint32_t ZeroFillPagesOptional : 1;                                          //0x0
    std::uint32_t GraphicsAlwaysSet : 1;                                              //0x0
    std::uint32_t GraphicsUseCoherentBus : 1;                                         //0x0
    std::uint32_t GraphicsNoCache : 1;                                                //0x0
    std::uint32_t GraphicsPageProtection : 3;                                         //0x0
};

struct shared_vad_flags_t {
    ULONG Lock : 1;                                                           //0x0
    ULONG LockContended : 1;                                                  //0x0
    ULONG DeleteInProgress : 1;                                               //0x0
    ULONG NoChange : 1;                                                       //0x0
    ULONG VadType : 3;                                                        //0x0
    ULONG Protection : 5;                                                     //0x0
    ULONG PreferredNode : 6;                                                  //0x0
    ULONG PageSize : 2;                                                       //0x0
    ULONG PrivateMemoryAlwaysClear : 1;                                       //0x0
    ULONG PrivateFixup : 1;                                                   //0x0
    ULONG HotPatchAllowed : 1;                                                //0x0
};

struct mmvad_short_t {
    union {
        struct {
            mmvad_short_t* m_next_vad;                                   //0x0
            void* m_extra_create_info;                                          //0x8
        };
        rtl_balanced_node_t m_vad_node;                                  //0x0
    };
    std::uint32_t m_starting_vpn;                                                      //0x18
    std::uint32_t m_ending_vpn;                                                        //0x1c
    std::uint8_t m_starting_vpn_high;                                                  //0x20
    std::uint8_t m_ending_vpn_high;                                                    //0x21
    std::uint8_t m_commit_charge_high;                                                 //0x22
    std::uint8_t m_spare_nt_64_vad_uchar;                                                //0x23
    std::uint32_t m_reference_count;                                                    //0x24
    ex_push_lock_t m_push_lock;                                          //0x28
    union {
        std::uint32_t m_long_flags;                                                    //0x30
        mmvad_flags_t m_vad_flags;
        private_vad_flags_t m_private_vad_flags;                       //0x30ags;                                       //0x30
        graphics_vad_node_t m_graphics_vad_flags;                     //0x30
        shared_vad_flags_t m_shared_vad_flags;                         //0x30
        volatile std::uint32_t m_volatile_vad_long;                                     //0x30
    } u;                                                                    //0x30
    union {
        std::uint32_t m_long_flags1;                                                   //0x34
        mmvad_flags1_t m_vad_flags1;                                     //0x34
    } u1;                                                                   //0x34
    void* m_event_list;                                  //0x38
};

struct device_object_t {
    short type;
    unsigned short size;
    long reference_count;
    driver_object_t* driver_object;
    device_object_t* next_device;
    device_object_t* attached_device;
    void* current_irp;
    void* timer_queue;
    unsigned long flags;
    unsigned long characteristics;
    void* vpb;
    void* device_extension;
    device_type_t device_type;
    unsigned char stack_size;
    union {
        struct {
            unsigned short pending_returned;
            unsigned short padding;
        } list_entry;
        void* wait_list_entry;
    } queue;
    unsigned long align_requirement;
    void* device_queue;
    void* dpc;
    unsigned long active_threads;
    void* security_descriptor;
    void* device_lock;
    unsigned short sector_size;
    unsigned short spare1;
    void* device_object_extension;
    void* reserved;
};

struct driver_object_t {
    short type;
    short size;
    device_object_t* device_object;
    unsigned long flags;
    void* driver_start;
    unsigned long driver_size;
    void* driver_section;
    void* driver_extension;
    unicode_string_t driver_name;
    unicode_string_t* hardware_database;
    void* fast_io_dispatch;
    void* driver_init;
    void* driver_start_io;
    void* driver_unload;
    void* major_function[ 28 ];
};

typedef
_IRQL_requires_same_
_Function_class_( driver_initialize )
nt_status_t
driver_initialize(
    _In_ driver_object_t* driver_object,
    _In_ unicode_string_t* registry_path
);
typedef driver_initialize* pdriver_initialize;

struct mdl_t {
    struct mdl_t* m_next;
    std::uint32_t m_size;
    std::uint32_t m_mdl_flags;

    struct eprocess_t* m_process;
    void* m_mapped_system_va;   /* see creators for field size annotations. */
    void* m_start_va;   /* see creators for validity; could be address 0.  */
    std::uint32_t m_byte_count;
    std::uint32_t m_byte_offset;
};

struct iop_irp_stack_profiler_t {
    std::uint8_t m_major_function;
    std::uint8_t m_minor_function;
    std::uint8_t m_flags;
    std::uint8_t m_control;

    union {
        // For NtCreateFile
        struct {
            void* m_security_context;
            std::uint32_t m_options;
            std::uint16_t m_file_attributes;
            std::uint16_t m_share_access;
            std::uint32_t m_ea_length;
        } m_create;

        // For NtCreateNamedPipeFile
        struct {
            void* m_security_context;
            std::uint32_t m_options;
            std::uint16_t m_reserved;
            std::uint16_t m_share_access;
            void* m_parameters;
        } m_create_pipe;

        // For NtCreateMailslotFile
        struct {
            void* m_security_context;
            std::uint32_t m_options;
            std::uint16_t m_reserved;
            std::uint16_t m_share_access;
            void* m_parameters;
        } m_create_mailslot;

        // For NtReadFile
        struct {
            std::uint32_t m_length;
            std::uint32_t m_key;
#if defined(_WIN64)
            std::uint32_t m_flags;
#endif
            std::int64_t m_byte_offset;
        } m_read;

        // For NtWriteFile
        struct {
            std::uint32_t m_length;
            std::uint32_t m_key;
#if defined(_WIN64)
            std::uint32_t m_flags;
#endif
            std::int64_t m_byte_offset;
        } m_write;

        // For NtQueryDirectoryFile
        struct {
            std::uint32_t m_length;
            void* m_file_name;
            std::uint32_t m_file_information_class;
            std::uint32_t m_file_index;
        } m_query_directory;

        // For NtNotifyChangeDirectoryFile
        struct {
            std::uint32_t m_length;
            std::uint32_t m_completion_filter;
        } m_notify_directory;

        // For NtNotifyChangeDirectoryFileEx
        struct {
            std::uint32_t m_length;
            std::uint32_t m_completion_filter;
            std::uint32_t m_directory_notify_information_class;
        } m_notify_directory_ex;

        // For NtQueryInformationFile
        struct {
            std::uint32_t m_length;
            std::uint32_t m_file_information_class;
        } m_query_file;

        // For NtSetInformationFile
        struct {
            std::uint32_t m_length;
            std::uint32_t m_file_information_class;
            void* m_file_object;
            union {
                struct {
                    std::uint8_t m_replace_if_exists;
                    std::uint8_t m_advance_only;
                };
                std::uint32_t m_cluster_count;
                void* m_delete_handle;
            };
        } m_set_file;

        // For NtQueryEaFile
        struct {
            std::uint32_t m_length;
            void* m_ea_list;
            std::uint32_t m_ea_list_length;
            std::uint32_t m_ea_index;
        } m_query_ea;

        // For NtSetEaFile
        struct {
            std::uint32_t m_length;
        } m_set_ea;

        // For NtQueryVolumeInformationFile
        struct {
            std::uint32_t m_length;
            std::uint32_t m_fs_information_class;
        } m_query_volume;

        // For NtSetVolumeInformationFile
        struct {
            std::uint32_t m_length;
            std::uint32_t m_fs_information_class;
        } m_set_volume;

        // For NtFsControlFile
        struct {
            std::uint32_t m_output_buffer_length;
            std::uint32_t m_input_buffer_length;
            std::uint32_t m_fs_control_code;
            void* m_type3_input_buffer;
        } m_file_system_control;

        // For NtLockFile/NtUnlockFile
        struct {
            void* m_length;
            std::uint32_t m_key;
            std::int64_t m_byte_offset;
        } m_lock_control;

        // For NtDeviceIoControlFile
        struct {
            std::uint32_t m_output_buffer_length;
            std::uint32_t m_input_buffer_length;
            std::uint32_t m_io_control_code;
            void* m_type3_input_buffer;
        } m_device_io_control;

        // For NtQuerySecurityObject
        struct {
            std::uint32_t m_security_information;
            std::uint32_t m_length;
        } m_query_security;

        // For NtSetSecurityObject
        struct {
            std::uint32_t m_security_information;
            void* m_security_descriptor;
        } m_set_security;

        // For MountVolume
        struct {
            void* m_vpb;
            void* m_device_object;
            std::uint32_t m_output_buffer_length;
        } m_mount_volume;

        // For VerifyVolume
        struct {
            void* m_vpb;
            void* m_device_object;
        } m_verify_volume;

        // For Scsi with internal device control
        struct {
            void* m_srb;
        } m_scsi;

        // For NtQueryQuotaInformationFile
        struct {
            std::uint32_t m_length;
            void* m_start_sid;
            void* m_sid_list;
            std::uint32_t m_sid_list_length;
        } m_query_quota;

        // For NtSetQuotaInformationFile
        struct {
            std::uint32_t m_length;
        } m_set_quota;

        // For IRP_MN_QUERY_DEVICE_RELATIONS
        struct {
            std::uint32_t m_type;
        } m_query_device_relations;

        // For IRP_MN_QUERY_INTERFACE
        struct {
            const void* m_interface_type;
            std::uint16_t m_size;
            std::uint16_t m_version;
            void* m_interface;
            void* m_interface_specific_data;
        } m_query_interface;

        // For IRP_MN_QUERY_CAPABILITIES
        struct {
            void* m_capabilities;
        } m_device_capabilities;

        // For IRP_MN_FILTER_RESOURCE_REQUIREMENTS
        struct {
            void* m_io_resource_requirement_list;
        } m_filter_resource_requirements;

        // For IRP_MN_READ_CONFIG and IRP_MN_WRITE_CONFIG
        struct {
            std::uint32_t m_which_space;
            void* m_buffer;
            std::uint32_t m_offset;
            std::uint32_t m_length;
        } m_read_write_config;

        // For IRP_MN_SET_LOCK
        struct {
            std::uint8_t m_lock;
        } m_set_lock;

        // For IRP_MN_QUERY_ID
        struct {
            std::uint32_t m_id_type;
        } m_query_id;

        // For IRP_MN_QUERY_DEVICE_TEXT
        struct {
            std::uint32_t m_device_text_type;
            std::uint32_t m_locale_id;
        } m_query_device_text;

        // For IRP_MN_DEVICE_USAGE_NOTIFICATION
        struct {
            std::uint8_t m_in_path;
            std::uint8_t m_reserved[ 3 ];
            std::uint32_t m_type;
        } m_usage_notification;

        // For IRP_MN_WAIT_WAKE
        struct {
            std::uint32_t m_power_state;
        } m_wait_wake;

        // For IRP_MN_POWER_SEQUENCE
        struct {
            void* m_power_sequence;
        } m_power_sequence;

        // For IRP_MN_SET_POWER and IRP_MN_QUERY_POWER
#if (NTDDI_VERSION >= NTDDI_VISTA)
        struct {
            union {
                std::uint32_t m_system_context;
                std::uint64_t m_system_power_state_context;
            };
            std::uint32_t m_type;
            std::uint32_t m_state;
            std::uint32_t m_shutdown_type;
        } m_power;
#else
        struct {
            std::uint32_t m_system_context;
            std::uint32_t m_type;
            std::uint32_t m_state;
            std::uint32_t m_shutdown_type;
        } m_power;
#endif

        // For StartDevice
        struct {
            void* m_allocated_resources;
            void* m_allocated_resources_translated;
        } m_start_device;

        // For WMI Irps
        struct {
            std::uint64_t m_provider_id;
            void* m_data_path;
            std::uint32_t m_buffer_size;
            void* m_buffer;
        } m_wmi;

        // Others - driver-specific
        struct {
            void* m_arg1;
            void* m_arg2;
            void* m_arg3;
            void* m_arg4;
        } m_others;

    } m_parameters;

    void* m_device_object;
    void* m_file_object;
    void* m_completion_routine;
    void* m_context;
};

struct iop_irp_t {
    std::uint16_t m_type;                  // Type
    std::uint16_t m_size;                  // Size
    void* m_mdl;                           // MdlAddress
    std::uint32_t m_flags;                 // Flags

    // AssociatedIrp union
    union {
        struct iop_irp_t* m_master_irp;    // MasterIrp
        volatile std::int32_t m_irp_count; // IrpCount
        void* m_system_buffer;             // SystemBuffer
    } m_associated_irp;

    // ThreadListEntry
    struct {
        void* m_flink;
        void* m_blink;
    } m_thread_list_entry;

    // IoStatus
    struct {
        nt_status_t m_status;             // Status
        std::uint64_t m_information;       // Information
    } m_io_status;

    std::uint8_t m_requestor_mode;         // RequestorMode
    std::uint8_t m_pending_returned;       // PendingReturned
    std::int8_t m_stack_count;             // StackCount
    std::int8_t m_current_location;        // CurrentLocation
    std::uint8_t m_cancel;                 // Cancel
    std::uint8_t m_cancel_irql;            // CancelIrql
    std::int8_t m_apc_environment;         // ApcEnvironment
    std::uint8_t m_allocation_flags;       // AllocationFlags

    // User parameters
    union {
        void* m_user_iosb;                 // UserIosb
        void* m_io_ring_context;           // IoRingContext
    } m_user_iosb_union;

    void* m_user_event;                    // UserEvent

    // Overlay union
    union {
        struct {
            union {
                void* m_user_apc_routine;   // UserApcRoutine
                void* m_issuing_process;    // IssuingProcess
            } m_apc_routine_union;

            union {
                void* m_user_apc_context;   // UserApcContext
                void* m_io_ring;            // IoRing
            } m_apc_context_union;
        } m_asynchronous_parameters;

        std::uint64_t m_allocation_size;    // AllocationSize
    } m_overlay;

    volatile void* m_cancel_routine;        // CancelRoutine
    void* m_user_buffer;                    // UserBuffer

    // Tail union
    union {
        struct {
            union {
                // DeviceQueueEntry
                struct {
                    void* m_device_list_entry;
                    void* m_sorting_key;
                    std::uint8_t m_inserted;
                } m_device_queue_entry;

                // DriverContext
                struct {
                    void* m_driver_context[ 4 ];
                };
            } m_device_context_union;

            void* m_thread;                 // Thread
            void* m_auxiliary_buffer;       // AuxiliaryBuffer

            struct {
                // ListEntry
                struct {
                    void* m_flink;
                    void* m_blink;
                } m_list_entry;

                union {
                    // CurrentStackLocation
                    struct iop_irp_stack_profiler_t* m_current_stack_location;
                    std::uint32_t m_packet_type;  // PacketType
                } m_stack_location_union;
            } m_list_stack_struct;

            void* m_original_file_object;   // OriginalFileObject
        } m_overlay;

        // Apc
        struct {
            // KAPC structure fields
            std::uint16_t m_type;
            std::uint16_t m_size;
            std::uint32_t m_spare0;
            void* m_thread;
            void* m_apc_list_entry;
            void* m_kernel_routine;
            void* m_rundown_routine;
            void* m_normal_routine;
            void* m_normal_context;
            void* m_system_argument1;
            void* m_system_argument2;
            std::uint8_t m_apc_state_index;
            std::uint8_t m_apc_mode;
            std::uint8_t m_inserted;
            std::uint8_t m_padding;
        } m_apc;

        void* m_completion_key;             // CompletionKey
    } m_tail;

    // The stack array is at the end (variable length)
    iop_irp_stack_profiler_t m_stack[ 1 ];
};

struct object_attributes_t {
    std::uint32_t m_length;
    void* m_root_directory;
    unicode_string_t* m_object_name;
    std::uint32_t m_attributes;
    void* m_security_descriptor;
    void* m_security_q1uality_of_service;
};

struct ex_rundown_ref_t {
    union {
        std::uint64_t m_count;                    // Size=0x8
        void* m_ptr;                              // Size=0x8
    };
};

struct ex_fast_ref_t {
    union {
        void* m_object;
        std::uint64_t m_ref_cnt : 4;
        std::uint64_t m_value;
    };
}; // Size: 0x8

struct se_audit_process_creation_info_t {
    unicode_string_t* m_image_file_name;    // Pointer to UNICODE_STRING
};

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

struct kprocess_t {
    dispatcher_header_t m_header;                       // +0x000
    list_entry_t m_profile_list_head;                  // +0x018
    std::uint64_t m_directory_table_base;              // +0x028
    list_entry_t m_thread_list_head;                   // +0x030
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

struct eprocess_t {
    kprocess_t m_pcb;                                                     // +0x000
    ex_push_lock_t m_process_lock;                                       // +0x438
    void* m_unique_process_id;                                           // +0x440
    list_entry_t m_active_process_links;                                // +0x448
    ex_rundown_ref_t m_rundown_protect;                                 // +0x458

    union {
        std::uint32_t m_flags2;                                         // +0x460
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
            std::uint32_t m_force_ws_watch : 1;
            std::uint32_t m_create_reported : 1;
            std::uint32_t m_default_io_priority : 3;
            std::uint32_t m_spare_bits : 17;
        };
    };

    union {
        std::uint32_t m_flags;                                          // +0x464
        struct {
            std::uint32_t m_create_time_reported : 1;
            std::uint32_t m_image_not_loaded : 1;
            std::uint32_t m_process_exiting : 1;
            std::uint32_t m_process_delete : 1;
            std::uint32_t m_wow64_split_pages : 1;
            std::uint32_t m_vm_deleted : 1;
            std::uint32_t m_outswap_enabled : 1;
            std::uint32_t m_outswapped : 1;
            std::uint32_t m_fork_failed : 1;
            std::uint32_t m_has_address_space : 1;
            std::uint32_t m_address_space_initialized : 2;
            std::uint32_t m_set_timer_resolution : 1;
            std::uint32_t m_break_on_termination : 1;
            std::uint32_t m_dependent_on_session : 1;
            std::uint32_t m_auto_alignment : 1;
            std::uint32_t m_prefer_32bit : 1;
            std::uint32_t m_wow64_valid : 1;
            std::uint32_t m_cross_session_create : 1;
            std::uint32_t m_spare_flags0 : 13;
        };
    };

    std::int64_t m_create_time;                                         // +0x468
    std::uint64_t m_process_quota_usage[ 2 ];                            // +0x470
    std::uint64_t m_process_quota_peak[ 2 ];                             // +0x480
    std::uint64_t m_peak_virtual_size;                                 // +0x490
    std::uint64_t m_virtual_size;                                      // +0x498
    list_entry_t m_session_process_links;                              // +0x4A0
    union {
        void* m_exception_port;                                         // +0x4B0
        std::uint64_t m_exception_port_value;                          // +0x4B0
    };
    ex_fast_ref_t m_token;                                             // +0x4B8
    std::uint64_t m_working_set_page_count;                           // +0x4C0
    ex_push_lock_t m_address_creation_lock;                            // +0x4C8
    ex_push_lock_t m_page_table_commit_lock;                          // +0x4D0
    void* m_rotate_in_progress;                                        // +0x4D8
    void* m_fork_in_progress;                                          // +0x4E0
    std::uint64_t m_hardware_counters;                                // +0x4E8
    void* m_spare_ptr0;                                                // +0x4F0
    std::uint64_t m_spare_ulong0;                                     // +0x4F8
    std::uint64_t m_spare_ulong1;                                     // +0x500
    std::uint64_t m_spare_ulong2;                                     // +0x508
    std::uint64_t m_spare_ulong3;                                     // +0x510
    void* m_section_object;                                            // +0x518
    void* m_section_base_address;                                      // +0x520
    std::uint32_t m_cookie;                                           // +0x528
    std::uint32_t m_padding1;                                         // +0x52C
    void* m_working_set_watch;                                         // +0x530
    void* m_win32_window_station;                                      // +0x538
    void* m_inherited_from_unique_process_id;                          // +0x540
    void* m_peb;                                                       // +0x548
    void* m_session;                                                   // +0x550
    void* m_spare1;                                                    // +0x558
    void* m_quota_block;                                               // +0x560
    void* m_object_table;                                              // +0x568
    void* m_debug_port;                                                // +0x570
    void* m_wow64_process;                                             // +0x578
    ex_fast_ref_t m_device_map;                                        // +0x580
    void* m_etw_data_source;                                           // +0x588
    std::uint64_t m_page_directory_pte;                               // +0x590
    void* m_image_file_pointer;                                        // +0x598
    char m_image_file_name[ 15 ];                                        // +0x5A0
    std::uint8_t m_priority_class;                                     // +0x5AF
    void* m_security_port;                                             // +0x5B0
    se_audit_process_creation_info_t m_se_audit_process_creation_info; // +0x5B8
    list_entry_t m_job_links;                                         // +0x5C0
    void* m_spare2;                                                    // +0x5D0
    list_entry_t m_thread_list_head;                                  // +0x5D8
    std::uint32_t m_active_threads;                                   // +0x5E8
    std::uint32_t m_image_path_hash;                                  // +0x5EC
    std::uint32_t m_default_harderror_processing;                     // +0x5F0
    std::int32_t m_last_thread_exit_status;                          // +0x5F4
    void* m_pde_table;                                                // +0x5F8
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
    std::uint32_t m_mx_csr;                      // +0x02C

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

    std::m128a_t m_xmm0;                               // +0x070
    std::m128a_t m_xmm1;                               // +0x080
    std::m128a_t m_xmm2;                               // +0x090
    std::m128a_t m_xmm3;                               // +0x0A0
    std::m128a_t m_xmm4;                               // +0x0B0
    std::m128a_t m_xmm5;                               // +0x0C0

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

struct kapc_state_t {
    list_entry_t m_apc_list_head[ 2 ];
    eprocess_t* m_process;
    std::uint8_t m_kernel_apc_in_progress;
    std::uint8_t m_kernel_apc_pending;
    std::uint8_t m_user_apc_pending;
    std::uint8_t m_pad;
}; // Size: 0x40

struct teb_t;
struct nt_tib_t {
    struct _exception_registration_record* m_exception_list;  // 0x000
    std::uint8_t* m_stack_base;                                // 0x008
    std::uint8_t* m_stack_limit;                              // 0x010
    std::uint8_t* m_sub_system_tib;                           // 0x018
    union {
        std::uint8_t* m_fiber_data;                           // 0x020
        std::uint32_t m_version;                            // 0x020
    };
    std::uint8_t* m_arbitrary_user_pointer;                    // 0x028
    teb_t* m_self;                                          // 0x030
};

struct activation_context_stack_t {
    std::uint8_t* active_frame;                    // 0x000
    list_entry_t frame_list_cache;               // 0x008
    std::uint32_t flags;                         // 0x018
    std::uint32_t next_cookie;                   // 0x01C
    std::uint32_t frame_count;                   // 0x020
    std::uint32_t padding;                       // 0x024
};

struct client_id_t {
    void* m_unique_process;
    void* m_unique_thread;
};

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
    peb_ldr_data_t m_ldr;
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
};

struct guid_t {
    std::uint32_t m_data1;                                                  // 0x00
    std::uint16_t m_data2;                                                  // 0x04
    std::uint16_t m_data3;                                                  // 0x06
    std::uint8_t m_data4[ 8 ];                                                // 0x08
};

struct gdi_teb_batch_t {
    std::uint32_t m_offset;                                                 // 0x00
    std::uint64_t m_hdc;                                                    // 0x08
    std::uint32_t m_buffer[ 310 ];                                            // 0x10
};

struct processor_number_t {
    std::uint16_t m_group;                                                  // 0x00
    std::uint8_t m_number;                                                  // 0x02
    std::uint8_t m_reserved;                                                // 0x03
};

struct teb_active_frame_t {
    std::uint32_t m_flags;                                                  // 0x00
    std::uint8_t m_padding[ 4 ];                                              // 0x04
    teb_active_frame_t* m_previous;                                         // 0x08
    // Add more fields as needed
};

struct teb_t {
    nt_tib_t m_nt_tib;                                                      // 0x000
    std::uint8_t* m_environment_pointer;                                    // 0x038
    client_id_t m_client_id;                                                // 0x040
    std::uint8_t* m_active_rpc_handle;                                      // 0x050
    std::uint8_t* m_thread_local_storage_pointer;                           // 0x058
    peb_t* m_process_environment_block;                                     // 0x060
    std::uint32_t m_last_error_value;                                       // 0x068
    std::uint32_t m_count_of_owned_critical_sections;                       // 0x06C
    std::uint8_t* m_csr_client_thread;                                      // 0x070
    std::uint8_t* m_win32_thread_info;                                      // 0x078
    std::uint32_t m_user32_reserved[ 26 ];                                    // 0x080
    std::uint32_t m_user_reserved[ 5 ];                                       // 0x0E8
    std::uint8_t* m_wow32_reserved;                                         // 0x100
    std::uint32_t m_current_locale;                                         // 0x108
    std::uint32_t m_fp_software_status_register;                            // 0x10C
    std::uint8_t* m_reserved_for_debugger_instrumentation[ 16 ];              // 0x110
    std::uint8_t* m_system_reserved1[ 30 ];                                   // 0x190
    std::uint8_t m_placeholder_compatibility_mode;                          // 0x280
    std::uint8_t m_placeholder_hydration_always_explicit;                   // 0x281
    std::uint8_t m_placeholder_reserved[ 10 ];                                // 0x282
    std::uint32_t m_proxied_process_id;                                     // 0x28C
    activation_context_stack_t m_activation_stack;                          // 0x290
    std::uint8_t m_working_on_behalf_ticket[ 8 ];                             // 0x2B8
    std::int32_t m_exception_code;                                          // 0x2C0
    std::uint8_t m_padding0[ 4 ];                                             // 0x2C4
    activation_context_stack_t* m_activation_context_stack_pointer;         // 0x2C8
    std::uint64_t m_instrumentation_callback_sp;                            // 0x2D0
    std::uint64_t m_instrumentation_callback_previous_pc;                   // 0x2D8
    std::uint64_t m_instrumentation_callback_previous_sp;                   // 0x2E0
    std::uint32_t m_tx_fs_context;                                          // 0x2E8
    std::uint8_t m_instrumentation_callback_disabled;                       // 0x2EC
    std::uint8_t m_unaligned_load_store_exceptions;                         // 0x2ED
    std::uint8_t m_padding1[ 2 ];                                             // 0x2EE
    gdi_teb_batch_t m_gdi_teb_batch;                                        // 0x2F0
    client_id_t m_real_client_id;                                           // 0x7D8
    std::uint8_t* m_gdi_cached_process_handle;                              // 0x7E8
    std::uint32_t m_gdi_client_pid;                                         // 0x7F0
    std::uint32_t m_gdi_client_tid;                                         // 0x7F4
    std::uint8_t* m_gdi_thread_local_info;                                  // 0x7F8
    std::uint64_t m_win32_client_info[ 62 ];                                  // 0x800
    std::uint8_t* m_gl_dispatch_table[ 233 ];                                 // 0x9F0
    std::uint64_t m_gl_reserved1[ 29 ];                                       // 0x1138
    std::uint8_t* m_gl_reserved2;                                           // 0x1220
    std::uint8_t* m_gl_section_info;                                        // 0x1228
    std::uint8_t* m_gl_section;                                             // 0x1230
    std::uint8_t* m_gl_table;                                               // 0x1238
    std::uint8_t* m_gl_current_rc;                                          // 0x1240
    std::uint8_t* m_gl_context;                                             // 0x1248
    std::uint32_t m_last_status_value;                                      // 0x1250
    std::uint8_t m_padding2[ 4 ];                                             // 0x1254
    unicode_string_t m_static_unicode_string;                               // 0x1258
    wchar_t m_static_unicode_buffer[ 261 ];                                   // 0x1268
    std::uint8_t m_padding3[ 6 ];                                             // 0x1472
    std::uint8_t* m_deallocation_stack;                                     // 0x1478
    std::uint8_t* m_tls_slots[ 64 ];                                          // 0x1480
    list_entry_t m_tls_links;                                               // 0x1680
    std::uint8_t* m_vdm;                                                    // 0x1690
    std::uint8_t* m_reserved_for_nt_rpc;                                    // 0x1698
    std::uint8_t* m_dbg_ss_reserved[ 2 ];                                     // 0x16A0
    std::uint32_t m_hard_error_mode;                                        // 0x16B0
    std::uint8_t m_padding4[ 4 ];                                             // 0x16B4
    std::uint8_t* m_instrumentation[ 11 ];                                    // 0x16B8
    guid_t m_activity_id;                                                   // 0x1710
    std::uint8_t* m_sub_process_tag;                                        // 0x1720
    std::uint8_t* m_perflib_data;                                           // 0x1728
    std::uint8_t* m_etw_trace_data;                                         // 0x1730
    std::uint8_t* m_win_sock_data;                                          // 0x1738
    std::uint32_t m_gdi_batch_count;                                        // 0x1740
    union {
        processor_number_t m_current_ideal_processor;                       // 0x1744
        std::uint32_t m_ideal_processor_value;                              // 0x1744
        struct {
            std::uint8_t m_reserved_pad0;                                   // 0x1744
            std::uint8_t m_reserved_pad1;                                   // 0x1745
            std::uint8_t m_reserved_pad2;                                   // 0x1746
            std::uint8_t m_ideal_processor;                                 // 0x1747
        };
    };
    std::uint32_t m_guaranteed_stack_bytes;                                 // 0x1748
    std::uint8_t m_padding5[ 4 ];                                             // 0x174C
    std::uint8_t* m_reserved_for_perf;                                      // 0x1750
    std::uint8_t* m_reserved_for_ole;                                       // 0x1758
    std::uint32_t m_waiting_on_loader_lock;                                 // 0x1760
    std::uint8_t m_padding6[ 4 ];                                             // 0x1764
    std::uint8_t* m_saved_priority_state;                                   // 0x1768
    std::uint64_t m_reserved_for_code_coverage;                             // 0x1770
    std::uint8_t* m_thread_pool_data;                                       // 0x1778
    std::uint8_t** m_tls_expansion_slots;                                   // 0x1780
    std::uint8_t* m_deallocation_b_store;                                   // 0x1788
    std::uint8_t* m_b_store_limit;                                          // 0x1790
    std::uint32_t m_mui_generation;                                         // 0x1798
    std::uint32_t m_is_impersonating;                                       // 0x179C
    std::uint8_t* m_nls_cache;                                              // 0x17A0
    std::uint8_t* m_shim_data;                                              // 0x17A8
    std::uint32_t m_heap_data;                                              // 0x17B0
    std::uint8_t m_padding7[ 4 ];                                             // 0x17B4
    std::uint8_t* m_current_transaction_handle;                             // 0x17B8  <- Good storage location!
    teb_active_frame_t* m_active_frame;                                     // 0x17C0
    std::uint8_t* m_fls_data;                                               // 0x17C8
    std::uint8_t* m_preferred_languages;                                    // 0x17D0
    std::uint8_t* m_user_pref_languages;                                    // 0x17D8
    std::uint8_t* m_merged_pref_languages;                                  // 0x17E0
    std::uint32_t m_mui_impersonation;                                      // 0x17E8
    union {
        volatile std::uint16_t m_cross_teb_flags;                           // 0x17EC
        std::uint16_t m_spare_cross_teb_bits : 16;                          // 0x17EC
    };
    union {
        std::uint16_t m_same_teb_flags;                                     // 0x17EE
        struct {
            std::uint16_t m_safe_thunk_call : 1;                            // 0x17EE
            std::uint16_t m_in_debug_print : 1;                             // 0x17EE
            std::uint16_t m_has_fiber_data : 1;                             // 0x17EE
            std::uint16_t m_skip_thread_attach : 1;                         // 0x17EE
            std::uint16_t m_wer_in_ship_assert_code : 1;                    // 0x17EE
            std::uint16_t m_ran_process_init : 1;                           // 0x17EE
            std::uint16_t m_cloned_thread : 1;                              // 0x17EE
            std::uint16_t m_suppress_debug_msg : 1;                         // 0x17EE
            std::uint16_t m_disable_user_stack_walk : 1;                    // 0x17EE
            std::uint16_t m_rtl_exception_attached : 1;                     // 0x17EE
            std::uint16_t m_initial_thread : 1;                             // 0x17EE
            std::uint16_t m_session_aware : 1;                              // 0x17EE
            std::uint16_t m_load_owner : 1;                                 // 0x17EE
            std::uint16_t m_loader_worker : 1;                              // 0x17EE
            std::uint16_t m_skip_loader_init : 1;                           // 0x17EE
            std::uint16_t m_spare_same_teb_bits : 1;                        // 0x17EE
        };
    };
    std::uint8_t* m_txn_scope_enter_callback;                               // 0x17F0
    std::uint8_t* m_txn_scope_exit_callback;                                // 0x17F8
    std::uint8_t* m_txn_scope_context;                                      // 0x1800
    std::uint32_t m_lock_count;                                             // 0x1808
    std::int32_t m_wow_teb_offset;                                          // 0x180C
    std::uint8_t* m_resource_ret_value;                                     // 0x1810
    std::uint8_t* m_reserved_for_wdf;                                       // 0x1818
    std::uint64_t m_reserved_for_crt;                                       // 0x1820
    guid_t m_effective_container_id;                                        // 0x1828
};

struct kdpc_t {
    std::uint16_t m_type;
    std::uint8_t m_importance;
    std::uint8_t m_number;
    list_entry_t m_dpc_list_entry;
    void* m_deferred_routine;
    void* m_deferred_context;
    void* m_system_argument1;
    void* m_system_argument2;
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

struct group_affinity_t {
    std::uint64_t m_mask;
    std::uint16_t m_group;
    std::uint16_t m_reserved[ 3 ];
}; // Size: 0x10

struct kevent_t {
    dispatcher_header_t m_header;
}; // Size: 0x18

struct kprocessor_state_t {
    struct {
        std::uint64_t m_rax;
        std::uint64_t m_rbx;
        std::uint64_t m_rcx;
        std::uint64_t m_rdx;
        std::uint64_t m_rsi;
        std::uint64_t m_rdi;
        std::uint64_t m_rbp;
        std::uint64_t m_rsp;
        std::uint64_t m_r8;
        std::uint64_t m_r9;
        std::uint64_t m_r10;
        std::uint64_t m_r11;
        std::uint64_t m_r12;
        std::uint64_t m_r13;
        std::uint64_t m_r14;
        std::uint64_t m_r15;
        std::uint64_t m_rip;
        std::uint64_t m_rflags;
        std::uint64_t m_cs;
        std::uint64_t m_ss;
        std::uint64_t m_ds;
        std::uint64_t m_es;
        std::uint64_t m_fs;
        std::uint64_t m_gs;
    } m_context_frame;
    std::uint16_t m_segment_registers[ 6 ];
    std::uint32_t m_reserved[ 6 ];
};

struct kspin_lock_queue_t {
    void* m_next;
    void* m_lock;
};

struct slist_header_t {
    union {
        std::uint64_t m_alignment;
        struct {
            single_list_entry_t m_next;
            std::uint16_t m_depth;
            std::uint16_t m_sequence;
        };
    };
};

enum class pool_type_t : std::uint32_t {
    non_paged_pool,
    paged_pool,
    non_paged_pool_must_succeed,
    dont_use_this_type,
    non_paged_pool_cache_aligned,
    paged_pool_cache_aligned,
    non_paged_pool_cache_aligned_must_s,
    max_pool_type,
    non_paged_pool_base = 0,
    non_paged_pool_base_must_succeed = 2,
    non_paged_pool_base_cache_aligned = 4,
    non_paged_pool_base_cache_aligned_must_s = 6
};

enum class processor_cache_type_t : std::uint32_t {
    unified = 0,
    instruction = 1,
    data = 2,
    trace = 3
};

struct cache_descriptor_t {
    std::uint8_t m_level;
    std::uint8_t m_associativity;
    std::uint16_t m_line_size;
    std::uint32_t m_size;
    processor_cache_type_t m_type;
};

struct pp_lookaside_list_t {
    struct {
        slist_header_t m_list_head;
        std::uint16_t m_depth;
        std::uint16_t m_maximum_depth;
        std::uint32_t m_total_allocates;
        union {
            std::uint32_t m_allocate_misses;
            std::uint32_t m_allocate_hits;
        };
        std::uint32_t m_total_frees;
        union {
            std::uint32_t m_free_misses;
            std::uint32_t m_free_hits;
        };
        pool_type_t m_pool_type;
        std::uint32_t m_tag;
        std::uint32_t m_size;
        void* m_allocate_ex;
        void* m_free_ex;
        list_entry_t m_list_entry;
    } m_p;
    struct {
        std::uint32_t m_total_allocates;
        union {
            std::uint32_t m_allocate_misses;
            std::uint32_t m_allocate_hits;
        };
        std::uint32_t m_total_frees;
        union {
            std::uint32_t m_free_misses;
            std::uint32_t m_free_hits;
        };
    } m_l;
};

struct general_lookaside_pool_t {
    slist_header_t m_list_head;
    std::uint16_t m_depth;
    std::uint16_t m_maximum_depth;
    std::uint32_t m_total_allocates;
    union {
        std::uint32_t m_allocate_misses;
        std::uint32_t m_allocate_hits;
    };
    std::uint32_t m_total_frees;
    union {
        std::uint32_t m_free_misses;
        std::uint32_t m_free_hits;
    };
    pool_type_t m_pool_type;
    std::uint32_t m_tag;
    std::uint32_t m_size;
    void* m_allocate_ex;
    void* m_free_ex;
    list_entry_t m_list_entry;
};

struct kstatic_affinity_block_t {
    std::uint64_t m_bitmap[ 64 ];
};

struct kdpc_data_t {
    list_entry_t m_dpc_list_head;
    std::uint32_t m_dpc_list_lock;
    std::uint32_t m_dpc_queue_depth;
    std::uint32_t m_dpc_count;
};

struct ktimer_table_t {
    std::uint64_t m_timer_expiry;
    kdpc_t m_timer_dpc;
    std::uint64_t m_timer_entries[ 256 ];
};

struct rtl_rb_tree_t {
    void* m_root;
    void* m_min;
};

struct kaffinity_ex_t {
    std::uint16_t m_count;          // +0x000 Count
    std::uint16_t m_size;           // +0x002 Size
    std::uint16_t m_reserved;       // +0x004 Reserved
    std::uint16_t m_maximum;        // +0x006 Maximum
    std::uint64_t m_bitmap[ 20 ];     // +0x008 Bitmap array
};  // Size: 0xA8 bytes

typedef struct ppm_selection_menu_t {
    std::uint32_t   m_count;         // number of selectable idle states
    std::uint32_t   m_next_index;    // next candidate index
    std::uint32_t   m_prev_index;    // previous candidate index
    std::uint32_t   m_selected_index; // currently selected
    std::uint32_t   m_padding;       // alignment pad
} ppm_selection_menu_t;

typedef struct ppm_coordinated_selection_t {
    std::uint32_t   m_selected_state;       // the chosen coordinated idle state
    std::uint32_t   m_target_processors;    // mask or count of processors targeted
    std::uint64_t   m_initiate_wake_stamp;  // timestamp when wake initiated
    std::uint32_t   m_flags;                // coordination flags
    std::uint32_t   m_padding;              // pad to align to 8 bytes
} ppm_coordinated_selection_t;

typedef struct perf_info_ppm_state_selection_t {
    std::uint32_t   m_state_index;     // which idle state is being traced
    std::uint32_t   m_latency;         // exit latency in this state
    std::uint32_t   m_residency;       // expected residency time
    std::uint32_t   m_flags;           // flags for tracing/coordination
} perf_info_ppm_state_selection_t;

typedef struct processor_idle_prepare_info_t {
    std::uint32_t   m_state_index;     // (input) which idle state is being prepared
    std::uint32_t   m_target_residency; // (input) desired residency in this state
    std::uint32_t   m_flags;           // (input) flags controlling preparation
    std::uint32_t   m_exit_latency;    // measured or estimated exit latency
    std::uint64_t   m_start_time;      // timestamp when idle was entered
    std::uint64_t   m_end_time;        // timestamp when idle exited
    void* m_context;         // driver-provided or system context pointer
} processor_idle_prepare_info_t;

struct ppm_idle_states_t {
    std::uint8_t     m_interface_version;                // 0x0
    std::uint8_t     m_idle_override;                    // 0x1
    std::uint8_t     m_estimate_idle_duration;           // 0x2
    std::uint8_t     m_exit_latency_trace_enabled;       // 0x3
    std::uint8_t     m_non_interruptible_transition;     // 0x4
    std::uint8_t     m_unaccounted_transition;           // 0x5
    std::uint8_t     m_idle_duration_limited;            // 0x6
    std::uint8_t     m_idle_check_limited;               // 0x7
    std::uint8_t     m_strict_veto_bias;                 // 0x8
    std::uint32_t    m_exit_latency_countdown;           // 0xc
    std::uint32_t    m_target_state;                     // 0x10
    std::uint32_t    m_actual_state;                     // 0x14
    std::uint32_t    m_old_state;                        // 0x18
    std::uint32_t    m_override_index;                   // 0x1c
    std::uint32_t    m_processor_idle_count;             // 0x20
    std::uint32_t    m_type;                             // 0x24
    std::uint64_t    m_level_id;                         // 0x28
    std::uint64_t    m_reason_flags;                     // 0x30
    volatile std::uint64_t m_initiate_wake_stamp;        // 0x38
    std::uint32_t    m_previous_status;                  // 0x40
    std::uint32_t    m_previous_cancel_reason;           // 0x44
    kaffinity_ex_t   m_primary_processor_mask;           // 0x48
    kaffinity_ex_t   m_secondary_processor_mask;         // 0xf0

    void ( *m_idle_prepare )( struct processor_idle_prepare_info_t* arg1 ); // 0x198
    long ( *m_idle_pre_execute )( void* arg1, std::uint32_t arg2, std::uint32_t arg3,
        std::uint32_t arg4, std::uint32_t* arg5 );           // 0x1a0
    long ( *m_idle_execute )( void* arg1, std::uint64_t arg2, std::uint32_t arg3,
        std::uint32_t arg4, std::uint32_t arg5, std::uint32_t arg6,
        std::uint32_t* arg7 );                                   // 0x1a8
    std::uint32_t( *m_idle_preselect )( void* arg1, struct processor_idle_constraints_t* arg2 ); // 0x1b0
    std::uint32_t( *m_idle_test )( void* arg1, std::uint32_t arg2, std::uint32_t arg3 );         // 0x1b8
    std::uint32_t( *m_idle_availability_check )( void* arg1, std::uint32_t arg2 );               // 0x1c0
    void ( *m_idle_complete )( void* arg1, std::uint32_t arg2, std::uint32_t arg3,
        std::uint32_t arg4, std::uint32_t* arg5 );             // 0x1c8
    void ( *m_idle_cancel )( void* arg1, std::uint32_t arg2 );                        // 0x1d0
    std::uint8_t( *m_idle_is_halted )( void* arg1 );                                 // 0x1d8
    std::uint8_t( *m_idle_initiate_wake )( void* arg1 );                             // 0x1e0

    processor_idle_prepare_info_t   m_prepare_info;        // 0x1e8
    kaffinity_ex_t                  m_deep_idle_snapshot;  // 0x240
    perf_info_ppm_state_selection_t* m_tracing;            // 0x2e8
    perf_info_ppm_state_selection_t* m_coordinated_tracing;// 0x2f0
    ppm_selection_menu_t            m_processor_menu;      // 0x2f8
    ppm_selection_menu_t            m_coordinated_menu;    // 0x308
    ppm_coordinated_selection_t     m_coordinated_selection; // 0x318
    ppm_idle_states_t* m_state[ 1 ];            // 0x330
};

struct processor_power_state_t {
    ppm_idle_states_t* m_idle_states;
    std::uint32_t m_idle_state_max;
    std::uint32_t m_last_idle_check;
    std::uint32_t m_last_thermal_interval;
    struct {
        std::uint32_t m_idle_check : 1;
        std::uint32_t m_thermal_check : 1;
        std::uint32_t m_thermal_active : 1;
        std::uint32_t m_reserved : 29;
    } m_flags;
    std::uint32_t m_last_idle_duration;
    std::uint32_t m_idle_sum;
    std::uint32_t m_idle_count;
    std::uint32_t m_idle_average;
    std::uint32_t m_thermal_sum;
    std::uint32_t m_thermal_count;
    std::uint32_t m_thermal_average;
    std::uint32_t m_thermal_interval;
    std::uint32_t m_reserved[ 8 ];
};

struct kgate_t {
    dispatcher_header_t m_header;
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

union ularge_integer_t {
    struct {
        std::uint32_t  m_low_part;                                                      //0x0
        std::uint32_t  m_high_part;                                                     //0x4
    };
    struct {
        std::uint32_t  m_low_part;                                                      //0x0
        std::uint32_t  m_high_part;                                                     //0x4
    } u;                                                                    //0x0
    std::uint64_t m_quad_part;                                                     //0x0
};

struct physical_memory_range_t {
    ularge_integer_t m_base_page;
    ularge_integer_t m_page_count;
}; // Size: 0x010

struct mm_copy_address_t {
    union {
        std::uint64_t m_virtual_address;    // +0x000
        physical_address_t m_physical_address;   // +0x000
    };
}; // Size: 0x008

struct synch_counters_t {
    std::uint32_t m_spinlock_acquire;
    std::uint32_t m_spinlock_content;
    std::uint32_t m_spinlock_spin;
    std::uint32_t m_kevent;
    std::uint32_t m_kevent_level;
    std::uint32_t m_kevent_spinlock_spin;
    std::uint32_t m_kmutex_acquire;
    std::uint32_t m_kmutex_content;
    std::uint32_t m_kmutex_spin;
    std::uint32_t m_fast_mutex_acquire;
    std::uint32_t m_fast_mutex_content;
    std::uint32_t m_fast_mutex_spin;
    std::uint32_t m_guarded_mutex_acquire;
    std::uint32_t m_guarded_mutex_content;
    std::uint32_t m_guarded_mutex_spin;
};

struct filesystem_disk_counters_t {
    std::uint32_t m_fs_read_operations;
    std::uint32_t m_fs_write_operations;
    std::uint32_t m_fs_other_operations;
    std::uint32_t m_fs_read_bytes;
    std::uint32_t m_fs_write_bytes;
    std::uint32_t m_fs_other_bytes;
};

struct xmm_save_area32_t {
    std::uint16_t m_control_word;
    std::uint16_t m_status_word;
    std::uint8_t m_tag_word;
    std::uint8_t m_reserved1;
    std::uint16_t m_error_opcode;
    std::uint32_t m_error_offset;
    std::uint16_t m_error_selector;
    std::uint16_t m_reserved2;
    std::uint32_t m_data_offset;
    std::uint16_t m_data_selector;
    std::uint16_t m_reserved3;
    std::uint32_t m_mx_csr;
    std::uint32_t m_mx_csr_mask;
    std::m128a_t m_float_registers[ 8 ];
    std::m128a_t m_xmm_registers[ 16 ];
    std::uint8_t m_reserved4[ 96 ];
};

struct context_t {
    std::uint64_t m_p1_home;                    // +0x000
    std::uint64_t m_p2_home;                    // +0x008
    std::uint64_t m_p3_home;                    // +0x010
    std::uint64_t m_p4_home;                    // +0x018
    std::uint64_t m_p5_home;                    // +0x020
    std::uint64_t m_p6_home;                    // +0x028
    std::uint32_t m_context_flags;              // +0x030
    std::uint32_t m_mx_csr;                     // +0x034
    std::uint16_t m_seg_cs;                     // +0x038
    std::uint16_t m_seg_ds;                     // +0x03A
    std::uint16_t m_seg_es;                     // +0x03C
    std::uint16_t m_seg_fs;                     // +0x03E
    std::uint16_t m_seg_gs;                     // +0x040
    std::uint16_t m_seg_ss;                     // +0x042
    std::uint32_t m_eflags;                     // +0x044
    std::uint64_t m_dr0;                        // +0x048
    std::uint64_t m_dr1;                        // +0x050
    std::uint64_t m_dr2;                        // +0x058
    std::uint64_t m_dr3;                        // +0x060
    std::uint64_t m_dr6;                        // +0x068
    std::uint64_t m_dr7;                        // +0x070
    std::uint64_t m_rax;                        // +0x078
    std::uint64_t m_rcx;                        // +0x080
    std::uint64_t m_rdx;                        // +0x088
    std::uint64_t m_rbx;                        // +0x090
    std::uint64_t m_rsp;                        // +0x098
    std::uint64_t m_rbp;                        // +0x0A0
    std::uint64_t m_rsi;                        // +0x0A8
    std::uint64_t m_rdi;                        // +0x0B0
    std::uint64_t m_r8;                         // +0x0B8
    std::uint64_t m_r9;                         // +0x0C0
    std::uint64_t m_r10;                        // +0x0C8
    std::uint64_t m_r11;                        // +0x0D0
    std::uint64_t m_r12;                        // +0x0D8
    std::uint64_t m_r13;                        // +0x0E0
    std::uint64_t m_r14;                        // +0x0E8
    std::uint64_t m_r15;                        // +0x0F0
    std::uint64_t m_rip;                        // +0x0F8
    union {
        xmm_save_area32_t m_flt_save;          // +0x100
        struct {
            std::m128a_t m_header[ 2 ];
            std::m128a_t m_legacy[ 8 ];
            std::m128a_t m_xmm0;
            std::m128a_t m_xmm1;
            std::m128a_t m_xmm2;
            std::m128a_t m_xmm3;
            std::m128a_t m_xmm4;
            std::m128a_t m_xmm5;
            std::m128a_t m_xmm6;
            std::m128a_t m_xmm7;
            std::m128a_t m_xmm8;
            std::m128a_t m_xmm9;
            std::m128a_t m_xmm10;
            std::m128a_t m_xmm11;
            std::m128a_t m_xmm12;
            std::m128a_t m_xmm13;
            std::m128a_t m_xmm14;
            std::m128a_t m_xmm15;
        };
    };
    std::m128a_t m_vector_register[ 26 ];             // +0x300
    std::uint64_t m_vector_control;            // +0x4A0
    std::uint64_t m_debug_control;             // +0x4A8
    std::uint64_t m_last_branch_to_rip;        // +0x4B0
    std::uint64_t m_last_branch_from_rip;      // +0x4B8
    std::uint64_t m_last_exception_to_rip;     // +0x4C0
    std::uint64_t m_last_exception_from_rip;   // +0x4C8
};

struct kentropy_timing_state_t {
    std::uint64_t m_enter_time;
    std::uint64_t m_enter_cycles;
    std::uint64_t m_reserved[ 2 ];
};

struct ksecure_fault_information_t {
    std::uint32_t m_fault_type;
    std::uint32_t m_reserved;
    std::uint64_t m_virtual_address;
};

struct kshared_ready_queue_t {
    std::uint32_t m_lock;
    std::uint32_t m_owner;
    std::uint32_t m_current_size;
    std::uint32_t m_maximum_size;
    list_entry_t m_list_head;
};

struct ktimer_expiration_trace_t {
    std::uint64_t m_time;
    void* m_thread;
};

struct machine_check_context_t {
    std::uint32_t m_version_id;
    std::uint32_t m_check_type;
    std::uint32_t m_processor_number;
    std::uint32_t m_bank_number;
    std::uint64_t m_address;
    std::uint64_t m_misc;
};

struct klock_queue_handle_t {
    kspin_lock_queue_t m_lock_queue;
    std::uint8_t m_old_irql;
};

struct request_mailbox_t {
    std::uint64_t m_next;
    std::uint32_t m_request_type;
    std::uint32_t m_request_flags;
    list_entry_t m_request_list_entry;
    std::uint64_t m_request_context;
    union {
        struct {
            std::uint32_t m_processor_number;
            std::uint32_t m_node_number;
        };
        std::uint64_t m_target_object;
    };
    std::uint64_t m_reserved[ 2 ];
};

struct kmutant_t {
    dispatcher_header_t header;                                       //0x0
    list_entry_t mutant_list_entry;                                     //0x18
    kthread_t* owner_thread;                                           //0x28
    union {
        std::uint8_t mutant_flags;                                                  //0x30
        struct {
            std::uint8_t abandoned : 1;                                              //0x30
            std::uint8_t spare1 : 7;                                                 //0x30
            std::uint8_t abandoned2 : 1;                                             //0x30
            std::uint8_t ab_enabled : 1;                                              //0x30
            std::uint8_t spare2 : 6;                                                 //0x30
        };
    };
    std::uint8_t apc_disable;                                                       //0x31
};

struct kprcb_t {
    std::uint32_t m_mxcsr;                              // +0x000
    std::uint8_t m_legacy_number;                       // +0x004
    std::uint8_t m_reserved_must_be_zero;               // +0x005
    std::uint8_t m_interrupt_request;                   // +0x006
    std::uint8_t m_idle_halt;                          // +0x007
    kthread_t* m_current_thread;                             // +0x008
    kthread_t* m_next_thread;                               // +0x010
    kthread_t* m_idle_thread;                               // +0x018
    std::uint8_t m_nesting_level;                      // +0x020
    std::uint8_t m_clock_owner;                        // +0x021
    std::uint8_t m_pad_0x22;                          // +0x022
    std::uint8_t m_idle_state;                         // +0x023
    std::uint32_t m_number;                            // +0x024
    std::uint64_t m_rsp_base;                         // +0x028
    std::uint64_t m_prcb_lock;                        // +0x030
    void* m_priority_state;                           // +0x038
    std::uint8_t m_cpu_type;                          // +0x040
    std::uint8_t m_cpu_id;                            // +0x041
    std::uint16_t m_pad_0x42;                         // +0x042
    std::uint32_t m_mhz;                              // +0x044
    std::uint64_t m_hal_reserved[ 8 ];                  // +0x048
    std::uint16_t m_minor_version;                    // +0x088
    std::uint16_t m_major_version;                    // +0x08A
    std::uint8_t m_build_type;                        // +0x08C
    std::uint8_t m_cpu_vendor;                        // +0x08D
    std::uint8_t m_cores_per_physical_processor;      // +0x08E
    std::uint8_t m_logical_processors_per_core;       // +0x08F
    std::uint64_t m_tsc_frequency;                    // +0x090
    std::uint32_t m_cores_per_physical_processor32;   // +0x098
    std::uint32_t m_logical_processors_per_core32;    // +0x09C
    std::uint64_t m_prcb_pad04[ 4 ];                   // +0x0A0
    void* m_parent_node;                             // +0x0C0
    std::uint64_t m_group_set_member;                // +0x0C8
    std::uint8_t m_group;                            // +0x0D0
    std::uint8_t m_group_index;                      // +0x0D1
    std::uint8_t m_pad_0xD2[ 2 ];                      // +0x0D2
    std::uint32_t m_initial_apic_id;                 // +0x0D4
    std::uint32_t m_scb_offset;                      // +0x0D8
    std::uint32_t m_apic_mask;                       // +0x0DC
    void* m_acpi_reserved;                           // +0x0E0
    std::uint32_t m_cflush_size;                     // +0x0E8
    std::uint32_t m_prcb_flags;                       // +0x0EC
    std::uint64_t m_prcb_pad11[ 2 ];                   // +0x0F0
    kprocessor_state_t m_processor_state;             // +0x100
    void* m_extended_supervisor_state;                // +0x6C0
    std::uint32_t m_processor_signature;              // +0x6C8
    std::uint32_t m_processor_flags;                  // +0x6CC
    std::uint64_t m_pad_0x6D0;                       // +0x6D0
    std::uint64_t m_pad_0x6D8;                       // +0x6D8
    kspin_lock_queue_t m_lock_queue[ 17 ];             // +0x6F0
    pp_lookaside_list_t m_pp_lookaside_list[ 16 ];     // +0x800
    general_lookaside_pool_t m_pp_nx_paged_lookaside_list[ 32 ];  // +0x900
    general_lookaside_pool_t m_pp_n_paged_lookaside_list[ 32 ];   // +0x1500
    general_lookaside_pool_t m_pp_paged_lookaside_list[ 32 ];     // +0x2100
    std::uint64_t m_msr_ia32_tsx_ctrl;               // +0x2D00
    single_list_entry_t m_deferred_ready_list_head;  // +0x2D08
    std::uint32_t m_mm_page_fault_count;             // +0x2D10
    std::uint32_t m_mm_copy_on_write_count;          // +0x2D14
    std::uint32_t m_mm_transition_count;             // +0x2D18
    std::uint32_t m_mm_demand_zero_count;            // +0x2D1C
    std::uint32_t m_mm_page_read_count;              // +0x2D20
    std::uint32_t m_mm_page_read_io_count;           // +0x2D24
    std::uint32_t m_mm_dirty_pages_write_count;      // +0x2D28
    std::uint32_t m_mm_dirty_write_io_count;         // +0x2D2C
    std::uint32_t m_mm_mapped_pages_write_count;     // +0x2D30
    std::uint32_t m_mm_mapped_write_io_count;        // +0x2D34
    std::uint32_t m_ke_system_calls;                 // +0x2D38
    std::uint32_t m_ke_context_switches;             // +0x2D3C
    std::uint32_t m_prcb_pad40;                      // +0x2D40
    std::uint32_t m_cc_fast_read_no_wait;            // +0x2D44
    std::uint32_t m_cc_fast_read_wait;               // +0x2D48
    std::uint32_t m_cc_fast_read_not_possible;       // +0x2D4C
    std::uint32_t m_cc_copy_read_no_wait;            // +0x2D50
    std::uint32_t m_cc_copy_read_wait;               // +0x2D54
    std::uint32_t m_cc_copy_read_no_wait_miss;       // +0x2D58
    std::uint32_t m_io_read_operation_count;         // +0x2D5C
    std::uint32_t m_io_write_operation_count;        // +0x2D60
    std::uint32_t m_io_other_operation_count;        // +0x2D64
    ularge_integer_t m_io_read_transfer_count;        // +0x2D68
    ularge_integer_t m_io_write_transfer_count;       // +0x2D70
    ularge_integer_t m_io_other_transfer_count;       // +0x2D78
    std::uint32_t m_packet_barrier;                  // +0x2D80
    std::uint32_t m_target_count;                    // +0x2D84
    std::uint32_t m_ipi_frozen;                      // +0x2D88
    std::uint32_t m_prcb_pad30;                      // +0x2D8C
    void* m_isr_dpc_stats;                          // +0x2D90
    std::uint32_t m_device_interrupts;               // +0x2D98
    std::uint32_t m_lookaside_irp_float;            // +0x2D9C
    std::uint32_t m_interrupt_last_count;            // +0x2DA0
    std::uint32_t m_interrupt_rate;                  // +0x2DA4
    std::uint64_t m_prcb_pad31;                     // +0x2DA8
    void* m_pair_prcb;                              // +0x2DB0
    kstatic_affinity_block_t m_static_affinity;     // +0x2DB8
    std::uint64_t m_prcb_pad35[ 5 ];                  // +0x3058
    slist_header_t m_interrupt_object_pool;          // +0x3080
    void* m_dpc_runtime_history_hash_table;         // +0x3090
    void* m_dpc_runtime_history_hash_table_cleanup_dpc; // +0x3098
    void* m_current_dpc_routine;                    // +0x30A0
    std::uint64_t m_current_dpc_runtime_history_cached; // +0x30A8
    std::uint64_t m_current_dpc_start_time;         // +0x30B0
    std::uint64_t m_prcb_pad41;                     // +0x30B8
    kdpc_data_t m_dpc_data[ 2 ];                      // +0x30C0
    void* m_dpc_stack;                              // +0x3110
    std::uint32_t m_maximum_dpc_queue_depth;        // +0x3118
    std::uint32_t m_dpc_request_rate;               // +0x311C
    std::uint32_t m_minimum_dpc_rate;               // +0x3120
    std::uint32_t m_dpc_last_count;                 // +0x3124
    std::uint8_t m_thread_dpc_enable;               // +0x3128
    std::uint8_t m_quantum_end;                     // +0x3129
    std::uint8_t m_dpc_routine_active;              // +0x312A
    std::uint8_t m_idle_schedule;                   // +0x312B
    std::uint32_t m_pad_0x312C;                    // +0x312C
    std::uint32_t m_prcb_pad93;                    // +0x3130
    std::uint32_t m_last_tick;                      // +0x3134
    std::uint32_t m_clock_interrupts;               // +0x3138
    std::uint32_t m_ready_scan_tick;                // +0x313C
    void* m_interrupt_object[ 256 ];                  // +0x3140
    ktimer_table_t m_timer_table;                   // +0x3940
    std::uint32_t m_prcb_pad92[ 10 ];                // +0x7B58
    kgate_t m_dpc_gate;                            // +0x7B80
    void* m_prcb_pad52;                            // +0x7B98
    kdpc_t m_call_dpc;                             // +0x7BA0
    std::uint32_t m_clock_keep_alive;               // +0x7BE0
    std::uint8_t m_prcb_pad60[ 2 ];                  // +0x7BE2
    std::uint16_t m_pad_0x7BE6;                    // +0x7BE6
    std::uint32_t m_dpc_watchdog_period;            // +0x7BE8
    std::uint32_t m_dpc_watchdog_count;             // +0x7BEC
    std::uint32_t m_ke_spinlock_ordering;           // +0x7BF0
    std::uint32_t m_dpc_watchdog_profile_cumulative_dpc_threshold; // +0x7BF4
    void* m_cached_ptes;                           // +0x7BF8
    list_entry_t m_wait_list_head;                 // +0x7C00
    std::uint64_t m_wait_lock;                     // +0x7C10
    std::uint32_t m_ready_summary;                  // +0x7C18
    std::uint32_t m_affinitized_selection_mask;     // +0x7C1C
    std::uint32_t m_queue_index;                    // +0x7C20
    std::uint32_t m_prcb_pad75[ 2 ];                 // +0x7C24
    std::uint32_t m_dpc_watchdog_sequence_number;   // +0x7C2C
    kdpc_t m_timer_expiration_dpc;                 // +0x7C30
    rtl_rb_tree_t m_scb_queue;                     // +0x7C70
    list_entry_t m_dispatcher_ready_list_head[ 32 ];  // +0x7C80
    std::uint32_t m_interrupt_count;                // +0x7E80
    std::uint32_t m_kernel_time;                    // +0x7E84
    std::uint32_t m_user_time;                      // +0x7E88
    std::uint32_t m_dpc_time;                      // +0x7E8C
    std::uint32_t m_interrupt_time;                 // +0x7E90
    std::uint32_t m_adjust_dpc_threshold;           // +0x7E94
    std::uint8_t m_debugger_saved_irql;            // +0x7E98
    std::uint8_t m_group_scheduling_over_quota;     // +0x7E99
    std::uint8_t m_deep_sleep;                     // +0x7E9A
    std::uint8_t m_prcb_pad80;                     // +0x7E9B
    std::uint32_t m_dpc_time_count;                // +0x7E9C
    std::uint32_t m_dpc_time_limit;                // +0x7EA0
    std::uint32_t m_periodic_count;                 // +0x7EA4
    std::uint32_t m_periodic_bias;                  // +0x7EA8
    std::uint32_t m_available_time;                 // +0x7EAC
    std::uint32_t m_ke_exception_dispatch_count;    // +0x7EB0
    std::uint32_t m_ready_thread_count;             // +0x7EB4
    std::uint64_t m_ready_queue_expected_run_time;  // +0x7EB8
    std::uint64_t m_start_cycles;                   // +0x7EC0
    std::uint64_t m_tagged_cycles_start;            // +0x7EC8
    std::uint64_t m_tagged_cycles[ 3 ];               // +0x7ED0
    std::uint64_t m_affinitized_cycles;             // +0x7EE8
    std::uint64_t m_important_cycles;               // +0x7EF0
    std::uint64_t m_unimportant_cycles;             // +0x7EF8ye
    std::uint32_t m_dpc_watchdog_profile_single_dpc_threshold; // +0x7F00
    std::uint32_t m_mm_spinlock_ordering;           // +0x7F04
    void* m_cached_stack;                          // +0x7F08
    std::uint32_t m_page_color;                    // +0x7F10
    std::uint32_t m_node_color;                    // +0x7F14
    std::uint32_t m_node_shifted_color;            // +0x7F18
    std::uint32_t m_secondary_color_mask;          // +0x7F1C
    std::uint8_t m_prcb_pad81[ 6 ];                 // +0x7F20
    std::uint8_t m_exception_stack_active;         // +0x7F26
    std::uint8_t m_tb_flush_list_active;          // +0x7F27
    void* m_exception_stack;                       // +0x7F28
    std::uint64_t m_prcb_pad82;                   // +0x7F30
    std::uint64_t m_cycle_time;                    // +0x7F38
    std::uint64_t m_cycles[ 8 ];                     // +0x7F40
    std::uint32_t m_cc_fast_mdl_read_no_wait;      // +0x7F80
    std::uint32_t m_cc_fast_mdl_read_wait;         // +0x7F84
    std::uint32_t m_cc_fast_mdl_read_not_possible; // +0x7F88
    std::uint32_t m_cc_map_data_no_wait;           // +0x7F8C
    std::uint32_t m_cc_map_data_wait;              // +0x7F90
    std::uint32_t m_cc_pin_mapped_data_count;      // +0x7F94
    std::uint32_t m_cc_pin_read_no_wait;           // +0x7F98
    std::uint32_t m_cc_pin_read_wait;              // +0x7F9C
    std::uint32_t m_cc_mdl_read_no_wait;           // +0x7FA0
    std::uint32_t m_cc_mdl_read_wait;              // +0x7FA4
    std::uint32_t m_cc_lazy_write_hot_spots;       // +0x7FA8
    std::uint32_t m_cc_lazy_write_ios;             // +0x7FAC
    std::uint32_t m_cc_lazy_write_pages;           // +0x7FB0
    std::uint32_t m_cc_data_flushes;               // +0x7FB4
    std::uint32_t m_cc_data_pages;                 // +0x7FB8
    std::uint32_t m_cc_lost_delayed_writes;        // +0x7FBC
    std::uint32_t m_cc_fast_read_resource_miss;     // +0x7FC0
    std::uint32_t m_cc_copy_read_wait_miss;         // +0x7FC4
    std::uint32_t m_cc_fast_mdl_read_resource_miss; // +0x7FC8
    std::uint32_t m_cc_map_data_no_wait_miss;       // +0x7FCC
    std::uint32_t m_cc_map_data_wait_miss;          // +0x7FD0
    std::uint32_t m_cc_pin_read_no_wait_miss;       // +0x7FD4
    std::uint32_t m_cc_pin_read_wait_miss;          // +0x7FD8
    std::uint32_t m_cc_mdl_read_no_wait_miss;       // +0x7FDC
    std::uint32_t m_cc_mdl_read_wait_miss;          // +0x7FE0
    std::uint32_t m_cc_read_ahead_ios;              // +0x7FE4
    std::uint32_t m_mm_cache_transition_count;       // +0x7FE8
    std::uint32_t m_mm_cache_read_count;            // +0x7FEC
    std::uint32_t m_mm_cache_io_count;              // +0x7FF0
    std::uint32_t m_prcb_pad91;                     // +0x7FF4
    void* m_mm_internal;                            // +0x7FF8
    processor_power_state_t m_power_state;          // +0x8000
    void* m_hyper_pte;                             // +0x8200
    list_entry_t m_scb_list;                       // +0x8208
    kdpc_t m_force_idle_dpc;                       // +0x8218
    kdpc_t m_dpc_watchdog_dpc;                     // +0x8258
    ktimer_t m_dpc_watchdog_timer;                 // +0x8298
    cache_descriptor_t m_cache[ 5 ];                  // +0x82D8
    std::uint32_t m_cache_count;                    // +0x8314
    std::uint32_t m_cached_commit;                  // +0x8318
    std::uint32_t m_cached_resident_available;      // +0x831C
    void* m_whea_info;                             // +0x8320
    void* m_etw_support;                           // +0x8328
    void* m_ex_sa_page_array;                      // +0x8330
    std::uint32_t m_ke_alignment_fixup_count;       // +0x8338
    std::uint32_t m_prcb_pad95;                     // +0x833C
    slist_header_t m_hypercall_page_list;           // +0x8340
    void* m_statistics_page;                        // +0x8350
    std::uint64_t m_generation_target;              // +0x8358
    std::uint64_t m_prcb_pad85[ 4 ];                 // +0x8360
    void* m_hypercall_cached_pages;                // +0x8380
    void* m_virtual_apic_assist;                   // +0x8388
    kaffinity_ex_t m_package_processor_set;        // +0x8390
    std::uint32_t m_package_id;                    // +0x8438
    std::uint32_t m_prcb_pad86;                    // +0x843C
    std::uint64_t m_shared_ready_queue_mask;        // +0x8440
    void* m_shared_ready_queue;                    // +0x8448
    std::uint32_t m_shared_queue_scan_owner;        // +0x8450
    std::uint32_t m_scan_sibling_index;            // +0x8454
    std::uint64_t m_core_processor_set;            // +0x8458
    std::uint64_t m_scan_sibling_mask;             // +0x8460
    std::uint64_t m_llc_mask;                      // +0x8468
    std::uint64_t m_cache_processor_mask[ 5 ];       // +0x8470
    void* m_processor_profile_control_area;        // +0x8498
    void* m_profile_event_index_address;           // +0x84A0
    void* m_dpc_watchdog_profile;                 // +0x84A8
    void* m_dpc_watchdog_profile_current_empty_capture; // +0x84B0
    void* m_scheduler_assist;                      // +0x84B8
    synch_counters_t m_synch_counters;            // +0x84C0
    std::uint64_t m_prcb_pad94;                   // +0x8578
    filesystem_disk_counters_t m_fs_counters;      // +0x8580
    std::uint8_t m_vendor_string[ 13 ];              // +0x8590
    std::uint8_t m_prcb_pad100[ 3 ];                // +0x859D
    std::uint64_t m_feature_bits;                   // +0x85A0
    ularge_integer_t m_update_signature;             // +0x85A8
    std::uint64_t m_pte_bit_cache;                 // +0x85B0
    std::uint32_t m_pte_bit_offset;                // +0x85B8
    std::uint32_t m_prcb_pad105;                   // +0x85BC
    context_t* m_context;                          // +0x85C0
    std::uint32_t m_context_flags_init;            // +0x85C8
    std::uint32_t m_prcb_pad115;                   // +0x85CC
    void* m_extended_state;                        // +0x85D0
    void* m_isr_stack;                            // +0x85D8
    kentropy_timing_state_t m_entropy_timing_state; // +0x85E0
    std::uint64_t m_prcb_pad110;                   // +0x8730
    void* m_stibp_pairing_trace;                   // +0x8738
    single_list_entry_t m_ab_self_io_boosts_list;  // +0x8770
    single_list_entry_t m_ab_propagate_boosts_list; // +0x8778
    kdpc_t m_ab_dpc;                              // +0x8780
    iop_irp_stack_profiler_t m_io_irp_stack_profiler_current;  // +0x87C0
    iop_irp_stack_profiler_t m_io_irp_stack_profiler_previous; // +0x8814
    ksecure_fault_information_t m_secure_fault;    // +0x8868
    std::uint64_t m_prcb_pad120;                   // +0x8878
    kshared_ready_queue_t m_local_shared_ready_queue; // +0x8880
    std::uint64_t m_prcb_pad125[ 2 ];                // +0x8AF0
    std::uint32_t m_timer_expiration_trace_count;   // +0x8B00
    std::uint32_t m_prcb_pad127;                   // +0x8B04
    ktimer_expiration_trace_t m_timer_expiration_trace[ 16 ]; // +0x8B08
    std::uint64_t m_prcb_pad128[ 7 ];                // +0x8C08
    void* m_mailbox;                              // +0x8C40
    std::uint64_t m_prcb_pad130[ 7 ];                // +0x8C48
    machine_check_context_t m_mcheck_context[ 2 ];    // +0x8C80
    std::uint64_t m_prcb_pad134[ 4 ];                // +0x8D20
    klock_queue_handle_t m_selfmap_lock_handle[ 4 ];  // +0x8D40
    std::uint64_t m_prcb_pad134a[ 4 ];               // +0x8DA0
    std::uint8_t m_prcb_pad138[ 128 ];               // +0x8DC0
    std::uint8_t m_prcb_pad138a[ 64 ];               // +0x8E40
    std::uint64_t m_kernel_directory_table_base;    // +0x8E80
    std::uint64_t m_rsp_base_shadow;               // +0x8E88
    std::uint64_t m_user_rsp_shadow;               // +0x8E90
    std::uint32_t m_shadow_flags;                  // +0x8E98
    std::uint32_t m_prcb_pad138b;                  // +0x8E9C
    std::uint64_t m_prcb_pad138c;                  // +0x8EA0
    std::uint16_t m_prcb_pad138d;                  // +0x8EA8
    std::uint16_t m_verw_selector;                 // +0x8EAA
    std::uint32_t m_dbg_mce_nesting_level;         // +0x8EAC
    std::uint32_t m_dbg_mce_flags;                 // +0x8EB0
    std::uint32_t m_prcb_pad139b;                  // +0x8EB4
    std::uint64_t m_prcb_pad140[ 505 ];              // +0x8EB8
    std::uint64_t m_prcb_pad140a[ 8 ];               // +0x9E80
    std::uint64_t m_prcb_pad141[ 504 ];              // +0x9EC0
    std::uint8_t m_prcb_pad141a[ 64 ];               // +0xAE80
    request_mailbox_t m_request_mailbox;           // +0xAEC0
}; // Size: 0xAF00

struct mi_active_pfn_t {
    std::uint64_t m_page_frame : 40;
    std::uint64_t m_priority : 8;
    std::uint64_t m_color : 16;
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

struct xsave_format_t {
    std::uint16_t control_word;
    std::uint16_t status_word;
    std::uint8_t tag_word;
    std::uint8_t reserved1;
    std::uint16_t error_opcode;
    std::uint32_t error_offset;
    std::uint16_t error_selector;
    std::uint16_t reserved2;
    std::uint32_t data_offset;
    std::uint16_t data_selector;
    std::uint16_t reserved3;
    std::uint32_t mx_csr;
    std::uint32_t mx_csr_mask;
    std::m128a_t float_registers[ 8 ];
    std::m128a_t xmm_registers[ 16 ];
    std::uint8_t reserved4[ 96 ];
};

struct exception_record_t {
    std::uint32_t m_exception_code;
    std::uint32_t m_exception_flags;
    exception_record_t* m_exception_record;
    void* m_exception_address;
    std::uint32_t m_number_parameters;
    std::uintptr_t m_exception_information[ 15 ];
};

struct kexception_frame_t {
    // Home address for the parameter registers.
    std::uint64_t m_p1_home;
    std::uint64_t m_p2_home;
    std::uint64_t m_p3_home;
    std::uint64_t m_p4_home;
    std::uint64_t m_p5;
    std::uint64_t m_spare1;

    // Saved nonvolatile floating registers.
    std::m128a_t m_xmm6;
    std::m128a_t m_xmm7;
    std::m128a_t m_xmm8;
    std::m128a_t m_xmm9;
    std::m128a_t m_xmm10;
    std::m128a_t m_xmm11;
    std::m128a_t m_xmm12;
    std::m128a_t m_xmm13;
    std::m128a_t m_xmm14;
    std::m128a_t m_xmm15;

    // Kernel callout frame variables.
    std::uint64_t m_trap_frame;
    std::uint64_t m_output_buffer;
    std::uint64_t m_output_length;
    std::uint64_t m_spare2;

    // Saved MXCSR when a thread is interrupted in kernel mode via a dispatch interrupt.
    std::uint64_t m_mxcsr;

    // Saved nonvolatile register - not always saved.
    std::uint64_t m_rbp;

    // Saved nonvolatile registers.
    std::uint64_t m_rbx;
    std::uint64_t m_rdi;
    std::uint64_t m_rsi;
    std::uint64_t m_r12;
    std::uint64_t m_r13;
    std::uint64_t m_r14;
    std::uint64_t m_r15;

    // EFLAGS and return address.
    std::uint64_t m_return;
};

using p_exception_record = exception_record_t*;

struct knonvolatile_context_pointers_t {
    union {
        struct {
            std::uint64_t* m_rax;
            std::uint64_t* m_rcx;
            std::uint64_t* m_rdx;
            std::uint64_t* m_rbx;
            std::uint64_t* m_rsp;
            std::uint64_t* m_rbp;
            std::uint64_t* m_rsi;
            std::uint64_t* m_rdi;
            std::uint64_t* m_r8;
            std::uint64_t* m_r9;
            std::uint64_t* m_r10;
            std::uint64_t* m_r11;
            std::uint64_t* m_r12;
            std::uint64_t* m_r13;
            std::uint64_t* m_r14;
            std::uint64_t* m_r15;
        };
        std::uint64_t* gp_registers[ 16 ];
    };
    union {
        struct {
            std::m128a_t* xmm0;
            std::m128a_t* xmm1;
            std::m128a_t* xmm2;
            std::m128a_t* xmm3;
            std::m128a_t* xmm4;
            std::m128a_t* xmm5;
            std::m128a_t* xmm6;
            std::m128a_t* xmm7;
            std::m128a_t* xmm8;
            std::m128a_t* xmm9;
            std::m128a_t* xmm10;
            std::m128a_t* xmm11;
            std::m128a_t* xmm12;
            std::m128a_t* xmm13;
            std::m128a_t* xmm14;
            std::m128a_t* xmm15;
        };
        std::m128a_t* fp_registers[ 16 ];
    };
};

struct exception_registration_record_t;

using p_exception_routine = std::uint32_t( __cdecl* )(
    struct exception_record_t* exception_record,
    void* establisher_frame,
    struct context_t* context_record,
    void* dispatcher_context
    );

struct mmpte_t {
    union {
        mmpte_hardware_t m_hard;
        std::uint64_t m_value;
    };
};

struct mipfnblink_t {
    union {
        std::uint64_t m_blink : 40;
        std::uint64_t m_type_size : 24;
    };
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

struct mmpfnentry1_t {
    std::uint8_t m_page_color : 6;
    std::uint8_t m_modified : 1;
    std::uint8_t m_read_in_progress : 1;
};

struct mmpfnentry3_t {
    std::uint8_t priority : 3;
    std::uint8_t on_protected_standby : 1;
    std::uint8_t in_page_error : 1;
    std::uint8_t system_charged_page : 1;
    std::uint8_t removal_requested : 1;
    std::uint8_t parity_error : 1;
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
                    struct {
                        std::uint64_t m_flink : 40;
                        std::uint64_t m_node_flink_low : 24;
                    };
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
        union {
            struct {
                std::uint16_t m_reference_count;
                mmpfnentry1_t m_e1;
                mmpfnentry3_t m_e3;
            };
            struct {
                std::uint16_t m_reference_count2;
            } m_e2;
            struct {
                std::uint32_t m_entire_field;
            } m_e4;
        };
    } m_u3;

    mi_pfn_ulong5_t m_u5;

    union {
        union {
            struct {
                std::uint64_t m_pte_frame : 40;
                std::uint64_t m_resident_page : 1;
                std::uint64_t m_unused1 : 1;
                std::uint64_t m_unused2 : 1;
                std::uint64_t m_partition : 10;
                std::uint64_t m_file_only : 1;
                std::uint64_t m_pfn_exists : 1;
                std::uint64_t m_node_flink_high : 5;
                std::uint64_t m_page_identity : 3;
                std::uint64_t m_prototype_pte : 1;
            };
            std::uint64_t m_entire_field;
        };
    } m_u4;
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

typedef enum e_bound_callback_status {
    bound_exception_continue_search,
    bound_exception_handled,
    bound_exception_error,
    bound_exception_maximum
};

struct ksemaphore_t {
    dispatcher_header_t m_header;                                       //0x0
    LONG m_limit;                                                             //0x18
};

struct rtlp_debug_print_entry_t {
    ex_rundown_ref_t m_lock;
    void* m_unamed1;
    void* m_callback;
    list_entry_t m_list_entry;
};

struct rtlp_debug_print_entry_11_t {
    ex_rundown_ref_t m_lock;
    std::uint64_t m_flags;
    void* m_callback;
    void* m_context;
    list_entry_t m_list_entry;
};

using hvl_invoke_hypercall_t = std::uint16_t( * )( std::uint64_t hypercall_input,
    std::uint64_t input,
    std::uint64_t output );
using switch_virtual_address_space_t = std::uint16_t( * )( std::uint64_t new_cr3 );

typedef
void
( *p_create_process_notify_routine )(
    _In_ HANDLE ParentId,
    _In_ HANDLE ProcessId,
    _In_ BOOLEAN Create
    );

struct image_info_t {
    union {
        ULONG m_properties;
        struct {
            ULONG m_image_addressing_mode : 8;
            ULONG m_system_mode_image : 1;
            ULONG m_image_mapped_to_all_pids : 1;
            ULONG m_extended_info_present : 1;
            ULONG m_machine_type_mismatch : 1;
            ULONG m_image_signature_level : 4;
            ULONG m_image_signature_type : 3;
            ULONG m_image_partial_map : 1;
            ULONG m_reserved : 12;
        };
    };
    PVOID  m_image_base;
    ULONG  m_image_selector;
    SIZE_T m_image_size;
    ULONG  m_image_section_number;
};

typedef
void
( *p_load_image_notify_routine )(
    _In_ unicode_string_t* ParentId,
    _In_ HANDLE ProcessId,
    _In_ image_info_t* ImageInfo
    );


typedef
void
( *p_create_thread_notify_routine )(
    _In_ HANDLE ProcessId,
    _In_ HANDLE ThreadId,
    _In_ BOOLEAN Create
    );

typedef
e_bound_callback_status
( *p_bound_callback )(
    void
    );

typedef
std::uint64_t
kipi_broadcast_worker(
    _In_ std::uint64_t argument
);

typedef struct _bus_handler bus_handler_t;
typedef struct _cm_resource_list cm_resource_list_t;
typedef struct _adapter_object adapter_object_t;
typedef struct _map_register_entry map_register_entry_t;
typedef struct _debug_device_descriptor debug_device_descriptor_t;
typedef struct _loader_parameter_block loader_parameter_block_t;
typedef struct _interrupt_remapping_info interrupt_remapping_info_t;
typedef struct _hal_dp_replace_parameters hal_dp_replace_parameters_t;
typedef struct _whea_error_record_section_descriptor whea_error_record_section_descriptor_t;
typedef struct _whea_processor_generic_error_section whea_processor_generic_error_section_t;
typedef struct _hal_intel_enlightenment_information hal_intel_enlightenment_information_t;
typedef struct _hal_log_register_context hal_log_register_context_t;
typedef struct _interrupt_vector_data interrupt_vector_data_t;
typedef struct _pci_busmaster_descriptor pci_busmaster_descriptor_t;
typedef struct _hal_pmc_counters hal_pmc_counters_t;
typedef struct _hal_unmasked_interrupt_information hal_unmasked_interrupt_information_t;
typedef struct _hal_clock_timer_configuration hal_clock_timer_configuration_t;
typedef struct _hal_iommu_dispatch hal_iommu_dispatch_t;
typedef struct _kinterrupt kinterrupt_t;
typedef struct _hal_lbr_entry hal_lbr_entry_t;
typedef struct _ext_iommu_device_id ext_iommu_device_id_t;
typedef struct _dma_iommu_interface dma_iommu_interface_t;
typedef struct _hidden_processor_power_interface hidden_processor_power_interface_t;
typedef struct _fault_information fault_information_t;

typedef enum _interface_type {
    m_interface_internal = 0,
    m_interface_isa,
    m_interface_eisa,
    m_interface_micro_channel,
    m_interface_turbo_channel,
    m_interface_pci_bus,
    m_interface_vme_bus,
    m_interface_nu_bus,
    m_interface_pcmcia_bus,
    m_interface_c_bus,
    m_interface_mpi_bus,
    m_interface_mpsa_bus,
    m_interface_processor_internal,
    m_interface_internal_power_bus,
    m_interface_pnp_isa_bus,
    m_interface_pnp_bus,
    m_interface_vmcs,
    m_interface_acpi_bus,
    m_interface_maximum_interface_type
} interface_type_t;

typedef enum _bus_data_type {
    m_config_space_undefined = -1,
    m_config_space_cmos,
    m_config_space_eisa_configuration,
    m_config_space_pos,
    m_config_space_cbios,
    m_config_space_pci_configuration,
    m_config_space_vme_configuration,
    m_config_space_nu_bus_configuration,
    m_config_space_pcmcia_configuration,
    m_config_space_mpi_configuration,
    m_config_space_mpsa_configuration,
    m_config_space_pnp_isa_configuration,
    m_config_space_sgi_internal_configuration,
    m_config_space_maximum_bus_data
} bus_data_type_t;

typedef enum _system_power_state {
    m_power_system_unspecified = 0,
    m_power_system_working,
    m_power_system_sleeping1,
    m_power_system_sleeping2,
    m_power_system_sleeping3,
    m_power_system_hibernate,
    m_power_system_shutdown,
    m_power_system_maximum
} system_power_state_t;

typedef enum _kinterrupt_mode {
    m_level_sensitive = 0,
    m_latched
} kinterrupt_mode_t;

typedef enum _kinterrupt_polarity {
    m_interrupt_polarity_unknown = 0,
    m_interrupt_active_high,
    m_interrupt_active_low,
    m_interrupt_active_both
} kinterrupt_polarity_t;

typedef enum _kprofile_source {
    m_profile_time = 0,
    m_profile_align_fixup,
    m_profile_total_issues,
    m_profile_pipeline_dry,
    m_profile_load_instructions,
    m_profile_pipeline_frozen,
    m_profile_branch_instructions,
    m_profile_total_nonissues,
    m_profile_dcache_misses,
    m_profile_icache_misses,
    m_profile_cache_misses,
    m_profile_branch_mispredictions,
    m_profile_store_instructions,
    m_profile_maximum
} kprofile_source_t;

typedef enum _hal_clock_timer_mode {
    m_clock_timer_inactive = 0,
    m_clock_timer_periodic,
    m_clock_timer_one_shot
} hal_clock_timer_mode_t;

typedef enum _kd_callback_action {
    m_kd_callback_continue = 0,
    m_kd_callback_stop
} kd_callback_action_t;

typedef enum _hal_processor_stat_type {
    m_hal_processor_stat_residency = 0,
    m_hal_processor_stat_count,
    m_hal_processor_stat_max
} hal_processor_stat_type_t;

struct hal_private_dispatch_t {
    uint32_t m_version;                                                                    // 0x0
    bus_handler_t* ( *m_hal_handler_for_bus )( interface_type_t arg1, uint32_t arg2 );        // 0x8
    bus_handler_t* ( *m_hal_handler_for_config_space )( bus_data_type_t arg1, uint32_t arg2 ); // 0x10
    void ( *m_hal_locate_hiber_ranges )( void* arg1 );                                         // 0x18
    int32_t( *m_hal_register_bus_handler )( interface_type_t arg1, bus_data_type_t arg2, uint32_t arg3, interface_type_t arg4, uint32_t arg5, uint32_t arg6, int32_t( *arg7 )( bus_handler_t* arg1 ), bus_handler_t** arg8 ); // 0x20
    void ( *m_hal_set_wake_enable )( uint8_t arg1 );                                           // 0x28
    int32_t( *m_hal_set_wake_alarm )( uint64_t arg1, uint64_t arg2 );                         // 0x30
    uint8_t( *m_hal_pci_translate_bus_address )( interface_type_t arg1, uint32_t arg2, ularge_integer_t arg3, uint32_t* arg4, ularge_integer_t* arg5 ); // 0x38
    int32_t( *m_hal_pci_assign_slot_resources )( unicode_string_t* arg1, unicode_string_t* arg2, driver_object_t* arg3, device_object_t* arg4, interface_type_t arg5, uint32_t arg6, uint32_t arg7, cm_resource_list_t** arg8 ); // 0x40
    void ( *m_hal_halt_system )( );                                                           // 0x48
    uint8_t( *m_hal_find_bus_address_translation )( ularge_integer_t arg1, uint32_t* arg2, ularge_integer_t* arg3, uint64_t* arg4, uint8_t arg5 ); // 0x50
    uint8_t( *m_hal_reset_display )( );                                                      // 0x58
    int32_t( *m_hal_allocate_map_registers )( adapter_object_t* arg1, uint32_t arg2, uint32_t arg3, map_register_entry_t* arg4 ); // 0x60
    int32_t( *m_kd_setup_pci_device_for_debugging )( void* arg1, debug_device_descriptor_t* arg2 ); // 0x68
    int32_t( *m_kd_release_pci_device_for_debugging )( debug_device_descriptor_t* arg1 );     // 0x70
    void* ( *m_kd_get_acpi_table_phase0 )( loader_parameter_block_t* arg1, uint32_t arg2 );    // 0x78
    void ( *m_kd_check_power_button )( );                                                     // 0x80
    uint8_t( *m_hal_vector_to_idt_entry )( uint32_t arg1 );                                   // 0x88
    void* ( *m_kd_map_physical_memory64 )( ularge_integer_t arg1, uint32_t arg2, uint8_t arg3 ); // 0x90
    void ( *m_kd_unmap_virtual_address )( void* arg1, uint32_t arg2, uint8_t arg3 );           // 0x98
    uint32_t( *m_kd_get_pci_data_by_offset )( uint32_t arg1, uint32_t arg2, void* arg3, uint32_t arg4, uint32_t arg5 ); // 0xa0
    uint32_t( *m_kd_set_pci_data_by_offset )( uint32_t arg1, uint32_t arg2, void* arg3, uint32_t arg4, uint32_t arg5 ); // 0xa8
    uint32_t( *m_hal_get_interrupt_vector_override )( interface_type_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint8_t* arg5, uint64_t* arg6 ); // 0xb0
    int32_t( *m_hal_get_vector_input_override )( uint32_t arg1, group_affinity_t* arg2, uint32_t* arg3, kinterrupt_polarity_t* arg4, interrupt_remapping_info_t* arg5 ); // 0xb8
    int32_t( *m_hal_load_microcode )( void* arg1 );                                           // 0xc0
    int32_t( *m_hal_unload_microcode )( );                                                   // 0xc8
    int32_t( *m_hal_post_microcode_update )( );                                              // 0xd0
    int32_t( *m_hal_allocate_message_target_override )( device_object_t* arg1, group_affinity_t* arg2, uint32_t arg3, kinterrupt_mode_t arg4, uint8_t arg5, uint32_t* arg6, uint8_t* arg7, uint32_t* arg8 ); // 0xd8
    void ( *m_hal_free_message_target_override )( device_object_t* arg1, uint32_t arg2, group_affinity_t* arg3 ); // 0xe0
    int32_t( *m_hal_dp_replace_begin )( hal_dp_replace_parameters_t* arg1, void** arg2 );     // 0xe8
    void ( *m_hal_dp_replace_target )( void* arg1 );                                           // 0xf0
    int32_t( *m_hal_dp_replace_control )( uint32_t arg1, void* arg2 );                        // 0xf8
    void ( *m_hal_dp_replace_end )( void* arg1 );                                              // 0x100
    void ( *m_hal_prepare_for_bugcheck )( uint32_t arg1 );                                     // 0x108
    uint8_t( *m_hal_query_wake_time )( uint64_t* arg1, uint64_t* arg2 );                      // 0x110
    void ( *m_hal_report_idle_state_usage )( uint8_t arg1, kaffinity_ex_t* arg2 );             // 0x118
    void ( *m_hal_tsc_synchronization )( uint8_t arg1, uint32_t* arg2 );                       // 0x120
    int32_t( *m_hal_whea_init_processor_generic_section )( whea_error_record_section_descriptor_t* arg1, whea_processor_generic_error_section_t* arg2 ); // 0x128
    void ( *m_hal_stop_legacy_usb_interrupts )( system_power_state_t arg1 );                   // 0x130
    int32_t( *m_hal_read_whea_physical_memory )( ularge_integer_t arg1, uint32_t arg2, void* arg3 ); // 0x138
    int32_t( *m_hal_write_whea_physical_memory )( ularge_integer_t arg1, uint32_t arg2, void* arg3 ); // 0x140
    int32_t( *m_hal_dp_mask_level_triggered_interrupts )( );                                 // 0x148
    int32_t( *m_hal_dp_unmask_level_triggered_interrupts )( );                               // 0x150
    int32_t( *m_hal_dp_get_interrupt_replay_state )( void* arg1, void** arg2 );               // 0x158
    int32_t( *m_hal_dp_replay_interrupts )( void* arg1 );                                     // 0x160
    uint8_t( *m_hal_query_io_port_access_supported )( );                                     // 0x168
    int32_t( *m_kd_setup_integrated_device_for_debugging )( void* arg1, debug_device_descriptor_t* arg2 ); // 0x170
    int32_t( *m_kd_release_integrated_device_for_debugging )( debug_device_descriptor_t* arg1 ); // 0x178
    void ( *m_hal_get_enlightenment_information )( hal_intel_enlightenment_information_t* arg1 ); // 0x180
    void* ( *m_hal_allocate_early_pages )( loader_parameter_block_t* arg1, uint32_t arg2, uint64_t* arg3, uint32_t arg4 ); // 0x188
    void* ( *m_hal_map_early_pages )( uint64_t arg1, uint32_t arg2, uint32_t arg3 );           // 0x190
    void* m_dummy1;                                                                        // 0x198
    void* m_dummy2;                                                                        // 0x1a0
    void ( *m_hal_notify_processor_freeze )( uint8_t arg1, uint8_t arg2 );                     // 0x1a8
    int32_t( *m_hal_prepare_processor_for_idle )( uint32_t arg1 );                            // 0x1b0
    void ( *m_hal_register_log_routine )( hal_log_register_context_t* arg1 );                  // 0x1b8
    void ( *m_hal_resume_processor_from_idle )( );                                            // 0x1c0
    void* m_dummy;                                                                         // 0x1c8
    uint32_t( *m_hal_vector_to_idt_entry_ex )( uint32_t arg1 );                               // 0x1d0
    int32_t( *m_hal_secondary_interrupt_query_primary_information )( interrupt_vector_data_t* arg1, uint32_t* arg2 ); // 0x1d8
    int32_t( *m_hal_mask_interrupt )( uint32_t arg1, uint32_t arg2 );                         // 0x1e0
    int32_t( *m_hal_unmask_interrupt )( uint32_t arg1, uint32_t arg2 );                       // 0x1e8
    uint8_t( *m_hal_is_interrupt_type_secondary )( uint32_t arg1, uint32_t arg2 );            // 0x1f0
    int32_t( *m_hal_allocate_gsiv_for_secondary_interrupt )( char* arg1, uint16_t arg2, uint32_t* arg3 ); // 0x1f8
    int32_t( *m_hal_add_interrupt_remapping )( uint32_t arg1, uint32_t arg2, pci_busmaster_descriptor_t* arg3, uint8_t arg4, interrupt_vector_data_t* arg5, uint32_t arg6 ); // 0x200
    void ( *m_hal_remove_interrupt_remapping )( uint32_t arg1, uint32_t arg2, pci_busmaster_descriptor_t* arg3, uint8_t arg4, interrupt_vector_data_t* arg5, uint32_t arg6 ); // 0x208
    void ( *m_hal_save_and_disable_hv_enlightenment )( );                                     // 0x210
    void ( *m_hal_restore_hv_enlightenment )( );                                              // 0x218
    void ( *m_hal_flush_io_buffers_external_cache )( mdl_t* arg1, uint8_t arg2 );              // 0x220
    void ( *m_hal_flush_external_cache )( uint8_t arg1 );                                      // 0x228
    int32_t( *m_hal_pci_early_restore )( system_power_state_t arg1 );                         // 0x230
    int32_t( *m_hal_get_processor_id )( uint32_t arg1, uint32_t* arg2, uint32_t* arg3 );      // 0x238
    int32_t( *m_hal_allocate_pmc_counter_set )( uint32_t arg1, kprofile_source_t* arg2, uint32_t arg3, hal_pmc_counters_t** arg4 ); // 0x240
    void ( *m_hal_collect_pmc_counters )( hal_pmc_counters_t* arg1, uint64_t* arg2 );          // 0x248
    void ( *m_hal_free_pmc_counter_set )( hal_pmc_counters_t* arg1 );                          // 0x250
    int32_t( *m_hal_processor_halt )( uint32_t arg1, void* arg2, int32_t( *arg3 )( void* arg1 ) ); // 0x258
    uint64_t( *m_hal_timer_query_cycle_counter )( uint64_t* arg1 );                           // 0x260
    void* m_dummy3;                                                                        // 0x268
    void ( *m_hal_pci_mark_hiber_phase )( );                                                  // 0x270
    int32_t( *m_hal_query_processor_restart_entry_point )( ularge_integer_t* arg1 );           // 0x278
    int32_t( *m_hal_request_interrupt )( uint32_t arg1 );                                     // 0x280
    int32_t( *m_hal_enumerate_unmasked_interrupts )( uint8_t( *arg1 )( void* arg1, hal_unmasked_interrupt_information_t* arg2 ), void* arg2, hal_unmasked_interrupt_information_t* arg3 ); // 0x288
    void ( *m_hal_flush_and_invalidate_page_external_cache )( ularge_integer_t arg1 );          // 0x290
    int32_t( *m_kd_enumerate_debugging_devices )( void* arg1, debug_device_descriptor_t* arg2, kd_callback_action_t( *arg3 )( debug_device_descriptor_t* arg1 ) ); // 0x298
    void ( *m_hal_flush_io_rectangle_external_cache )( mdl_t* arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5, uint8_t arg6 ); // 0x2a0
    void ( *m_hal_power_early_restore )( uint32_t arg1 );                                      // 0x2a8
    int32_t( *m_hal_query_capsule_capabilities )( void* arg1, uint32_t arg2, uint64_t* arg3, uint32_t* arg4 ); // 0x2b0
    int32_t( *m_hal_update_capsule )( void* arg1, uint32_t arg2, ularge_integer_t arg3 );      // 0x2b8
    uint8_t( *m_hal_pci_multi_stage_resume_capable )( );                                     // 0x2c0
    void ( *m_hal_dma_free_crash_dump_registers )( uint32_t arg1 );                            // 0x2c8
    uint8_t( *m_hal_acpi_aoac_capable )( );                                                  // 0x2d0
    int32_t( *m_hal_interrupt_set_destination )( interrupt_vector_data_t* arg1, group_affinity_t* arg2, uint32_t* arg3 ); // 0x2d8
    void ( *m_hal_get_clock_configuration )( hal_clock_timer_configuration_t* arg1 );          // 0x2e0
    void ( *m_hal_clock_timer_activate )( uint8_t arg1 );                                      // 0x2e8
    void ( *m_hal_clock_timer_initialize )( );                                                // 0x2f0
    void ( *m_hal_clock_timer_stop )( );                                                      // 0x2f8
    int32_t( *m_hal_clock_timer_arm )( hal_clock_timer_mode_t arg1, uint64_t arg2, uint64_t* arg3 ); // 0x300
    uint8_t( *m_hal_timer_only_clock_interrupt_pending )( );                                 // 0x308
    void* ( *m_hal_acpi_get_multi_node )( );                                                  // 0x310
    void ( *( *m_hal_power_set_reboot_handler )( void ( *arg1 )( uint32_t arg1, volatile int32_t* arg2 ) ) )( uint32_t arg1, volatile int32_t* arg2 ); // 0x318
    void ( *m_hal_iommu_register_dispatch_table )( hal_iommu_dispatch_t* arg1 );               // 0x320
    void ( *m_hal_timer_watchdog_start )( );                                                  // 0x328
    void ( *m_hal_timer_watchdog_reset_countdown )( );                                        // 0x330
    void ( *m_hal_timer_watchdog_stop )( );                                                   // 0x338
    uint8_t( *m_hal_timer_watchdog_generated_last_reset )( );                                // 0x340
    int32_t( *m_hal_timer_watchdog_trigger_system_reset )( uint8_t arg1 );                    // 0x348
    int32_t( *m_hal_interrupt_vector_data_to_gsiv )( interrupt_vector_data_t* arg1, uint32_t* arg2 ); // 0x350
    int32_t( *m_hal_interrupt_get_highest_priority_interrupt )( uint32_t* arg1, uint8_t* arg2 ); // 0x358
    int32_t( *m_hal_processor_on )( uint32_t arg1 );                                          // 0x360
    int32_t( *m_hal_processor_off )( );                                                      // 0x368
    int32_t( *m_hal_processor_freeze )( );                                                   // 0x370
    int32_t( *m_hal_dma_link_device_object_by_token )( uint64_t arg1, device_object_t* arg2 ); // 0x378
    int32_t( *m_hal_dma_check_adapter_token )( uint64_t arg1 );                               // 0x380
    void* m_dummy4;                                                                        // 0x388
    int32_t( *m_hal_timer_convert_performance_counter_to_auxiliary_counter )( uint64_t arg1, uint64_t* arg2, uint64_t* arg3 ); // 0x390
    int32_t( *m_hal_timer_convert_auxiliary_counter_to_performance_counter )( uint64_t arg1, uint64_t* arg2, uint64_t* arg3 ); // 0x398
    int32_t( *m_hal_timer_query_auxiliary_counter_frequency )( uint64_t* arg1 );              // 0x3a0
    int32_t( *m_hal_connect_thermal_interrupt )( uint8_t( *arg1 )( kinterrupt_t* arg1, void* arg2 ) ); // 0x3a8
    uint8_t( *m_hal_is_efi_runtime_active )( );                                              // 0x3b0
    uint8_t( *m_hal_timer_query_and_reset_rtc_errors )( uint8_t arg1 );                       // 0x3b8
    void ( *m_hal_acpi_late_restore )( );                                                     // 0x3c0
    int32_t( *m_kd_watchdog_delay_expiration )( uint64_t* arg1 );                             // 0x3c8
    int32_t( *m_hal_get_processor_stats )( hal_processor_stat_type_t arg1, uint32_t arg2, uint32_t arg3, uint64_t* arg4 ); // 0x3d0
    uint64_t( *m_hal_timer_watchdog_query_due_time )( uint8_t arg1 );                         // 0x3d8
    int32_t( *m_hal_connect_synthetic_interrupt )( uint8_t( *arg1 )( kinterrupt_t* arg1, void* arg2 ) ); // 0x3e0
    void ( *m_hal_preprocess_nmi )( uint32_t arg1 );                                           // 0x3e8
    int32_t( *m_hal_enumerate_environment_variables_with_filter )( uint32_t arg1, uint8_t( *arg2 )( guid_t* arg1, wchar_t* arg2 ), void* arg3, uint32_t* arg4 ); // 0x3f0
    int32_t( *m_hal_capture_last_branch_record_stack )( uint32_t arg1, hal_lbr_entry_t* arg2, uint32_t* arg3 ); // 0x3f8
    uint8_t( *m_hal_clear_last_branch_record_stack )( );                                     // 0x400
    int32_t( *m_hal_configure_last_branch_record )( uint32_t arg1, uint32_t arg2 );           // 0x408
    uint8_t( *m_hal_get_last_branch_information )( uint32_t* arg1, uint32_t* arg2 );          // 0x410
    void ( *m_hal_resume_last_branch_record )( uint8_t arg1 );                                 // 0x418
    int32_t( *m_hal_start_last_branch_record )( uint32_t arg1, uint32_t* arg2 );              // 0x420
    int32_t( *m_hal_stop_last_branch_record )( uint32_t arg1 );                               // 0x428
    int32_t( *m_hal_iommu_block_device )( void* arg1 );                                       // 0x430
    int32_t( *m_hal_iommu_unblock_device )( ext_iommu_device_id_t* arg1, void** arg2 );       // 0x438
    int32_t( *m_hal_get_iommu_interface )( uint32_t arg1, dma_iommu_interface_t* arg2 );      // 0x440
    int32_t( *m_hal_request_generic_error_recovery )( void* arg1, uint32_t* arg2 );           // 0x448
    int32_t( *m_hal_timer_query_host_performance_counter )( uint64_t* arg1 );                 // 0x450
    int32_t( *m_hal_topology_query_processor_relationships )( uint32_t arg1, uint32_t arg2, uint8_t* arg3, uint8_t* arg4, uint8_t* arg5, uint32_t* arg6, uint32_t* arg7 ); // 0x458
    void ( *m_hal_init_platform_debug_triggers )( );                                          // 0x460
    void ( *m_hal_run_platform_debug_triggers )( uint8_t arg1 );                               // 0x468
    void* ( *m_hal_timer_get_reference_page )( );                                             // 0x470
    int32_t( *m_hal_get_hidden_processor_power_interface )( hidden_processor_power_interface_t* arg1 ); // 0x478
    uint32_t( *m_hal_get_hidden_processor_package_id )( uint32_t arg1 );                      // 0x480
    uint32_t( *m_hal_get_hidden_package_processor_count )( uint32_t arg1 );                   // 0x488
    int32_t( *m_hal_get_hidden_processor_apic_id_by_index )( uint32_t arg1, uint32_t* arg2 ); // 0x490
    int32_t( *m_hal_register_hidden_processor_idle_state )( uint32_t arg1, uint64_t arg2 );   // 0x498
    void ( *m_hal_iommu_report_iommu_fault )( uint64_t arg1, fault_information_t* arg2 );      // 0x4a0
    uint8_t( *m_hal_iommu_dma_remapping_capable )( ext_iommu_device_id_t* arg1, uint32_t* arg2 ); // 0x4a8
};

enum system_information_class_t : std::uint32_t {
    system_basic_information = 0,
    system_processor_information = 1,
    system_performance_information = 2,
    system_time_of_day_information = 3,
    system_path_information = 4,
    system_process_information = 5,
    system_call_count_information = 6,
    system_device_information = 7,
    system_processor_performance_information = 8,
    system_flags_information = 9,
    system_call_time_information = 10,
    system_module_information = 11,
    system_locks_information = 12,
    system_stack_trace_information = 13,
    system_paged_pool_information = 14,
    system_non_paged_pool_information = 15,
    system_handle_information = 16,
    system_object_information = 17,
    system_page_file_information = 18,
    system_vdm_instemul_information = 19,
    system_vdm_bop_information = 20,
    system_file_cache_information = 21,
    system_pool_tag_information = 22,
    system_interrupt_information = 23,
    system_dpc_behavior_information = 24,
    system_full_memory_information = 25,
    system_load_gdi_driver_information = 26,
    system_unload_gdi_driver_information = 27,
    system_time_adjustment_information = 28,
    system_summary_memory_information = 29,
    system_mirror_memory_information = 30,
    system_performance_trace_information = 31,
    system_crash_dump_information = 32,
    system_exception_information = 33,
    system_crash_dump_state_information = 34,
    system_kernel_debugger_information = 35,
    system_context_switch_information = 36,
    system_registry_quota_information = 37,
    system_extend_service_table_information = 38,
    system_priority_separation = 39,
    system_verifier_add_driver_information = 40,
    system_verifier_remove_driver_information = 41,
    system_processor_idle_information = 42,
    system_legacy_driver_information = 43,
    system_current_time_zone_information = 44,
    system_lookaside_information = 45,
    system_time_slip_notification = 46,
    system_session_create = 47,
    system_session_detach = 48,
    system_session_information = 49,
    system_range_start_information = 50,
    system_verifier_information = 51,
    system_verifier_thunk_extend = 52,
    system_session_process_information = 53,
    system_load_gdi_driver_in_system_space = 54,
    system_numa_processor_map = 55,
    system_prefetch_information = 56,
    system_extended_process_information = 57,
    system_recommended_shared_data_alignment = 58,
    system_com_plus_package = 59,
    system_numa_available_memory = 60,
    system_processor_power_information = 61,
    system_emulation_basic_information = 62,
    system_emulation_processor_information = 63,
    system_extended_handle_information = 64,
    system_lost_delayed_write_information = 65,
    system_big_pool_information = 66,
    system_session_pool_tag_information = 67,
    system_session_mapped_view_information = 68,
    system_hot_patch_information = 69,
    system_object_security_mode = 70,
    system_watchdog_timer_handler_information = 71,
    system_watchdog_timer_information = 72,
    system_logical_processor_information = 73,
    system_wow64_shared_information_obsolete = 74,
    system_register_firmware_table_information_handler = 75,
    system_firmware_table_information = 76,
    system_module_information_ex = 77,
    system_verifier_triage_information = 78,
    system_superfetch_information = 79,
    system_memory_list_information = 80,
    system_filecache_information_ex = 81,
    system_thread_priority_client_id_information = 82,
    system_processor_idle_cycle_time_information = 83,
    system_verifier_cancellation_information = 84,
    system_processor_power_information_ex = 85,
    system_ref_trace_information = 86,
    system_special_pool_information = 87,
    system_process_id_information = 88,
    system_error_port_information = 89,
    system_boot_environment_information = 90,
    system_hypervisor_information = 91,
    system_verifier_information_ex = 92,
    system_time_zone_information = 93,
    system_image_file_execution_options_information = 94,
    system_coverage_information = 95,
    system_prefetch_patch_information = 96,
    system_verifier_faults_information = 97,
    system_system_partition_information = 98,
    system_system_disk_information = 99,
    system_processor_performance_distribution = 100,
    system_numa_proximity_node_information = 101,
    system_dynamic_time_zone_information = 102,
    system_code_integrity_information = 103,
    system_processor_microcode_update_information = 104,
    system_processor_brand_string = 105,
    system_virtual_address_information = 106,
    system_logical_processor_and_group_information = 107,
    system_processor_cycle_time_information = 108,
    system_store_information = 109,
    system_registry_append_string = 110,
    system_air_sop_information = 111,
    system_dpc_watchdog_information = 112,
    system_product_type = 113,
    system_actual_product_type = 114,
    system_memory_topology_information = 115,
    system_memory_channel_information = 116,
    system_boot_logo_information = 117,
    system_processor_performance_information_ex = 118,
    system_critical_process_error_log_information = 119,
    system_secure_boot_policy_information = 120,
    system_page_file_information_ex = 121,
    system_secure_boot_information = 122,
    system_entropy_interrupt_timing_callback = 123,
    system_console_information = 124,
    system_platform_binary_information = 125,
    system_policy_information = 126,
    system_hypervisor_processor_count_information = 127,
    system_device_data_information = 128,
    system_device_data_enumeration_information = 129,
    system_memory_topology_information_ex = 130,
    system_memory_channel_information_ex = 131,
    system_boot_statistics_information = 132,
    system_codeintegrity_unlock_information = 133,
    system_flush_information = 134,
    system_processor_idle_state_information = 135,
    system_secure_dump_encryption_information = 136,
    system_write_constraint_information = 137,
    system_kernel_va_shadow_information = 138,
    system_hypervisor_shared_page_information = 139,
    system_firmware_ramdisk_information = 140,
    system_secure_speculation_control_information = 141,
    system_speculation_control_information = 142,
    system_dma_guard_policy_information = 143,
    system_enclave_launch_control_information = 144,
    system_workload_allowed_information = 145,
    system_codeintegrity_policy_information = 146,
    system_codeintegrity_unlock_mode_information = 147,
    system_leap_second_information = 148,
    system_flags2_information = 149,
    system_security_model_information = 150,
    system_codeintegrity_platform_manifest_information = 151,
    system_interrupt_cpu_sets_information = 152,
    system_secure_boot_policy_full_information = 153,
    system_codeintegrity_policy_full_information = 154,
    system_affinity_update_mode_information = 155,
    system_coverage_ex_information = 156,
    system_prefetch_fault_information = 157,
    system_verifier_flags_information = 158,
    system_system_partition_information_ex = 159,
    system_kernel_debugger_information_ex = 160,
    system_codeintegrity_certificate_information = 161,
    system_physical_memory_information = 162,
    system_control_flow_transition = 163,
    system_kernel_deferred_error_information = 164,
    system_max_info_class = 165
};

typedef struct _ob_operation_registration_t {
    void** ObjectType;              // Pointer to PsProcessType or PsThreadType
    ULONG Operations;                       // OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE
    POB_PRE_OPERATION_CALLBACK PreOperation;
    POB_POST_OPERATION_CALLBACK PostOperation;
} ob_operation_registration_t, * pob_operation_registration_t;

// Callback registration structure (what you pass to ObRegisterCallbacks)
typedef struct _ob_callback_registration_t {
    USHORT m_version;                       // OB_FLT_REGISTRATION_VERSION (0x0100)
    USHORT m_operation_registration_count;  // Number of operations
    unicode_string_t m_altitude;            // Altitude string
    PVOID m_registration_context;           // Optional context
    ob_operation_registration_t* m_operation_registration;  // Array of operations (NOT void*)
} ob_callback_registration_t, * pob_callback_registration_t;

typedef struct callback_entry_t {
    unsigned short m_version;
    unsigned short m_operation_registration_count;
    unsigned long m_unk1;
    void* m_registration_context;
    unicode_string_t m_altitude;
};

struct callback_entry_item_t {
    list_entry_t m_callback_list;
    unsigned long m_operations;
    unsigned long m_active;
    callback_entry_t* m_callback_entry;
    POBJECT_TYPE* m_object_type;
    POB_PRE_OPERATION_CALLBACK m_pre_operation;
    POB_POST_OPERATION_CALLBACK m_post_operation;
    std::uint64_t m_unk1;
};

typedef struct _runtime_function_t {
    unsigned int m_begin_address;
    unsigned int m_end_address;
    union {
        unsigned int m_unwind_info_address;
        unsigned int m_unwind_data;
    };
} runtime_function_t, * pruntime_function_t;

typedef struct _unwind_history_table_entry_t {
    DWORD64 m_image_base;
    runtime_function_t* m_function_entry;
} unwind_history_table_entry_t, * punwind_history_table_entry_t;

typedef struct _unwind_history_table_t {
    unsigned int m_count;
    std::uint8_t  m_local_hint;
    std::uint8_t  m_global_hint;
    std::uint8_t  m_search;
    std::uint8_t  m_once;
    std::uint64_t m_low_address;
    std::uint64_t m_high_address;
    unwind_history_table_entry_t m_entry[ 12 ];
} unwind_history_table_t, * punwind_history_table_t;

struct file_network_open_information_t {
    ularge_integer_t creation_time;
    ularge_integer_t last_access_time;
    ularge_integer_t last_write_time;
    ularge_integer_t change_time;
    ularge_integer_t allocation_size;
    ularge_integer_t end_of_file;
    std::uint32_t file_attributes;
    std::uint32_t reserved;
};

//0x78 bytes (sizeof)
struct _OBJECT_TYPE_INITIALIZER {
    USHORT length;                                                          //0x0
    union {
        USHORT object_type_flags;                                           //0x2
        struct {
            UCHAR case_insensitive : 1;                                       //0x2
            UCHAR unnamed_objects_only : 1;                                   //0x2
            UCHAR use_default_object : 1;                                     //0x2
            UCHAR security_required : 1;                                      //0x2
            UCHAR maintain_handle_count : 1;                                  //0x2
            UCHAR maintain_type_list : 1;                                     //0x2
            UCHAR supports_object_callbacks : 1;                              //0x2
            UCHAR cache_aligned : 1;                                          //0x2
            UCHAR use_extended_parameters : 1;                                //0x3
            UCHAR reserved : 7;                                               //0x3
        };
    };
    ULONG object_type_code;                                                 //0x4
    ULONG invalid_attributes;                                               //0x8
    struct _GENERIC_MAPPING generic_mapping;                                //0xc
    ULONG valid_access_mask;                                                //0x1c
    ULONG retain_access;                                                    //0x20
    enum _POOL_TYPE pool_type;                                              //0x24
    ULONG default_paged_pool_charge;                                        //0x28
    ULONG default_non_paged_pool_charge;                                    //0x2c
    VOID( *dump_procedure )( VOID* arg1, struct _OBJECT_DUMP_CONTROL* arg2 );  //0x30
    LONG( *open_procedure )( enum _OB_OPEN_REASON arg1, CHAR arg2, struct _EPROCESS* arg3, VOID* arg4, ULONG* arg5, ULONG arg6 ); //0x38
    VOID( *close_procedure )( struct _EPROCESS* arg1, VOID* arg2, ULONGLONG arg3, ULONGLONG arg4 ); //0x40
    VOID( *delete_procedure )( VOID* arg1 );                                   //0x48
    union {
        LONG( *parse_procedure )( VOID* arg1, VOID* arg2, struct _ACCESS_STATE* arg3, CHAR arg4, ULONG arg5, struct _UNICODE_STRING* arg6, struct _UNICODE_STRING* arg7, VOID* arg8, struct _SECURITY_QUALITY_OF_SERVICE* arg9, VOID** arg10 ); //0x50
        LONG( *parse_procedure_ex )( VOID* arg1, VOID* arg2, struct _ACCESS_STATE* arg3, CHAR arg4, ULONG arg5, struct _UNICODE_STRING* arg6, struct _UNICODE_STRING* arg7, VOID* arg8, struct _SECURITY_QUALITY_OF_SERVICE* arg9, struct _OB_EXTENDED_PARSE_PARAMETERS* arg10, VOID** arg11 ); //0x50
    };
    LONG( *security_procedure )( VOID* arg1, enum _SECURITY_OPERATION_CODE arg2, ULONG* arg3, VOID* arg4, ULONG* arg5, VOID** arg6, enum _POOL_TYPE arg7, struct _GENERIC_MAPPING* arg8, CHAR arg9 ); //0x58
    LONG( *query_name_procedure )( VOID* arg1, UCHAR arg2, struct _OBJECT_NAME_INFORMATION* arg3, ULONG arg4, ULONG* arg5, CHAR arg6 ); //0x60
    UCHAR( *okay_to_close_procedure )( struct _EPROCESS* arg1, VOID* arg2, VOID* arg3, CHAR arg4 ); //0x68
    ULONG wait_object_flag_mask;                                            //0x70
    USHORT wait_object_flag_offset;                                         //0x74
    USHORT wait_object_pointer_offset;                                      //0x76
};

struct object_type_t {
    list_entry_t type_list;                                            //0x0
    unicode_string_t name;                                            //0x10
    void* default_object;                                                    //0x20
    unsigned char index;                                                            //0x28
    unsigned long total_number_of_objects;                                             //0x2c
    unsigned long total_number_of_handles;                                             //0x30
    unsigned long high_water_number_of_objects;                                         //0x34
    unsigned long high_water_number_of_handles;                                         //0x38
    struct _OBJECT_TYPE_INITIALIZER type_info;                               //0x40
    ex_push_lock_t type_lock;                                          //0xb8
    unsigned long key;                                                              //0xc0
    list_entry_t callback_list;                                        //0xc8
};

struct object_name_information_t {
    unicode_string_t name;
};

typedef struct io_status_block_t {
    union {
        nt_status_t m_status;
        void* m_pointer;
    };

    std::uint64_t m_information;
};

struct file_object_t {
    std::int16_t m_type;
    std::int16_t m_size;
    int pad_0;
    void* m_device_object;
    void* m_vpb;
    void* m_fs_context;
    void* m_fs_context2;
    void* m_section_object_pointer;
    void* m_private_cache_map;
    nt_status_t m_final_status;
    int pad_1;
    file_object_t* m_related_file_object;
    bool m_lock_operation;
    bool m_delete_pending;
    bool m_read_access;
    bool m_write_access;
    bool m_delete_access;
    bool m_shared_read;
    bool m_shared_write;
    bool m_shared_delete;
    std::uint32_t m_flags;
    int pad_2;
    unicode_string_t m_file_name;
    ularge_integer_t m_current_byte_offset;
    volatile std::uint32_t m_waiters;
    volatile std::uint32_t m_busy;
    void* m_last_lock;
    kevent_t m_lock;
    kevent_t m_event;
    volatile void* m_completion_context;
    kspin_lock_queue_t m_irp_list_lock;
    list_entry_t m_irp_list;
    volatile void* m_file_object_extension; 
};

struct kidtentry64_t {
    std::uint16_t m_offset_low;          // 0x0 - Bits 0-15 of ISR address
    std::uint16_t m_selector;            // 0x2 - Code segment selector

    union {
        struct {
            std::uint16_t m_ist_index : 3;      // 0x4 - Interrupt Stack Table index
            std::uint16_t m_reserved0 : 5;      // 0x4
            std::uint16_t m_type : 5;           // 0x4 - Gate type
            std::uint16_t m_dpl : 2;            // 0x4 - Descriptor Privilege Level
            std::uint16_t m_present : 1;        // 0x4 - Present flag
        };
        std::uint16_t m_access;                 // 0x4
    };

    std::uint16_t m_offset_middle;       // 0x6 - Bits 16-31 of ISR address
    std::uint32_t m_offset_high;         // 0x8 - Bits 32-63 of ISR address
    std::uint32_t m_reserved1;           // 0xC - Reserved
};

#pragma pack(push, 1)  // CRITICAL: No padding/alignment!
struct ktss64_t {
    std::uint32_t Reserved0;        // 0x0
    std::uint64_t Rsp0;             // 0x4 (unaligned!)
    std::uint64_t Rsp1;             // 0xc (unaligned!)
    std::uint64_t Rsp2;             // 0x14 (unaligned!)
    std::uint64_t m_ist[ 8 ];         // 0x1c - IST[3] is NMI stack
    std::uint64_t Reserved1;        // 0x5c
    std::uint16_t Reserved2;        // 0x64
    std::uint16_t IoMapBase;        // 0x66
};
#pragma pack(pop)

using io_apc_routine_t = void ( * )(
    void* m_apc_context,
    io_status_block_t* m_io_status_block,
    std::uint32_t m_reserved
    );

struct kgdtentry64_t {
    std::uint16_t m_limit_low;           // 0x0 - Segment limit bits 0-15
    std::uint16_t m_base_low;            // 0x2 - Base address bits 0-15

    union {
        struct {
            std::uint8_t m_base_middle;      // 0x4 - Base address bits 16-23
            std::uint8_t m_flags1;           // 0x5 - Type, S, DPL, P
            std::uint8_t m_flags2;           // 0x6 - Limit 16-19, AVL, L, D/B, G
            std::uint8_t m_base_high;        // 0x7 - Base address bits 24-31
        } m_bytes;

        struct {
            std::uint32_t m_base_middle : 8;     // 0x4
            std::uint32_t m_type : 5;            // 0x5
            std::uint32_t m_dpl : 2;             // 0x5 - Descriptor Privilege Level
            std::uint32_t m_present : 1;         // 0x5
            std::uint32_t m_limit_high : 4;      // 0x6
            std::uint32_t m_system : 1;          // 0x6
            std::uint32_t m_long_mode : 1;       // 0x6 - L bit (64-bit code segment)
            std::uint32_t m_default_big : 1;     // 0x6 - D/B bit
            std::uint32_t m_granularity : 1;     // 0x6 - G bit
            std::uint32_t m_base_high : 8;       // 0x7
        } m_bits;
    };

    std::uint32_t m_base_upper;          // 0x8 - Base address bits 32-63 (for system segments)
    std::uint32_t m_must_be_zero;        // 0xC - Reserved, must be zero
};

// 0x178 bytes (sizeof)
struct kpcr_t {
    union {
        nt_tib_t m_nt_tib;                                                  // 0x0
        struct {
            kgdtentry64_t* m_gdt_base;                                      // 0x0
            ktss64_t* m_tss_base;                                           // 0x8
            std::uint64_t m_user_rsp;                                       // 0x10
            kpcr_t* m_self;                                                 // 0x18
            kprcb_t* m_current_prcb;                                        // 0x20
            kspin_lock_queue_t* m_lock_array;                               // 0x28
            void* m_used_self;                                              // 0x30
        };
    };
    kidtentry64_t* m_idt_base;                                              // 0x38
    std::uint64_t m_unused[ 2 ];                                              // 0x40
    std::uint8_t m_irql;                                                    // 0x50
    std::uint8_t m_second_level_cache_associativity;                        // 0x51
    std::uint8_t m_obsolete_number;                                         // 0x52
    std::uint8_t m_fill0;                                                   // 0x53
    std::uint32_t m_unused0[ 3 ];                                             // 0x54
    std::uint16_t m_major_version;                                          // 0x60
    std::uint16_t m_minor_version;                                          // 0x62
    std::uint32_t m_stall_scale_factor;                                     // 0x64
    void* m_unused1[ 3 ];                                                     // 0x68
    std::uint32_t m_kernel_reserved[ 15 ];                                    // 0x80
    std::uint32_t m_second_level_cache_size;                                // 0xBC
    std::uint32_t m_hal_reserved[ 16 ];                                       // 0xC0
    std::uint32_t m_unused2;                                                // 0x100
    void* m_kd_version_block;                                               // 0x108
    void* m_unused3;                                                        // 0x110
    std::uint32_t m_pcr_align1[ 24 ];                                         // 0x118
};

// Special registers saved during context switch
struct kspecial_registers_t {
    std::uint64_t m_cr0;                 // 0x00 - Control Register 0
    std::uint64_t m_cr2;                 // 0x08 - Control Register 2 (Page Fault Linear Address)
    std::uint64_t m_cr3;                 // 0x10 - Control Register 3 (Page Directory Base)
    std::uint64_t m_cr4;                 // 0x18 - Control Register 4
    std::uint64_t m_kd_base_address;     // 0x20 - Kernel Debugger Base Address
    std::uint64_t m_cr8;                 // 0x28 - Control Register 8 (Task Priority Register / IRQL)
    std::uint64_t m_gdtr_base;           // 0x30 - GDT Base Address
    std::uint16_t m_gdtr_limit;          // 0x38 - GDT Limit
    std::uint16_t m_idtr_limit;          // 0x3A - IDT Limit
    std::uint32_t m_reserved0;           // 0x3C - Reserved/Padding
    std::uint64_t m_idtr_base;           // 0x40 - IDT Base Address
    std::uint16_t m_tr;                  // 0x48 - Task Register
    std::uint16_t m_ldtr;                // 0x4A - Local Descriptor Table Register
    std::uint32_t m_reserved1;           // 0x4C - Reserved/Padding
    std::uint64_t m_xcr0;                // 0x50 - Extended Control Register 0 (XFEATURE_ENABLED_MASK)
    std::uint64_t m_reserved2[ 2 ];        // 0x58 - Reserved
    std::uint64_t m_mxcsr;               // 0x68 - SSE Control and Status Register
    std::uint64_t m_mxcsr_mask;          // 0x70 - MXCSR Mask
    std::uint64_t m_debug_control;       // 0x78 - Debug Control MSR
    std::uint64_t m_last_branch_to_rip;  // 0x80 - Last Branch To RIP
    std::uint64_t m_last_branch_from_rip;// 0x88 - Last Branch From RIP
    std::uint64_t m_last_exception_to_rip;   // 0x90 - Last Exception To RIP
    std::uint64_t m_last_exception_from_rip; // 0x98 - Last Exception From RIP
};

struct os_version_info_t {
    std::uint32_t m_size;
    std::uint32_t m_major_version;
    std::uint32_t m_minor_version;
    std::uint32_t m_build_number;
    std::uint32_t m_platform_id;
    wchar_t m_csd_version[ 128 ];
};

struct machine_frame_t {
    ULONGLONG Rip;                                                          //0x0
    USHORT SegCs;                                                           //0x8
    USHORT Fill1[ 3 ];                                                        //0xa
    ULONG EFlags;                                                           //0x10
    ULONG Fill2;                                                            //0x14
    ULONGLONG Rsp;                                                          //0x18
    USHORT SegSs;                                                           //0x20
    USHORT Fill3[ 3 ];                                                        //0x22
};

// NMI handler callback structure
struct knmi_handler_callback_t {
    knmi_handler_callback_t* Next;
    bool ( *Callback )( void*, bool );
    void* Context;
    void* Handle;
};

typedef struct kse_hook_t {
    std::uint64_t m_type; // 0: Function, 1: IRP Callback, 2: Last
    union {
        char* m_function_name; // If Type == 0
        std::uint64_t m_callback_id; // If Type == 1
    };
    void* m_hook_function;      // +16: Validated during registration (must be in driver)
    void* m_original_function;  // +24: Populated by us, swapped before patching
};

typedef struct kse_hook_collection_t {
    std::uint64_t m_type; // 0: NT Export, 1: HAL Export, 2: Driver Export, 3: Callback, 4: Last
    wchar_t* m_export_driver_name; // If Type == 2
    kse_hook_t* m_hook_array; // array of _KSE_HOOK
};

typedef struct kse_shim_t {
    std::size_t m_size;
    guid_t* m_shim_guid;
    wchar_t* m_shim_name;
    void* m_kse_callback_routines;
    void* m_shimmed_driver_targeted_notification;
    void* m_shimmed_driver_untargeted_notification;
    kse_hook_collection_t* m_hook_collections_array; // array of _KSE_HOOK_COLLECTION
};

typedef struct kse_engine_t {
    std::uint32_t m_disable_flags;        // 0x01: DisableDriverShims, 0x02: DisableDeviceShims
    std::uint32_t m_state;                // 0: Not Ready, 1: In Progress, 2: Ready
    std::uint32_t m_flags;                // 0x02: GroupPolicyOK, 0x800: DrvShimsActive, 0x1000: DevShimsActive
    list_entry_t m_providers_list_head;   // Registered shims list
    list_entry_t m_shimmed_drivers_list_head;
    void* m_kse_get_io_callbacks_routine;
    void* m_kse_set_completion_hook_routine;
    void* m_device_info_cache;
    void* m_hardware_id_cache;
    void* m_shimmed_driver_hint;
};

typedef struct driver_load_info_t {
    void* m_section_object;
    std::uint8_t m_pad1[ 40 ];
    std::uint64_t m_base_address;
    std::uint8_t m_pad2[ 16 ];
    std::uint32_t m_size;
    std::uint8_t m_pad3[ 52 ];
    std::uint32_t m_field_120;
    std::uint8_t m_pad4[ 32 ];
    std::uint32_t m_field_156;
};

typedef struct driver_info_t {
    std::uint8_t m_pad1[ 8 ];
    const wchar_t* m_name;
};

typedef struct shim_entry_t {
    std::uint64_t m_flink;
    std::uint64_t m_blink;
    std::uint64_t m_shim_ptr;
    std::uint64_t m_field_18;
    std::uint32_t m_flags;
    std::uint8_t m_pad1[ 4 ];
    std::uint64_t m_field_20;
    std::uint64_t m_driver_object;
    std::uint8_t m_pad2[ 72 ];
};

struct shim_resolve_entry_t {
    std::uint64_t m_entry[ 10 ];
};

typedef struct wnode_header_t {
    std::uint32_t m_buffer_size;        // Size of entire buffer inclusive of this ULONG
    std::uint32_t m_provider_id;        // Provider Id of driver returning this buffer
    union {
        std::uint64_t m_historical_context;  // Logger use
        struct {
            std::uint32_t m_version;         // Reserved
            std::uint32_t m_linkage;         // Linkage field reserved for WMI
        };
    };
    union {
        std::uint32_t m_count_lost;          // Reserved
        void* m_kernel_handle;               // Kernel handle for data block
        ularge_integer_t m_time_stamp;        // Timestamp as returned in units of 100ns since 1/1/1601
    };
    guid_t m_guid;                           // Guid for data block returned with results
    std::uint32_t m_client_context;
    std::uint32_t m_flags;                   // Flags, see below
};

typedef struct event_trace_properties_t {
    wnode_header_t m_wnode;
    std::uint32_t m_buffer_size;
    std::uint32_t m_minimum_buffers;
    std::uint32_t m_maximum_buffers;
    std::uint32_t m_maximum_file_size;
    std::uint32_t m_log_file_mode;
    std::uint32_t m_flush_timer;
    std::uint32_t m_enable_flags;
    std::int32_t m_age_limit;
    std::uint32_t m_number_of_buffers;
    std::uint32_t m_free_buffers;
    std::uint32_t m_events_lost;
    std::uint32_t m_buffers_written;
    std::uint32_t m_log_buffers_lost;
    std::uint32_t m_real_time_buffers_lost;
    void* m_logger_thread_id;
    std::uint32_t m_log_file_name_offset;
    std::uint32_t m_logger_name_offset;
};

typedef struct ckcl_trace_properties_t : event_trace_properties_t {
    std::uint64_t m_unknown[ 3 ];
    unicode_string_t m_provider_name;
};

enum class ckcl_trace_operation_t : std::uint32_t {
    start,
    syscall,
    end
};

enum class event_trace_information_class_t : std::uint32_t {
    event_trace_kernel_version_information,
    event_trace_group_mask_information,
    event_trace_performance_information,
    event_trace_time_profile_information,
    event_trace_session_security_information,
    event_trace_spinlock_information,
    event_trace_stack_tracing_information,
    event_trace_executive_resource_information,
    event_trace_heap_tracing_information,
    event_trace_heap_summary_tracing_information,
    event_trace_pool_tag_filter_information,
    event_trace_pebs_tracing_information,
    event_trace_profile_config_information,
    event_trace_profile_source_list_information,
    event_trace_profile_event_list_information,
    event_trace_profile_counter_list_information,
    event_trace_stack_caching_information,
    event_trace_object_type_filter_information,
    max_event_trace_info_class
};

typedef struct event_trace_profile_counter_information_t {
    event_trace_information_class_t m_event_trace_information_class;
    void* m_trace_handle;
    std::uint32_t m_profile_source[ 1 ];
} event_trace_profile_counter_information_t;

typedef struct event_trace_system_event_information_t {
    event_trace_information_class_t m_event_trace_information_class;
    void* m_trace_handle;
    std::uint32_t m_hook_id[ 1 ];
} event_trace_system_event_information_t;

typedef struct _dbgkd_debug_data_header64_t {
    std::uint64_t m_flink;
    std::uint64_t m_blink;
    std::uint32_t m_owner_tag;
    std::uint32_t m_size;
} dbgkd_debug_data_header64_t;

typedef struct _kd_debugger_data64_t {
    dbgkd_debug_data_header64_t m_header;
    std::uint64_t m_kern_base;
    std::uint64_t m_breakpoint_with_status;
    std::uint64_t m_saved_context;
    std::uint16_t m_th_callback_stack;
    std::uint16_t m_next_callback;
    std::uint16_t m_frame_pointer;
    std::uint16_t m_pae_enabled;
    std::uint64_t m_ki_call_user_mode;
    std::uint64_t m_ke_user_callback_dispatcher;
    std::uint64_t m_ps_loaded_module_list;
    std::uint64_t m_ps_active_process_head;
    std::uint64_t m_psp_cid_table;
    std::uint64_t m_exp_system_resources_list;
    std::uint64_t m_exp_paged_pool_descriptor;
    std::uint64_t m_exp_number_of_paged_pools;
    std::uint64_t m_ke_time_increment;
    std::uint64_t m_ke_bug_check_callback_list_head;
    std::uint64_t m_ki_bugcheck_data;
    std::uint64_t m_iop_error_log_list_head;
    std::uint64_t m_obp_root_directory_object;
    std::uint64_t m_obp_type_object_type;
    std::uint64_t m_mm_system_cache_start;
    std::uint64_t m_mm_system_cache_end;
    std::uint64_t m_mm_system_cache_ws;
    std::uint64_t m_mm_pfn_database;
    std::uint64_t m_mm_system_ptes_start;
    std::uint64_t m_mm_system_ptes_end;
    std::uint64_t m_mm_subsection_base;
    std::uint64_t m_mm_number_of_paging_files;
    std::uint64_t m_mm_lowest_physical_page;
    std::uint64_t m_mm_highest_physical_page;
    std::uint64_t m_mm_number_of_physical_pages;
    std::uint64_t m_mm_maximum_non_paged_pool_in_bytes;
    std::uint64_t m_mm_non_paged_system_start;
    std::uint64_t m_mm_non_paged_pool_start;
    std::uint64_t m_mm_non_paged_pool_end;
    std::uint64_t m_mm_paged_pool_start;
    std::uint64_t m_mm_paged_pool_end;
    std::uint64_t m_mm_paged_pool_information;
    std::uint64_t m_mm_page_size;
    std::uint64_t m_mm_size_of_paged_pool_in_bytes;
    std::uint64_t m_mm_total_commit_limit;
    std::uint64_t m_mm_total_committed_pages;
    std::uint64_t m_mm_shared_commit;
    std::uint64_t m_mm_driver_commit;
    std::uint64_t m_mm_process_commit;
    std::uint64_t m_mm_paged_pool_commit;
    std::uint64_t m_mm_extended_commit;
    std::uint64_t m_mm_zeroed_page_list_head;
    std::uint64_t m_mm_free_page_list_head;
    std::uint64_t m_mm_standby_page_list_head;
    std::uint64_t m_mm_modified_page_list_head;
    std::uint64_t m_mm_modified_no_write_page_list_head;
    std::uint64_t m_mm_available_pages;
    std::uint64_t m_mm_resident_available_pages;
    std::uint64_t m_pool_track_table;
    std::uint64_t m_non_paged_pool_descriptor;
    std::uint64_t m_mm_highest_user_address;
    std::uint64_t m_mm_system_range_start;
    std::uint64_t m_mm_user_probe_address;
    std::uint64_t m_kd_print_circular_buffer;
    std::uint64_t m_kd_print_circular_buffer_end;
    std::uint64_t m_kd_print_write_pointer;
    std::uint64_t m_kd_print_rollover_count;
    std::uint64_t m_mm_loaded_user_image_list;
    std::uint64_t m_nt_build_lab;
    std::uint64_t m_ki_normal_system_call;
    std::uint64_t m_ki_processor_block;
    std::uint64_t m_mm_unloaded_drivers;
    std::uint64_t m_mm_last_unloaded_driver;
    std::uint64_t m_mm_triage_action_taken;
    std::uint64_t m_mm_special_pool_tag;
    std::uint64_t m_kernel_verifier;
    std::uint64_t m_mm_verifier_data;
    std::uint64_t m_mm_allocated_non_paged_pool;
    std::uint64_t m_mm_peak_commitment;
    std::uint64_t m_mm_total_commit_limit_maximum;
    std::uint64_t m_cm_nt_csd_version;
    std::uint64_t m_mm_physical_memory_block;
    std::uint64_t m_mm_session_base;
    std::uint64_t m_mm_session_size;
    std::uint64_t m_mm_system_parent_table_page;
    std::uint64_t m_mm_virtual_translation_base;
    std::uint16_t m_offset_k_thread_next_processor;
    std::uint16_t m_offset_k_thread_teb;
    std::uint16_t m_offset_k_thread_kernel_stack;
    std::uint16_t m_offset_k_thread_initial_stack;
    std::uint16_t m_offset_k_thread_apc_process;
    std::uint16_t m_offset_k_thread_state;
    std::uint16_t m_offset_k_thread_b_store;
    std::uint16_t m_offset_k_thread_b_store_limit;
    std::uint16_t m_size_e_process;
    std::uint16_t m_offset_eprocess_peb;
    std::uint16_t m_offset_eprocess_parent_cid;
    std::uint16_t m_offset_eprocess_directory_table_base;
    std::uint16_t m_size_prcb;
    std::uint16_t m_offset_prcb_dpc_routine;
    std::uint16_t m_offset_prcb_current_thread;
    std::uint16_t m_offset_prcb_mhz;
    std::uint16_t m_offset_prcb_cpu_type;
    std::uint16_t m_offset_prcb_vendor_string;
    std::uint16_t m_offset_prcb_proc_state_context;
    std::uint16_t m_offset_prcb_number;
    std::uint16_t m_size_e_thread;
    std::uint64_t m_kd_print_circular_buffer_ptr;
    std::uint64_t m_kd_print_buffer_size;
    std::uint64_t m_ke_loader_block;
    std::uint16_t m_size_pcr;
    std::uint16_t m_offset_pcr_self_pcr;
    std::uint16_t m_offset_pcr_current_prcb;
    std::uint16_t m_offset_pcr_contained_prcb;
    std::uint16_t m_offset_pcr_initial_b_store;
    std::uint16_t m_offset_pcr_b_store_limit;
    std::uint16_t m_offset_pcr_initial_stack;
    std::uint16_t m_offset_pcr_stack_limit;
    std::uint16_t m_offset_prcb_pcr_page;
    std::uint16_t m_offset_prcb_proc_state_special_reg;
    std::uint16_t m_gdt_r0_code;
    std::uint16_t m_gdt_r0_data;
    std::uint16_t m_gdt_r0_pcr;
    std::uint16_t m_gdt_r3_code;
    std::uint16_t m_gdt_r3_data;
    std::uint16_t m_gdt_r3_teb;
    std::uint16_t m_gdt_ldt;
    std::uint16_t m_gdt_tss;
    std::uint16_t m_gdt64_r3_cm_code;
    std::uint16_t m_gdt64_r3_cm_teb;
    std::uint64_t m_iop_num_triage_dump_data_blocks;
    std::uint64_t m_iop_triage_dump_data_blocks;
    std::uint64_t m_vf_crash_data_block;
    std::uint64_t m_mm_bad_pages_detected;
    std::uint64_t m_mm_zeroed_page_single_bit_errors_detected;
    std::uint64_t m_etwp_debugger_data;
    std::uint16_t m_offset_prcb_context;
    std::uint16_t m_offset_prcb_max_breakpoints;
    std::uint16_t m_offset_prcb_max_watchpoints;
    std::uint32_t m_offset_k_thread_stack_limit;
    std::uint32_t m_offset_k_thread_stack_base;
    std::uint32_t m_offset_k_thread_queue_list_entry;
    std::uint32_t m_offset_e_thread_irp_list;
    std::uint16_t m_offset_prcb_idle_thread;
    std::uint16_t m_offset_prcb_normal_dpc_state;
    std::uint16_t m_offset_prcb_dpc_stack;
    std::uint16_t m_offset_prcb_isr_stack;
    std::uint16_t m_size_kdpc_stack_frame;
    std::uint16_t m_offset_k_pri_queue_thread_list_head;
    std::uint16_t m_offset_k_thread_wait_reason;
    std::uint16_t m_padding;
    std::uint64_t m_pte_base;
    std::uint64_t m_retpoline_stub_function_table;
    std::uint32_t m_retpoline_stub_function_table_size;
    std::uint32_t m_retpoline_stub_offset;
    std::uint32_t m_retpoline_stub_size;
} kd_debugger_data64_t;


using pknormal_routine_t = void ( NTAPI* )( void* normal_context,
    void* system_arg1,
    void* system_arg2 );

using pkkernel_routine_t = void ( NTAPI* )( struct kapc_t* apc,
    pknormal_routine_t* normal_routine,
    void** normal_context,
    void** system_arg1,
    void** system_arg2 );

using pkrundown_routine_t = void ( NTAPI* )( struct kapc_t* apc );

enum class kprocessor_mode_t : std::uint8_t {
    kernel_mode = 0,
    user_mode = 1
};

struct kapc_t {
    std::uint8_t         m_type;
    std::uint8_t         m_spare_byte0;
    std::uint8_t         m_size;
    std::uint8_t         m_spare_byte1;
    std::uint32_t        m_spare_long0;
    kthread_t* m_thread;
    list_entry_t         m_apc_list_entry;
    pkkernel_routine_t   m_kernel_routine;
    pkrundown_routine_t  m_rundown_routine;
    pknormal_routine_t   m_normal_routine;
    void* m_normal_context;
    void* m_system_argument1;
    void* m_system_argument2;
    std::int8_t          m_apc_state_index;
    kprocessor_mode_t    m_apc_mode;
    bool                 m_inserted;
};

struct context_return_t {
    void* m_stack;
    std::uint64_t m_function;
    std::uint64_t m_rcx;
    std::uint64_t m_rdx;
    std::uint64_t m_r8;
    std::uint64_t m_r9;
    std::uint64_t m_rax;

    std::uint64_t m_r12;
    std::uint64_t m_r13;
    std::uint64_t m_r14;
    std::uint64_t m_r15;

    std::uint64_t m_rdi;
    std::uint64_t m_rsi;
    std::uint64_t m_rbx;
    std::uint64_t m_rbp;

    std::uint64_t m_rflags;
};

typedef void ( *pworker_thread_routine_t )( void* context );

enum class work_queue_type_t : std::uint32_t {
    critical_work_queue,
    delayed_work_queue,
    hyper_critical_work_queue,
    normal_work_queue,
    background_work_queue,
    real_time_work_queue,
    super_critical_work_queue,
    maximum_work_queue
};

struct work_queue_item_t {
    list_entry_t m_list_entry;
    pworker_thread_routine_t m_worker_routine;
    void* m_parameter;
};

struct sid_identifier_authority_t {
    std::uint8_t m_value[ 6 ];
};

struct sid_t {
    std::uint8_t m_revision;
    std::uint8_t m_sub_authority_count;
    sid_identifier_authority_t m_identifier_authority;
    std::uint32_t m_sub_authority[ 1 ];
};

struct sid_and_attributes_t {
    sid_t* m_sid;
    std::uint32_t m_atributes;
};

struct token_user_t {
    sid_and_attributes_t m_user;
};

struct sid_internal_t {
    std::uint8_t m_revision;
    std::uint8_t m_sub_authority_count;
    sid_identifier_authority_t m_identifier_authority;
    std::uint32_t m_sub_authority[ 5 ];
};

enum class fs_information_class_t : std::uint32_t {
    fs_volume_information = 1,
    fs_label_information = 2,
    fs_size_information = 3,
    fs_device_information = 4,
    fs_attribute_information = 5,
    fs_control_information = 6,
    fs_full_size_information = 7,
    fs_objectid_information = 8,
    fs_driver_path_information = 9,
    fs_volume_flags_information = 10,
    fs_sector_size_information = 11
};

struct file_fs_volume_information_t {
    ularge_integer_t m_volume_creation_time;
    std::uint32_t m_volume_serial_number;
    std::uint32_t m_volume_label_length;
    std::uint8_t m_supports_objects;
    wchar_t m_volume_label[ 1 ];
};

struct file_fs_device_information_t {
    std::uint32_t m_device_type;
    std::uint32_t m_characteristics;
};

struct file_fs_attribute_information_t {
    std::uint32_t m_file_system_attributes;
    std::int32_t m_maximum_component_name_length;
    std::uint32_t m_file_system_name_length;
    wchar_t m_file_system_name[ 1 ];
};

struct file_fs_size_information_t {
    ularge_integer_t m_total_allocation_units;
    ularge_integer_t m_available_allocation_units;
    std::uint32_t m_sectors_per_allocation_unit;
    std::uint32_t m_bytes_per_sector;
};

struct file_fs_full_size_information_t {
    ularge_integer_t m_total_allocation_units;
    ularge_integer_t m_caller_available_allocation_units;
    ularge_integer_t m_actual_available_allocation_units;
    std::uint32_t m_sectors_per_allocation_unit;
    std::uint32_t m_bytes_per_sector;
};

enum thread_information_class_t : std::uint32_t {
    thread_basic_information = 0,
    thread_hide_from_debugger = 17,
    thread_break_on_termination = 18,
    thread_set_tlsarray_address = 20,
    thread_is_io_pending = 16,
    thread_query_set_win32_start_address = 9,
    thread_zero_tlsinfo = 27,
    thread_instrumentation_callback = 40,
};

struct rtl_process_module_information_t {
    void* m_section;
    void* m_mapped_base;
    void* m_image_base;
    unsigned long m_image_size;
    unsigned long m_flags;
    unsigned short m_load_order_index;
    unsigned short m_init_order_index;
    unsigned short m_load_count;
    unsigned short m_offset_to_file_name;
    unsigned char m_full_path_name[ 256 ];
};

struct rtl_process_modules {
    unsigned long m_number_of_modules;
    rtl_process_module_information_t m_modules[ 1 ];
};

const char m_pdb_magic[ 32 ] = {
    0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66, 0x74, 0x20, 0x43, 0x2F,
    0x43, 0x2B, 0x2B, 0x20, 0x4D, 0x53, 0x46, 0x20, 0x37, 0x2E, 0x30, 0x30,
    0x0D, 0x0A, 0x1A, 0x44, 0x53, 0x00, 0x00, 0x00
};

#pragma pack(push, 1)
struct pdb_super_block_t {
    char m_file_magic[ sizeof( m_pdb_magic ) ];
    unsigned long m_block_size;
    unsigned long m_free_block_map_block;
    unsigned long m_num_blocks;
    unsigned long m_num_directory_bytes;
    unsigned long m_unknown;
    unsigned long m_block_map_addr;
};

struct pdb_stream_data_t {
    char* m_stream_pointer;
    size_t m_stream_size;
};

struct pdb_dbi_header_t {
    long m_version_signature;
    unsigned long m_version_header;
    unsigned long m_age;
    unsigned short m_global_stream_index;
    unsigned short m_build_number;
    unsigned short m_public_stream_index;
    unsigned short m_pdb_dll_version;
    unsigned short m_sym_record_stream;
    unsigned short m_pdb_dll_rbld;
    long m_mod_info_size;
    long m_section_contribution_size;
    long m_section_map_size;
    long m_source_info_size;
    long m_type_server_size;
    unsigned long m_mfc_type_server_index;
    long m_optional_dbg_header_size;
    long m_ec_substream_size;
    unsigned short m_flags;
    unsigned short m_machine;
    unsigned long m_padding;
};

struct pdb_pubsym32_t {
    unsigned short m_rec_len;
    unsigned short m_rec_type;
    unsigned long m_pubsym_flags;
    unsigned long m_offset;
    unsigned short m_segment;
    char m_name[ 1 ];
};

struct file_standard_info_t {
    std::int64_t allocation_size;
    std::int64_t end_of_file;
    std::uint32_t number_of_links;
    std::uint8_t delete_pending;
    std::uint8_t directory;
};

enum pdb_sym_type_e {
    // [...]
    sym_constant = 0x1107,   // constant symbol
    sym_udt = 0x1108,        // User defined type
    sym_ldata32 = 0x110c,    // Module-local symbol
    sym_gdata32 = 0x110d,    // Global data symbol
    sym_pub32 = 0x110e,      // a public symbol (CV internal reserved)
    sym_procref = 0x1125,    // Reference to a procedure
    sym_lprocref = 0x1127,   // Local Reference to a procedure
    // [...]
};

struct pdb_symbol_data_t {
    char* m_symbol_name;
    unsigned long m_section_offset;
    unsigned long m_symbol_rva;
    unsigned short m_section_number;
};
#pragma pack(pop)

struct file_standard_information_t {
    ularge_integer_t m_allocation_size;
    ularge_integer_t m_end_of_file;
    unsigned long m_number_of_links;
    unsigned char m_delete_pending;
    unsigned char m_directory;
};

struct pdb_info_t {
    std::uint32_t m_signature;
    guid_t m_guid;
    std::uint32_t m_age;
    char m_file_name[ 1 ];
};

struct ansi_string_t {
    std::uint16_t m_length;
    std::uint16_t m_maximum_length;
    char* m_buffer;
};

typedef enum e_key_value_information_class {
    key_value_basic_information,
    key_value_full_information,
    key_value_partial_information,
    key_value_full_information_align64,
    key_value_partial_information_align64,
    key_value_layer_information,
    max_key_value_info_class
};

typedef struct memory_basic_information_t {
    void* m_base_address;
    void* m_allocation_base;
    unsigned long m_allocation_protect;
    unsigned short m_partition_id;
    std::uint64_t m_region_size;
    unsigned long m_state;
    unsigned long m_protect;
    unsigned long m_type;
};

struct memory_range_entry_t {
    void* m_virtual_address;
    std::size_t m_number_of_bytes;
};

static_assert( sizeof( kse_hook_t ) == 32, "kse_hook_t size mismatch" );
static_assert( sizeof( kse_hook_collection_t ) == 24, "kse_hook_collection_t size mismatch" );
static_assert( sizeof( kse_shim_t ) == 56, "kse_shim_t size mismatch" );