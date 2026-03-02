#pragma once

namespace hyperspace {
    bool initialize( );
    bool is_active( );
    void unload( );
    void unhook( );

    namespace root {
        void poll_logs( );
    }

    namespace eac {
        bool is_active( );

        std::uint64_t allocate_virtual( std::size_t size, std::uint32_t protection );
        bool free_virtual( std::uint64_t allocation_base, std::size_t size );

        bool attach_process( eprocess_t* process, kapc_state_t* apc_state );
        bool detach_process( eprocess_t* process, kapc_state_t apc_state );

        bool read_virtual( std::uint64_t source, void* destination, std::size_t size );
        bool write_virtual( std::uint64_t source, void* destination, std::size_t size );

        bool page_memory( std::uint64_t address, std::uint64_t size );
        bool flush_virtual( std::uint64_t* address, std::size_t* size );

        bool set_information_virtual( std::uint64_t address, std::size_t size );
        bool protect_virtual( std::uint64_t& address, std::uint32_t& size, std::uint32_t protection, std::uint32_t* old_protection );
        bool query_virtual( std::uint64_t address, memory_basic_information_t* memory_info );
    }

    namespace paging {
        extern std::uint64_t m_process_cr3;
        extern std::uint64_t m_system_cr3;

        std::uint64_t get_process_cr3( eprocess_t* eprocess );
        std::uint64_t swap_context( std::uint64_t process_cr3 );

        bool attach_process( eprocess_t* eprocess, eprocess_t** old_eprocess = nullptr );
        bool detach_process( eprocess_t* old_eprocess );

        // Cached allocations (using EAC if active or Remapping)
        std::uint64_t allocate_virtual( std::size_t size, std::uint32_t protection = 0x40 );
        bool free_virtual( std::uint64_t allocation_base );

        bool hyperspace_entries( pt_entries_t& pt_entries, std::uint64_t va );
        bool set_page_protection( std::uint64_t va, page_protection protection, bool supervisor = false );
        std::uint64_t remap_physical_page( std::uint64_t va );

        bool is_address_valid( std::uint64_t pa );
        bool translate_linear( std::uint64_t va, std::uint64_t* pa );
        std::uint64_t translate_linear( std::uint64_t va ); // Cached Virtual Address Space

		// Copy-On-Write
        namespace cow {
            bool create_exception( std::uint64_t va );
        }

        // Shared Virtual Address Space
        namespace context {
            bool create_hyperspace( );
            bool allocate_virtual( std::uint32_t size, bool high_address = false );

            bool switch_to_hyperspace( );
            bool switch_from_hyperspace( );
        }

        // Direct Page Maniplation
        namespace dpm {
            bool read_physical( void* dst, std::uint64_t pa, std::size_t size );
            bool write_physical( void* dst, std::uint64_t pa, std::size_t size );

            bool read_virtual( void* dst, std::uint64_t pa, std::size_t size );
            bool write_virtual( void* dst, std::uint64_t pa, std::size_t size );
        }

        // Page-Table Maniplation
        namespace ptm {
            void* map_process_page( std::uint64_t pa, page_protection protection, bool supervisor = false );
            bool read_physical( void* dst, std::uint64_t pa, std::size_t size );
            bool write_physical( std::uint64_t pa, void* dst, std::size_t size );
        }

        // Extend VAD for allocation
        namespace vad {
            std::uint64_t allocate_virtual( std::size_t size );
        }

        // Remap memory from csrss.exe
        namespace remap {
            std::uint64_t allocate_virtual( std::size_t size );
        }
    }

    namespace target {
        extern std::uint32_t m_thread_id;
        extern std::uint32_t m_process_id;
        extern HWND m_window_handle;
        extern eprocess_t* m_eprocess;
        extern peb_t* m_process_peb;
        extern std::uint64_t m_base_address;

        std::uint32_t get_process_id( std::wstring process_name );
        HWND get_window_handle( std::uint32_t process_pid );
        eprocess_t* get_eprocess( std::uint32_t process_id );
        std::uint64_t get_base_address( eprocess_t* eprocess );
        peb_t* get_process_peb( eprocess_t* eprocess );
        std::uintptr_t get_process_module( const wchar_t* module_name );
        std::uintptr_t get_module_export( std::uint64_t module_base, const char* export_name );

        bool benchmark_read( std::chrono::milliseconds duration = std::chrono::seconds( 1 ) );
        bool attach_process( const wchar_t* process_name );
        void detach_process( );
    }

    template <typename addr_t>
    bool read_virt( addr_t va, void* buffer, size_t size ) {
        std::uint64_t va64;
        if constexpr ( std::is_pointer_v<addr_t> ) {
            va64 = reinterpret_cast< std::uint64_t >( va );
        }
        else if constexpr ( std::is_integral_v<addr_t> ) {
            va64 = static_cast< std::uint64_t >( va );
        }
        else {
            static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
                           "addr_t must be pointer or integral" );
        }

        return paging::dpm::read_virtual( buffer, va64, size );
    }

    template <typename addr_t>
    bool read_phys( addr_t pa, void* buffer, size_t size ) {
        std::uint64_t pa64;
        if constexpr ( std::is_pointer_v<addr_t> ) {
            pa64 = reinterpret_cast< std::uint64_t >( pa );
        }
        else if constexpr ( std::is_integral_v<addr_t> ) {
            pa64 = static_cast< std::uint64_t >( pa );
        }
        else {
            static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
                           "addr_t must be pointer or integral" );
        }

        return paging::dpm::read_physical( buffer, pa64, size );
    }

    template <typename addr_t>
    bool write_virt( addr_t va, void* buffer, size_t size ) {
        std::uint64_t va64;
        if constexpr ( std::is_pointer_v<addr_t> ) {
            va64 = reinterpret_cast< std::uint64_t >( va );
        }
        else if constexpr ( std::is_integral_v<addr_t> ) {
            va64 = static_cast< std::uint64_t >( va );
        }
        else {
            static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
                           "addr_t must be pointer or integral" );
        }

        return paging::dpm::write_virtual( buffer, va64, size );
    }

    template <typename addr_t>
    bool write_phys( addr_t pa, void* buffer, size_t size ) {
        std::uint64_t pa64;
        if constexpr ( std::is_pointer_v<addr_t> ) {
            pa64 = reinterpret_cast< std::uint64_t >( pa );
        }
        else if constexpr ( std::is_integral_v<addr_t> ) {
            pa64 = static_cast< std::uint64_t >( pa );
        }
        else {
            static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
                           "addr_t must be pointer or integral" );
        }

        return paging::dpm::write_physical( buffer, pa64, size );
    }

    template <typename ret_t = std::uint64_t, typename addr_t>
    ret_t read_virt( addr_t va ) {
        ret_t buffer{ };
        if ( !read_virt(
            va,
            &buffer,
            sizeof( ret_t ) ) )
            return ret_t{ };
        return buffer;
    }

    template <typename ret_t = std::uint64_t, typename addr_t>
    ret_t read_phys( addr_t pa ) {
        ret_t buffer{ };
        if ( !read_phys(
            pa,
            &buffer,
            sizeof( ret_t ) ) )
            return ret_t{ };
        return buffer;
    }

    template <typename value_t, typename addr_t>
    bool write_virt( addr_t va, const value_t& value ) {
        return write_virt(
            va,
            const_cast< value_t* >( &value ),
            sizeof( value_t )
        );
    }

    template <typename value_t, typename addr_t>
    bool write_phys( addr_t pa, const value_t& value ) {
        return write_phys(
            pa,
            const_cast< value_t* >( &value ),
            sizeof( value_t )
        );
    }
}