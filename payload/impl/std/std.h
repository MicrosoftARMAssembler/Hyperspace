 #include <intrin.h>
namespace std {
    using int8_t = signed char;
    using int16_t = short;
    using int32_t = int;
    using int64_t = long long;
    using uint8_t = unsigned char;
    using uint16_t = unsigned short;
    using uint32_t = unsigned int;
    using uint64_t = unsigned long long;

    using int_least8_t = signed char;
    using int_least16_t = short;
    using int_least32_t = int;
    using int_least64_t = long long;
    using uint_least8_t = unsigned char;
    using uint_least16_t = unsigned short;
    using uint_least32_t = unsigned int;
    using uint_least64_t = unsigned long long;

    using int_fast8_t = signed char;
    using int_fast16_t = int;
    using int_fast32_t = int;
    using int_fast64_t = long long;
    using uint_fast8_t = unsigned char;
    using uint_fast16_t = unsigned int;
    using uint_fast32_t = unsigned int;
    using uint_fast64_t = unsigned long long;

    using uintptr_t = unsigned long long;
    using size_t = unsigned long long;
    using intmax_t = long long;
    using uintmax_t = long long;
    using ptrdiff_t = long long;

    using double_t = double;
    using float_t = float;

    struct m128a_t {
        std::uint64_t m_low;
        std::int64_t m_high;
    };

    struct uint128_t {
        std::uint64_t m_low;
        std::uint64_t m_high;

        uint128_t( ) : m_low( 0 ), m_high( 0 ) { }
        uint128_t( std::uint64_t low, std::uint64_t high ) :
            m_low( low ), m_high( high ) {
        }

        bool operator==( const uint128_t& other ) const {
            return m_low == other.m_low && m_high == other.m_high;
        }

        bool operator!=( const uint128_t& other ) const {
            return !( *this == other );
        }

        uint128_t& operator=( std::uint64_t value ) {
            m_low = value;
            m_high = 0;
            return *this;
        }
    };
}


enum nt_status_t {
    success,
    unsuccessful = 0xc0000001,
    alerted = 0x101,
    timeout = 0x102,
    pending = 0x103,
    control_c_exit = 0xc000013a,
    buffer_too_small = 0xc0000023,
    info_length_mismatch = 0xc4l,
    insufficient_resources = 0xc9A,
    length_mismatch = 0xc4,
    invalid_parameter = 0xcd,
    access_violation = 0xc5,
    cancelled = 0xc0000120,
    not_supported = 0xc00000bb
};

namespace std {
    void* memset( void* dest, int c, size_t count ) {
        unsigned char* ptr;
        size_t			i;

        i = 0;
        ptr = ( unsigned char* )dest;
        while ( i < count ) {
            ptr[ i++ ] = c;
        }
        return ( dest );
    }
}

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
        return m_magic ==pe_magic_t::dos_header;
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

struct thunk_data_t {
    union {
        std::uint64_t m_forwarder_string;
        std::uint64_t m_function;
        std::uint64_t m_address_of_data;
        std::uint64_t m_ordinal;
    };
};

struct import_by_name_t {
    std::uint16_t m_hint;
    char m_name[ 1 ];
};

struct debug_directory_t {
    std::uint32_t m_characteristics;
    std::uint32_t m_time_date_stamp;
    std::uint16_t m_major_version;
    std::uint16_t m_minor_version; 
    std::uint32_t m_type;
    std::uint32_t m_size_of_data; 
    std::uint32_t m_address_of_raw_data;
    std::uint32_t m_pointer_to_raw_data;
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

