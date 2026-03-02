#pragma once

namespace lbr {
    paging::pth::hook_state_t m_hook_state;

    struct context_t {
        eprocess_t* m_target_process;
        std::uint64_t m_target_cr3;
    };

    namespace hook {
        kevent_t m_event;
        bool ( *m_callback )( void* ) = nullptr;
        void* m_context = nullptr;

        std::uint8_t( *hal_clear_last_branch_record_stack_org )( ) = nullptr;
        std::uint8_t __fastcall hal_clear_last_branch_record_stack_hk( ) {
            if ( !m_callback || !m_context )
                return hal_clear_last_branch_record_stack_org( );

            if ( m_callback( m_context ) )
                nt::ke_set_event( &m_event, 0, false );

            return hal_clear_last_branch_record_stack_org( );
        }

        bool install( ) {
            auto hal_dispatch = nt::get_hal_private_dispatch( );
            if ( !hal_dispatch )
                return false;

            auto hal_clear_last_branch_record_stack = reinterpret_cast< std::uint64_t >(
                hal_dispatch->m_hal_clear_last_branch_record_stack
                );
            if ( !hal_clear_last_branch_record_stack )
                return false;

            if ( !paging::pth::create(
                hal_clear_last_branch_record_stack,
                hal_clear_last_branch_record_stack_hk,
                &hal_clear_last_branch_record_stack_org,
                &m_hook_state
            ) ) return false;

            if ( auto* cpu_tracing_flags = nt::get_ki_cpu_tracing_flags( ) )
                *cpu_tracing_flags = 2;

            return true;
        }

        bool uninstall( ) {
            if ( hal_clear_last_branch_record_stack_org ) {
                auto hal_dispatch = nt::get_hal_private_dispatch( );
                if ( !hal_dispatch )
                    return false;

                auto target_fn = reinterpret_cast< std::uint64_t >(
                    hal_dispatch->m_hal_clear_last_branch_record_stack
                    );

                if ( !target_fn )
                    return false;

                if ( !paging::pth::remove( target_fn, m_hook_state ) )
                    return false;
            }

            if ( auto* flags = nt::get_ki_cpu_tracing_flags( ) )
                *flags = 0;

            return true;
        }
    }

    bool get_process_cr3( eprocess_t* target_process, std::uint64_t* out_cr3 ) {
        auto* ctx = reinterpret_cast< context_t* >(
            mmu::alloc_page( sizeof( context_t ) )
            );

        if ( !ctx )
            return false;

        ctx->m_target_process = target_process;
        nt::ke_initialize_event( &hook::m_event, 1, false );

        hook::m_context = ctx;
        hook::m_callback = [ ] ( void* data ) -> bool {
            if ( !data )
                return false;

            auto* ctx = reinterpret_cast< context_t* >( data );
            if ( ctx->m_target_process == nt::io_get_current_process( ) ) {
                ctx->m_target_cr3 = current_cr3;
                return true;
            }

            return false;
            };

        if ( !hook::install( ) )
            return false;

        auto result = nt::ke_wait_for_single_object(
            &hook::m_event, 0, 0, false, 20000
        );

        hook::uninstall( );
        hook::m_callback = nullptr;

        if ( out_cr3 )
            *out_cr3 = ctx->m_target_cr3;

        mmu::free_page(
            reinterpret_cast< std::uint64_t >( ctx ),
            sizeof( context_t )
        );

        nt::ke_clear_event( &hook::m_event );
        return !result;
    }
}