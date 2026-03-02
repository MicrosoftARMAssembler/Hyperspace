namespace driver {
#pragma pack(push, 1)
    struct logical_memory_info_t {
        std::uint64_t physical_address;  // Offset 0
        std::uint64_t virtual_address;   // Offset 8
        std::uint32_t size;              // Offset 16
        std::uint32_t reserved;          // Offset 20
    };
#pragma pack(pop)

#pragma section(".text")
    __declspec( allocate( ".text" ) )
        const std::uint8_t syscall_stub[ ]{
        0x4C, 0x8B, 0x11, //mov    r10, QWORD PTR [rcx]
        0x8B, 0x41, 0x08, //mov    eax, DWORD PTR [rcx+0x8]
        0x0F, 0x05,       //syscall
        0xC3              //ret
    };

    template <
        typename ret_type = void,
        typename first = void*,
        typename... args
    >
    
    ret_type syscall(
        DWORD index,
        first arg = first{},
        args... NextArgs
    ) {
        struct data_struct {
            first arg1;
            DWORD syscall_id;
        } data{ arg, index };

        using call_stub = ret_type( __fastcall* )( data_struct*, args... );
        return ( ( call_stub )&syscall_stub[ 0 ] )( &data, NextArgs... );
    }

    class c_driver {
        NTSTATUS send_cmd( uint32_t ioctl_code, void* input_buffer, uint32_t input_size,
            void* output_buffer, uint32_t output_size ) const {
            if ( m_driver_handle == INVALID_HANDLE_VALUE ) {
                logging::print( oxorany( "[c_driver] send_cmd: Could not open driver handle." ) );
                return false;
            }

            IO_STATUS_BLOCK block;
            auto result = driver::syscall<NTSTATUS>( 7,
                this->m_driver_handle,
                nullptr,
                nullptr,
                nullptr,
                &block,
                ioctl_code,
                input_buffer,
                input_size,
                output_buffer,
                output_size
            );

            return result;
        }

    public:
        bool initialize( ) {
            this->m_driver_handle = CreateFileW(
                L"\\\\.\\ASMMAP64",
                0xC0000000, 3u, 0i64, 3u, 0, 0i64
            );

            if ( !m_driver_handle || m_driver_handle == INVALID_HANDLE_VALUE ) {
                logging::print( oxorany( "[c_driver] initialize: Could not open driver handle." ) );
                return false;
            }

            logging::print( oxorany( "[c_driver] initialize: Driver handle: 0x%llx\n" ), m_driver_handle );
            return true;
        }

        void unload( ) {
            if ( m_driver_handle != INVALID_HANDLE_VALUE ) {
				CloseHandle( m_driver_handle );
				m_driver_handle = INVALID_HANDLE_VALUE;
            }
        }

		HANDLE get_handle( ) const {
			return m_driver_handle;
		}

        std::pair<std::uint64_t, std::uint64_t> allocate_logical_memory( uint32_t size ) {
            auto ioctl_buffer = std::make_unique<logical_memory_info_t>( );
            ioctl_buffer->size = size;

            auto result = send_cmd( 0x9C402588,
                ioctl_buffer.get( ),
                sizeof( logical_memory_info_t ),
                ioctl_buffer.get( ),
                sizeof( logical_memory_info_t )
            );

            if ( result ) {
                logging::print( oxorany( "[c_driver] allocate_logical_memory: Could not allocate physical memory: %i" ), result );
                return { 0, 0 };
            }

            return { ioctl_buffer->virtual_address, ioctl_buffer->physical_address };
        }

        std::uint64_t map_physical_memory( uint64_t physical_address, uint32_t size ) {
            auto ioctl_buffer = std::make_unique<logical_memory_info_t>( );

            ioctl_buffer->physical_address = physical_address;
            ioctl_buffer->size = size;

            auto result = send_cmd( 0x9C402580,
                ioctl_buffer.get( ),
                sizeof( logical_memory_info_t ),
                ioctl_buffer.get( ),
                sizeof( logical_memory_info_t )
            );

            if ( result ) {
                logging::print( oxorany( "[c_driver] map_physical_memory: Could not map physical memory: %i" ), result );
                return 0;
            }

            return ioctl_buffer->virtual_address;
        }

        void unmap_physical_memory( void* mapped_address, uint32_t size ) {
            auto ioctl_buffer = std::make_unique<logical_memory_info_t>( );

            ioctl_buffer->virtual_address = reinterpret_cast< std::uint64_t >( mapped_address );
            ioctl_buffer->size = size;

            auto result = send_cmd( 0x9C402584,
                ioctl_buffer.get( ),
                sizeof( logical_memory_info_t ),
                ioctl_buffer.get( ),
                sizeof( logical_memory_info_t )
            );

            if ( result ) {
                logging::print( oxorany( "[c_driver] unmap_physical_memory: Could not unmap memory: %i" ), result );
            }
        }

        bool read_physical_memory( uint64_t physical_address, void* buffer, uint32_t size ) {
            auto mapped_va = ( void* )map_physical_memory( physical_address, size );
            if ( !mapped_va )
                return false;

            crt::memcpy( buffer, mapped_va, size );

            unmap_physical_memory( mapped_va, size );
            return true;
        }

        bool write_physical_memory( uint64_t physical_address, void* buffer, uint32_t size ) {
            auto mapped_va = ( void* )map_physical_memory( physical_address, size );
            if ( !mapped_va )
                return false;

            crt::memcpy( mapped_va, buffer, size );

            unmap_physical_memory( mapped_va, size );
            return true;
        }

        template <typename ret_t = std::uint64_t, typename addr_t>
        ret_t read_physical( addr_t address ) {
            ret_t buffer{ };
            if ( !read_physical_memory( address, &buffer, sizeof( addr_t ) ) )
                return ret_t{};
            return buffer;
        }

        template <typename addr_t = std::uint64_t, typename value_t>
        bool write_physical( addr_t address, value_t value ) {
            return write_physical_memory( address, &value, sizeof( value_t ) );
        }

    private:
        HANDLE m_driver_handle = INVALID_HANDLE_VALUE;
    };
}