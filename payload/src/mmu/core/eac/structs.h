#pragma once

namespace eac {
    std::uint64_t m_module_base{ };
    std::uint32_t m_module_size{ };

    std::uint64_t m_allocate_virtual{ };
    std::uint64_t m_free_virtual{ };
    std::uint64_t m_flush_virtual{ };

    std::uint64_t m_page_memory{ };
    std::uint64_t m_protect_virtual{ };
    std::uint64_t m_query_virtual{ };
    std::uint64_t m_set_information_virtual{ };

    std::uint64_t m_create_callback{ };

    std::uint64_t m_read_virtual{ };
    std::uint64_t m_write_virtual{ };

    std::uint64_t m_attach_process{ };
    std::uint64_t m_detach_process{ };

    struct control_t {
        void* m_control_type;
    };

    struct protect_control_t : public control_t {
        void* m_address;
        std::size_t m_size;
        int m_new_protection;
        int m_old_protection;
    };

    struct read_control_t : public control_t {
        void* m_source_address;
        std::size_t m_size;
        void* m_destination_buffer;
        std::size_t m_bytes_read;
    };

    struct write_control_t : public control_t {
        void* m_source_buffer;
        std::size_t m_size;
        void* m_destination_address;
        std::size_t m_bytes_written;
    };

    struct query_control_t : public control_t {
        void* m_address;
        std::uint32_t m_information_class;
        std::uint32_t m_padding;
        void* m_information_buffer;
        std::uint32_t m_information_length;
        std::uint32_t m_return_length;
    };

    struct flush_control_t : public control_t {
        void* m_address;
        std::size_t m_region_size;
        io_status_block_t m_io_status;
    };

    struct set_information_control_t : public control_t {
        std::uint32_t m_vm_information_class;
        void* m_virtual_addresses;
        std::uint32_t m_number_of_entries;
        void* m_vm_information;
        std::uint32_t m_information_length;
    };
}