#pragma once 

namespace mmu {
    enum protection_style {
        no_access = 0,
        readwrite = 4,
        readwrite_execute = 7,
        execute_read = 21,
        execute_readwrite = 24,
        readwrite_nocache = 12,
        execute_writecopy = 0x1a
    };

    std::uint64_t alloc_page( std::size_t size ) {
        auto buffer = nt::mm_allocate_independent_pages( size );
        if ( !buffer )
            return {};

        memset( buffer, 0, size );
        return reinterpret_cast< std::uint64_t >( buffer );
    }

    void free_page( std::uint64_t alloc_base, std::size_t size ) {
        nt::mm_free_independent_pages( alloc_base, size );
    }

    bool is_user_address( std::uint64_t address, std::size_t size, std::uint32_t alignment ) {
        if ( size == 0 )
            return false;

        const auto current = address;
        if ( ( current & ( alignment - 1 ) ) != 0 ) {
            return false;
        }

        const auto last = current + size - 1;
        if ( ( last < current ) || ( last >= nt::mm_user_probe_address( ) ) ) {
            return false;
        }

        return true;
    }

    bool is_kernel_address( std::uint64_t address, std::size_t size, std::uint32_t alignment ) {
        if ( size == 0 )
            return false;

        const auto current = address;
        if ( ( current & ( alignment - 1 ) ) != 0 ) {
            return false;
        }

        const auto last = current + size - 1;
        if ( ( last < current ) || ( last >= nt::mm_system_range_start( ) ) ) {
            return false;
        }

        return true;
    }

    std::uint64_t find_unused_space( void* base, std::uint32_t section_size, std::size_t space_size ) {
        auto* data_section = static_cast< uint8_t* >( base );
        for ( auto idx = 0; idx <= section_size - space_size; ) {
            bool found_space = true;
            for ( auto j = 0; j < space_size; ++j ) {
                if ( data_section[ idx + j ] != 0x00 ) {
                    found_space = false;
                    idx += j + 1;
                    break;
                }
            }

            if ( found_space ) {
                return reinterpret_cast< std::uint64_t >( &data_section[ idx ] );
            }
        }

        return 0;
    }

    bool is_exception_on_stack( ) {
        auto exception_offset = nt::get_exception_offset( );
        if ( !exception_offset )
            return false;

        if ( __readgsbyte( exception_offset - 2 ) )
            return false;

        return __readgsqword( exception_offset ) - reinterpret_cast< std::uint64_t >( address_of_return ) < 0x6000ui64;
    }

    template<typename callback_t>
    bool traverse_stack( callback_t callback, void* stack_frame = nullptr, bool reverse = false ) {
        auto stack_start = stack_frame ? reinterpret_cast< void** >( stack_frame ) : max_stack_frame;
        auto stack_end = address_of_return;

        bool exception_on_stack = is_exception_on_stack( );
        if ( exception_on_stack ) {
            stack_start = reinterpret_cast< void** >( __readgsqword( nt::get_exception_offset( ) ) );
        }
        else {
            if ( stack_start - stack_end > 0x6000 || stack_end > stack_start ) {
                return false;
            }
        }

        if ( reverse ) {
            for ( auto stack_current = stack_end; stack_current < stack_start; ++stack_current ) {
                if ( callback( stack_current ) )
                    return true;
            }
        }
        else {
            for ( auto stack_current = stack_start; stack_current > stack_end; --stack_current ) {
                if ( callback( stack_current ) )
                    return true;
            }
        }
        return false;
    }

    template<typename callback_t>
    bool traverse_vad_node( eprocess_t* eprocess, mmvad_short_t* vad_node, callback_t func ) {
        if ( !vad_node ) {
            return true;
        }

        if ( !func( vad_node ) ) {
            return false;
        }

        if ( !traverse_vad_node( eprocess, reinterpret_cast< mmvad_short_t* >( vad_node->m_vad_node.m_left ), func ) ) {
            return false;
        }

        if ( !traverse_vad_node( eprocess, reinterpret_cast< mmvad_short_t* >( vad_node->m_vad_node.m_right ), func ) ) {
            return false;
        }

        return true;
    }

    template<typename callback_t>
    bool traverse_vad_node( mmvad_short_t* vad_node, callback_t func ) {
        if ( !vad_node ) {
            return true;
        }

        if ( !func( vad_node ) ) {
            return false;
        }

        if ( !traverse_vad_node( reinterpret_cast< mmvad_short_t* >( vad_node->m_vad_node.m_left ), func ) ) {
            return false;
        }

        if ( !traverse_vad_node( reinterpret_cast< mmvad_short_t* >( vad_node->m_vad_node.m_right ), func ) ) {
            return false;
        }

        return true;
    }

    nt_status_t sleep( std::int32_t milliseconds ) {
        std::int64_t interval = milliseconds * -10000i64;
        return nt::ke_delay_execution_thread( 0, false, reinterpret_cast< ularge_integer_t* >( &interval ) );
    }
}