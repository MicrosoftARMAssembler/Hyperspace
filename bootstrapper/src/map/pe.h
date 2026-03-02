#pragma once

namespace map {
    struct reloc_entry_t {
        std::uint32_t m_to_rva;
        std::uint32_t m_size;
        struct {
            std::uint16_t m_offset : 12;
            std::uint16_t m_type : 4;
        } m_item[ 1 ];
    };

    struct pdb_symbols_t {
        std::uint64_t m_mi_copy_on_write;
        std::uint64_t m_mm_set_page_protection;
        std::uint64_t m_mi_is_page_on_bad_list;
        std::uint64_t m_mi_make_page_bad;
        std::uint64_t m_rtlp_debug_print_callback_list;
        std::uint64_t m_psp_load_image_notify_routine;
        std::uint64_t m_ki_is_process_termination_requested;
        std::uint64_t m_ps_query_thread_start_address;
        std::uint64_t m_mi_lock_page_table_page;
        std::uint64_t m_mm_initialize_process_address_space;
        std::uint64_t m_ke_get_prcb;
        std::uint64_t m_ki_nmi_callback_list_head;
        std::uint64_t m_po_idle;
        std::uint64_t m_etwp_max_pmc_counter;
        std::uint64_t m_etwp_debugger_data;
        std::uint64_t m_mi_obtain_section_for_driver;
        std::uint64_t m_mm_allocate_independent_pages;
        std::uint64_t m_mm_free_independent_pages;
        std::uint64_t m_ki_dynamic_trace_mask;
        std::uint64_t m_ps_enum_process_threads;
        std::uint64_t m_ps_integrity_check_enabled;
        std::uint64_t m_exp_license_watch_init_worker;
        std::uint64_t m_ki_filter_fiber_context;
        std::uint64_t m_ki_sw_interrupt_dispatch;
        std::uint64_t m_fs_rtl_uninitialize_small_mcb;
        std::uint64_t m_ki_dispatch_exception;
        std::uint64_t m_ki_expand_kernel_stack_and_callout;
        std::uint64_t m_fs_rtl_mdl_read_complete_dev_ex;
        std::uint64_t m_ki_initialize_boot_structures;
        std::uint64_t m_halp_stall_counter;
        std::uint64_t m_kd_trap;
        std::uint64_t m_ke_freeze_execution;
        std::uint64_t m_ke_stall_execution_processor;
        std::uint64_t m_kdp_report;
        std::uint64_t m_kdp_trap;
        std::uint64_t m_kd_enter_debugger;
        std::uint64_t m_nt_global_flag;
        std::uint64_t m_kdp_debug_routine_select;
        std::uint64_t m_kd_debugger_lock;
        std::uint64_t m_kd_ignore_um_exceptions;
    };
}