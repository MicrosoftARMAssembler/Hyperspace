#pragma once

namespace eac {
	namespace memory {
        std::uint64_t allocate_virtual( std::uint64_t process_id, std::uint32_t size, std::uint32_t protect ) {
            if ( !m_allocate_virtual )
                return false;

            using function_t = std::uint64_t( __fastcall* )( std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint32_t );
            return reinterpret_cast< function_t >( m_allocate_virtual )( 
                process_id, 0, size, 0, protect );
        }

        bool free_virtual( std::uint64_t process_id, std::uint64_t allocation_base, std::uint32_t size ) {
            if ( !m_free_virtual )
                return false;

            using function_t = bool( __fastcall* )( std::uint64_t, std::uint64_t, std::uint64_t );
            return reinterpret_cast< function_t >( m_free_virtual )(
                process_id, allocation_base, size );
        }

        bool read_virtual(
            eprocess_t* eprocess,
            std::uint64_t source_address,
            void* destination_buffer,
            std::size_t size,
            std::size_t* bytes_read = nullptr ) {

            if ( !m_read_virtual )
                return false;

            if ( !destination_buffer || !size || !source_address )
                return false;

            read_control_t control{};
            control.m_control_type = reinterpret_cast< void* >( 2 );
            control.m_source_address = reinterpret_cast< void* >( source_address );
            control.m_size = size;
            control.m_destination_buffer = destination_buffer;
            control.m_bytes_read = 0;

            using function_t = bool( __fastcall* )( eprocess_t*, read_control_t* );
            auto result = reinterpret_cast< function_t >( m_read_virtual )( eprocess, &control );
            if ( !result )
                return false;

            if ( bytes_read )
                *bytes_read = control.m_bytes_read;

            return true;
        }

        bool write_virtual(
            eprocess_t* eprocess,
            std::uint64_t destination_address,
            const void* source_buffer,
            std::size_t size,
            std::size_t* bytes_written = nullptr ) {

            if ( !m_write_virtual )
                return false;

            if ( !source_buffer || !size || !destination_address )
                return false;

            write_control_t control{};
            control.m_control_type = reinterpret_cast< void* >( 3 );
            control.m_source_buffer = const_cast< void* >( source_buffer );
            control.m_size = size;
            control.m_destination_address = reinterpret_cast< void* >( destination_address );
            control.m_bytes_written = 0;

            using function_t = bool( __fastcall* )( eprocess_t*, write_control_t* );
            auto result = reinterpret_cast< function_t >( m_write_virtual )( eprocess, &control );
            if ( !result )
                return false;

            if ( bytes_written )
                *bytes_written = control.m_bytes_written;

            return true;
        }

        bool flush_virtual( eprocess_t* eprocess, std::uint64_t* base_address, std::size_t* size ) {
            if ( !m_flush_virtual )
                return false;

            flush_control_t flush_control{ };
            flush_control.m_control_type = reinterpret_cast< void* >( 6 );
            flush_control.m_address = reinterpret_cast< void* >( *base_address );
            flush_control.m_region_size = *size;

            using function_t = bool( __fastcall* )( eprocess_t*, flush_control_t* );
            auto result = reinterpret_cast< function_t >( m_flush_virtual )( eprocess, &flush_control );
            if ( !result )
                return false;

            *size = flush_control.m_region_size;
            *base_address = reinterpret_cast< std::uint64_t >(
                flush_control.m_address
                );

            return result;
        }

        bool set_information_virtual( eprocess_t* eprocess, void* base_address, std::size_t size ) {
            if ( !m_set_information_virtual )
                return false;

            memory_range_entry_t range{};
            range.m_virtual_address = base_address;
            range.m_number_of_bytes = size;

            std::uint8_t dummy_buffer[ 1 ] = { 0 };

            set_information_control_t information_control{};
            information_control.m_control_type = reinterpret_cast< void* >( 8 );
            information_control.m_vm_information_class = 0;
            information_control.m_virtual_addresses = &range;
            information_control.m_number_of_entries = 1;
            information_control.m_vm_information = dummy_buffer;
            information_control.m_information_length = 0;

            using function_t = bool( __fastcall* )( eprocess_t*, set_information_control_t* );
            auto result = reinterpret_cast< function_t >( m_set_information_virtual )( eprocess, &information_control );
            if ( !result )
                return false;

            return true;
        }

        bool protect_virtual( eprocess_t* eprocess, std::uint64_t* base_address, std::size_t* size, std::uint32_t protection, std::uint32_t* old_protection ) {
            if ( !m_protect_virtual )
                return false;

            protect_control_t protect_control{ };
            protect_control.m_control_type = reinterpret_cast< void* >( 4 );
            protect_control.m_address = reinterpret_cast< void* >( *base_address );
            protect_control.m_new_protection = protection;
            protect_control.m_size = *size;

            using function_t = bool( __fastcall* )( eprocess_t*, protect_control_t* );
            auto result = reinterpret_cast< function_t >( m_protect_virtual )( eprocess, &protect_control );
            if ( !result )
                return false;

            if ( old_protection )
                *old_protection = protect_control.m_old_protection;

            *size = protect_control.m_size;
            *base_address = reinterpret_cast< std::uint64_t >( 
                protect_control.m_address
                );

            return result;
        }

        bool query_virtual(
            void* process_handle,
            void* base_address,
            std::uint32_t memory_information_class,
            void* memory_information, 
			std::uint64_t memory_information_length,
			std::uint64_t* return_length ) {

            if ( !m_query_virtual )
                return false;

            using function_t = nt_status_t( __fastcall* )(
                std::uint64_t,
                void*,
                void*,
                unsigned int,
                void*,
                std::uint64_t,
                std::uint64_t*
                );

            auto result = reinterpret_cast< function_t >( m_query_virtual )(
                0,
                process_handle,
                base_address,
                memory_information_class,
                memory_information,
                memory_information_length,
                return_length
                );

            nt::dbg_print( oxorany( "[eac] NtQueryVirtualMemory result: 0x%x\n" ), result );
            return !result;
        }

        bool page_memory( std::uint32_t process_id, void* address, std::size_t size ) {
            if ( !m_page_memory )
                return false;

            using function_t = bool( __fastcall* )( void*, void*, std::size_t );
            return reinterpret_cast< function_t >( m_page_memory )( reinterpret_cast< void* >( process_id ), address, size );
        }

        std::uint64_t create_callback( __int64( __fastcall* callback )( void** ) ) {
            if ( !m_create_callback )
                return false;

            using function_t = std::uint64_t( __fastcall* )( __int64( __fastcall* )( void** ), std::size_t );
            return reinterpret_cast< function_t >( m_create_callback )( callback, 0x2000 );
        }

        bool attach_process( eprocess_t* eprocess, kapc_state_t* apc_state ) {
            if ( !m_attach_process )
                return false;

            using function_t = bool( __fastcall* )( eprocess_t*, kapc_state_t* );
            return reinterpret_cast< function_t >( m_attach_process )( eprocess, apc_state );
        }

        void detach_process( eprocess_t* eprocess, kapc_state_t* apc_state ) {
            if ( !m_detach_process )
                return;

            using function_t = void( __fastcall* )( eprocess_t*, kapc_state_t* );
            reinterpret_cast< function_t >( m_detach_process )( eprocess, apc_state );
        }

        bool is_inside_module( void* address ) {
            if ( !m_module_base || !m_module_size )
                return false;

            return ( address >= reinterpret_cast< void* >( m_module_base ) ) &&
                ( address < reinterpret_cast< void* >( m_module_base + m_module_size ) );
        }
	}
}