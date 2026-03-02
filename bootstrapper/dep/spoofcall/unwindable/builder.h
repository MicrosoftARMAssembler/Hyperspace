#pragma once

namespace uwd_engine {
    using namespace uwd_private;

    class uwd_builder {
    public:
        uwd_builder( ) = default;

        void setup( ) {
            this->m_kernel_base = reinterpret_cast< std::uint64_t >( GetModuleHandleA( 0 ) );
            this->m_entries = reinterpret_cast < PIMAGE_RUNTIME_FUNCTION_ENTRY > (
                get_exception_directory( m_kernel_base, &m_rt_table_size )
                );

            logging::print( encrypt( "[unwindable builder] initialize: Exception table [%d]: 0x%llx\n" ), m_rt_table_size, m_entries );
        }

    private:
        union argument_holder {

            void* _ptr;
            uintptr_t		_uintptr;
            float			_float;
            double			_double;
            const char* _cstr;
            int				_int;
            unsigned int	_uint;
            bool			_bool;
            std::nullptr_t	_null;

            argument_holder( void* ptr ) : _ptr( ptr ) { }
            argument_holder( uintptr_t uint ) : _uintptr( uint ) { }
            argument_holder( float f ) : _float( f ) { }
            argument_holder( double d ) : _double( d ) { }
            argument_holder( const char* cstr ) : _cstr( cstr ) { }
            argument_holder( int i ) : _int( i ) { }
            argument_holder( unsigned int i ) : _uint( i ) { }
            argument_holder( bool b ) : _bool( b ) { }
            argument_holder( std::nullptr_t null ) : _null( null ) { }
            argument_holder( ) = default;

        };

        uwd_frames get_default_frames( ) {
            uwd_frames frames;

            frames.modules[ 0 ] = utility::get_module( L"kernel32.dll" );
            frames.exp[ 0 ] = get_export( reinterpret_cast < uint8_t* > ( frames.modules[ 0 ] ), "BaseThreadInitThunk" );

            frames.modules[ 1 ] = utility::get_module( L"ntdll.dll" );
            frames.exp[ 1 ] = get_export( reinterpret_cast < uint8_t* > ( frames.modules[ 1 ] ), "RtlUserThreadStart" );

            return frames;
        }

    public:

        template <typename T = uintptr_t>
        auto generate( T dest, uwd_frames* frames = nullptr ) {

            unwind_cfg		config;

            unwind_info* info = nullptr;
            uint32_t		stack_offset = 0;

            uwd_frames		local_frames;
            uwd_search		searcher( m_kernel_base, &config );

            // setup the fake frames info.
            if ( frames == nullptr ) {
                local_frames = get_default_frames( );
            }
            else {
                memcpy( &local_frames, frames, sizeof( uwd_frames ) );
            }

            memset( &config, 0, sizeof( unwind_cfg ) );

            // info that will be used in call.
            config.spoof_ptr = reinterpret_cast < void* > ( dest );

            config.base_thread_init_thunk_ptr = local_frames.exp[ 0 ];
            config.rtl_user_thread_start_ptr = local_frames.exp[ 1 ];

            info = reinterpret_cast < unwind_info* > ( local_frames.modules[ 0 ] + ( get_function_entry( local_frames.modules[ 0 ], reinterpret_cast < uint64_t > ( local_frames.exp[ 0 ] ) - local_frames.modules[ 0 ] ) )->UnwindData );
            get_stack_size_ignore_fpreg( local_frames.modules[ 0 ], info, &stack_offset );
            config.base_thread_init_thunk_size = stack_offset;

            info = reinterpret_cast < unwind_info* > ( local_frames.modules[ 1 ] + ( get_function_entry( local_frames.modules[ 1 ], reinterpret_cast < uint64_t > ( local_frames.exp[ 1 ] ) - local_frames.modules[ 1 ] ) )->UnwindData );
            get_stack_size_ignore_fpreg( local_frames.modules[ 1 ], info, &stack_offset );
            config.rtl_user_thread_start_size = stack_offset;

            // now, just search gadgets & prologs for this configuration.
            uint32_t table_last_index = m_rt_table_size / 12, table_save_index = 0;
            uint32_t is_skip_prologue = 0, is_skip_pop_rsp = 0, is_skip_jmp = 0, is_skip_stack_pivot = 1;
            uint32_t stack_size = 0;
            uint64_t table_target_offset = 0;

            searcher.find_prolog( m_entries, table_last_index, &stack_size, &table_save_index, &is_skip_prologue, &table_target_offset );
            searcher.find_push_rbp( m_entries, table_last_index, &stack_size, &table_save_index, &is_skip_pop_rsp, &table_target_offset );
            searcher.find_gadget( m_entries, table_last_index, &stack_size, &table_save_index, &is_skip_jmp, uwd_search::gadget_type::gdt_add_rsp );
            searcher.find_gadget( m_entries, table_last_index, &stack_size, &table_save_index, &is_skip_stack_pivot, uwd_search::gadget_type::gdt_jmp_rbx );

            return config;

        }

        template <typename T = uintptr_t, typename... Args>
        __forceinline T call( unwind_cfg* config, Args... args ) {

            // was built according to https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170.

            constexpr auto	args_cnt = sizeof... ( Args );

            if constexpr ( args_cnt > 0 ) {
                argument_holder args_arr[ args_cnt ] = { argument_holder( args )... };
                bool			float_arr[ args_cnt ] = { std::is_floating_point <Args>::value... };

                memcpy( config->args, &args_arr, args_cnt * 8 );
                memcpy( config->is_floating, &float_arr, sizeof( config->is_floating ) );
            }
            // prepare to jmp.
            config->args_cnt = args_cnt;
            config->ret_addr = _AddressOfReturnAddress( );

            // do nothing, just a direct call to needed place.
            if constexpr ( std::is_same_v <T, void> ) {

                spoof_call( config );

            }
            else {

                // floating points.
                if constexpr ( std::is_floating_point_v <T> || std::is_same_v<T, float> ) {
                    spoof_call( config );
                    return sizeof( T ) == sizeof( float ) ? get_float_register( ) : get_double_register( );
                }

                // and whatever else.
                return ( T )( spoof_call( config ) );

            }

        }

    private:
        uint64_t m_kernel_base;
        uint32_t m_rt_table_size;
        PIMAGE_RUNTIME_FUNCTION_ENTRY m_entries;
    };
}