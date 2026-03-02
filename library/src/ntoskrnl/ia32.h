#pragma once
#define in_range(x, a, b) (x >= a && x <= b) 
#define get_bits(x) (in_range(x, '0', '9') ? (x - '0') : ((x - 'A') + 0xA))
#define get_byte(x) ((BYTE)(get_bits(x[0]) << 4 | get_bits(x[1])))
#define putchar(c) putc((c),stdout)

typedef ULONG64 WNF_STATE_NAME;
typedef ULONG WNF_CHANGE_STAMP;

typedef struct _WNF_TYPE_ID* PWNF_TYPE_ID;
typedef const struct _WNF_TYPE_ID* PCWNF_TYPE_ID;

#ifndef LOGICAL
typedef ULONG LOGICAL;
#endif

typedef struct _WNF_STATE_NAME_INTERNAL {
    ULONG Data[ 2 ];
} WNF_STATE_NAME_INTERNAL, * PWNF_STATE_NAME_INTERNAL;

typedef NTSTATUS( NTAPI* nt_update_wnf_state_data_t )(
    _In_ const WNF_STATE_NAME* StateName,
    _In_reads_bytes_opt_( Length ) const VOID* Buffer,
    _In_opt_ ULONG Length,
    _In_opt_ PCWNF_TYPE_ID TypeId,
    _In_opt_ const VOID* ExplicitScope,
    _In_ WNF_CHANGE_STAMP MatchingChangeStamp,
    _In_ LOGICAL CheckStamp
    );


enum nt_status_t {
    success,
    unsuccessful = 0xc0000001,
    alerted = 0x101,
    timeout = 0x102,
    pending = 0x103,
    control_c_exit = 0xc000013a,
    info_length_mismatch = 0xc4l,
    insufficient_resources = 0xc9A,
    length_mismatch = 0xc4,
    invalid_parameter = 0xcd,
    access_violation = 0xc5,
    cancelled = 0xc0000120,
    not_supported = 0xc00000bb
};

enum nt_build_t {
    win11_23h2 = 0x589c,
    win11_22h2 = 0x585d,
    win11_21h2 = 0x55f0,
    win10_22h2 = 0x5a63,
    win10_21h1 = 0x4fc6,
    win10_20h2 = 0x4ec2,
    win10_20h1 = 0x4a61,
    win_server_2022 = 0x5900,
    win_server_2019 = 0x3c5a,
    win_server_2016 = 0x23f0,
    win8_1_update = 0x1db0,
    win8_1 = 0x1a2b,
    win7_sp1 = 0x1db1,
    win7_rtm = 0x1a28
};

enum pe_magic_t {
    dos_header = 0x5a4d,
    nt_headers = 0x4550,
    opt_header = 0x020b
};

struct unicode_string_t {
    std::uint16_t m_length;
    std::uint16_t m_maximum_length;
    wchar_t* m_buffer;
};

struct security_descriptor_t {
    std::uint8_t m_revision;
    std::uint8_t m_sbz1;
    std::uint16_t m_control;
    void* m_owner;
    void* m_group;
    void* m_sacl;
    void* m_dacl;
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

    [[ nodiscard ]]
    constexpr bool is_valid( ) {
        return m_magic == pe_magic_t::dos_header;
    }
};

struct data_directory_t {
    std::int32_t m_virtual_address;
    std::int32_t m_size;

    template< class type_t, typename addr_t >
    [[ nodiscard ]]
    type_t as_rva(
        addr_t rva
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

    [[ nodiscard ]]
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

struct list_entry_t {
    list_entry_t* m_flink;
    list_entry_t* m_blink;
};

struct single_list_entry_t {
    single_list_entry_t* m_next;
};

struct rtl_process_module_information_t {
    HANDLE  m_section;
    void* m_mapped_base;
    void* m_image_base;
    uint32_t m_image_size;
    uint32_t m_flags;
    uint16_t m_load_order_index;
    uint16_t m_init_order_index;
    uint16_t m_load_count;
    uint16_t m_offset_to_file_name;
    uint8_t  m_full_path[ 256 ];
};

struct rtl_process_modules_t {
    uint32_t    m_count;
    rtl_process_module_information_t m_modules[ 1 ];
};

#pragma once
struct driver_object_t;

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

typedef union _cr3 {
    std::uint64_t flags;

    struct {
        std::uint64_t pcid : 12;
        std::uint64_t pfn : 36;
        std::uint64_t reserved_1 : 12;
        std::uint64_t reserved_2 : 3;
        std::uint64_t pcid_invalidate : 1;
    };
} cr3, * pcr3;


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

struct ktimer_t {
    dispatcher_header_t m_header;
    std::uint64_t m_due_time;
    list_entry_t m_timer_list_entry;
    struct kdpc_t* m_dpc;
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
    void* m_trap_frame;                     // +0x090

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

struct mm_copy_address_t {
    union {
        std::uint64_t m_virtual_address;    // +0x000
        physical_address_t m_physical_address;   // +0x000
    };
}; // Size: 0x008


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

struct process_instrumentation_callback_t {
    ULONG m_version;
    ULONG m_reserved;
    void* m_callback;
};

typedef NTSTATUS( NTAPI* nt_set_process_information_t )(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength
    );


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