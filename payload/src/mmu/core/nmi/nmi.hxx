#pragma once

namespace nmi {
	nt_status_t install_hook( ) {
		auto hal_private_dispatch = nt::get_hal_private_dispatch( );
		if ( !hal_private_dispatch )
			return nt_status_t::unsuccessful;

		auto hal_preprocess_nmi = reinterpret_cast
			< std::uint64_t >( hal_private_dispatch->m_hal_preprocess_nmi );
		if ( !hal_preprocess_nmi )
			return nt_status_t::unsuccessful;

		if ( !paging::pth::create(
			hal_preprocess_nmi,
			handler::nmi_hook,
			&handler::m_hal_preprocess_nmi_original
		) ) return nt_status_t::unsuccessful;

		return nt_status_t::success;
	}

	void unhook( ) {
		if ( !handler::m_hal_preprocess_nmi_original )
			return;

		auto hal_private_dispatch = nt::get_hal_private_dispatch( );
		if ( !hal_private_dispatch )
			return;

		hal_private_dispatch->m_hal_preprocess_nmi = handler::m_hal_preprocess_nmi_original;
	}

	knmi_handler_callback_t* scan_list_head( ) {
		return reinterpret_cast< knmi_handler_callback_t* >( nt::m_pdb_symbols.m_ki_nmi_callback_list_head );
	}

	nt_status_t scan_po_idle( ) {
		handler::m_po_idle = nt::m_pdb_symbols.m_po_idle;
		return handler::m_po_idle ? nt_status_t::success : nt_status_t::unsuccessful;
	}

	bool initialize( ) {
		auto status = nt_status_t::success;

		do {
			handler::m_list_head = scan_list_head( );
			if ( !handler::m_list_head ) {
				status = nt_status_t::unsuccessful;
				break;
			}

			status = scan_po_idle( );
			if ( !NT_SUCCESS( status ) )
				break;

			auto num_processors = nt::ke_query_active_processor_count_ex( );
			handler::m_core_infos = static_cast< handler::nmi_core_info_t* >(
				nt::ex_allocate_pool_with_tag(
					POOL_FLAG_NON_PAGED,
					num_processors * sizeof( handler::nmi_core_info_t ),
					handler::nmi_core_info_tag
				)
				);

			if ( !handler::m_core_infos ) {
				status = nt_status_t::insufficient_resources;
				break;
			}

			handler::m_restore_callback.Callback = handler::restore_trap_frame;
			handler::m_restore_callback.Context = nullptr;
			handler::m_restore_callback.Next = nullptr;

			status = install_hook( );

		} while ( false );

		if ( !NT_SUCCESS( status ) )
			return false;

		return true;
	}

	void cleanup( ) {
		if ( handler::m_core_infos ) {
			nt::ex_free_pool_with_tag( handler::m_core_infos, handler::nmi_core_info_tag );
			handler::m_core_infos = nullptr;
		}
		unhook( );
	}
}
