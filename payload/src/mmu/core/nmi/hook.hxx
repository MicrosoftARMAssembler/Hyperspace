#pragma once

namespace nmi {
	namespace handler {
		struct nmi_core_info_t {
			std::uint64_t m_prev_rip;
			std::uint64_t m_prev_rsp;
			kthread_t* m_prev_current_thread;
			kthread_t* m_prev_next_thread;
			bool m_prev_running;
		};

		constexpr auto nmi_core_info_tag = 'IMNC';
		nmi_core_info_t* m_core_infos{ nullptr };
		knmi_handler_callback_t* m_list_head{ nullptr };
		knmi_handler_callback_t* m_callback_parent{ nullptr };
		knmi_handler_callback_t m_restore_callback{ };
		std::uint64_t m_po_idle{ 0 };
		void ( *m_hal_preprocess_nmi_original )( std::uint32_t ) = nullptr;

		bool restore_trap_frame( void* context, bool handled ) {
			( void )context;

			auto kpcr = nt::ke_get_pcr( );
			if ( !kpcr )
				return handled;

			auto kprcb = kpcr->m_current_prcb;
			if ( !kprcb )
				return handled;

			auto tss = *reinterpret_cast< ktss64_t** >(
				reinterpret_cast< std::uint64_t >( kpcr ) + 0x08 );
			if ( !tss || !nt::mm_is_address_valid( tss ) ) {
				return handled;
			}

			auto machine_frame = reinterpret_cast< machine_frame_t* >(
				tss->m_ist[ 3 ] - sizeof( machine_frame_t ) );
			if ( !nt::mm_is_address_valid( machine_frame ) ) {
				return handled;
			}

			auto processor_index = nt::ke_get_current_processor_number( );

			machine_frame->Rip = m_core_infos[ processor_index ].m_prev_rip;
			machine_frame->Rsp = m_core_infos[ processor_index ].m_prev_rsp;
			kprcb->m_current_thread = m_core_infos[ processor_index ].m_prev_current_thread;
			kprcb->m_next_thread = m_core_infos[ processor_index ].m_prev_next_thread;
			kprcb->m_idle_thread->m_running = m_core_infos[ processor_index ].m_prev_running;

			if ( m_callback_parent )
				m_callback_parent->Next = nullptr;

			return handled;
		}

		void nmi_hook( std::uint32_t arg1 ) {
			m_hal_preprocess_nmi_original( arg1 );

			nt::dbg_print( oxorany( "[nmi] NMI hit! arg1=%d\n" ), arg1 );

			if ( arg1 == 1 )
				return;

			if ( !m_list_head )
				return;

			m_callback_parent = nullptr;
			auto current_callback = m_list_head;
			while ( current_callback ) {
				m_callback_parent = current_callback;
				current_callback = current_callback->Next;
			}
			m_callback_parent->Next = &m_restore_callback;

			auto kpcr = KeGetPcr( );
			auto kprcb = ( kprcb_t* )kpcr->CurrentPrcb;
			auto tss = ( ktss64_t* )kpcr->TssBase;
			if ( !nt::mm_is_address_valid( tss ) ) {
				nt::dbg_print( oxorany( "[nmi] Could not get TssBase\n" ) );
				return;
			}

			auto machine_frame = ( machine_frame_t* )( tss->m_ist[ 3 ] - sizeof( machine_frame_t ) );
			if ( !nt::mm_is_address_valid( machine_frame ) ) {
				nt::dbg_print( oxorany( "[nmi] Could not get MachineFrame\n" ) );
				return;
			}

			auto processor_index = nt::ke_get_current_processor_number( );

			nt::dbg_print( oxorany( "[nmi] CPU %d | RIP: 0x%llX | RSP: 0x%llX\n" ),
				processor_index, machine_frame->Rip, machine_frame->Rsp );

			m_core_infos[ processor_index ].m_prev_rip = machine_frame->Rip;
			m_core_infos[ processor_index ].m_prev_rsp = machine_frame->Rsp;
			m_core_infos[ processor_index ].m_prev_current_thread = kprcb->m_current_thread;
			m_core_infos[ processor_index ].m_prev_next_thread = kprcb->m_next_thread;
			m_core_infos[ processor_index ].m_prev_running = kprcb->m_idle_thread->m_running;

			machine_frame->Rip = m_po_idle;
			machine_frame->Rsp = reinterpret_cast< std::uint64_t >(
				reinterpret_cast< std::uint8_t* >( kprcb->m_idle_thread->m_initial_stack ) - 0x38 );
			kprcb->m_current_thread = kprcb->m_idle_thread;
			kprcb->m_next_thread = nullptr;
			kprcb->m_idle_thread->m_running = true;

			nt::dbg_print( oxorany( "[nmi] Spoofed to PoIdle (0x%llX)\n" ), m_po_idle );
		}
	}
}
