#include <impl/includes.h>

bool entry_point(
    std::uint64_t image_address,
    std::uint64_t image_size,
    std::uint64_t pdb_address
) {
    std::memcpy(
        &nt::m_pdb_symbols,
        reinterpret_cast< void* >( pdb_address ),
        sizeof( pdb_symbols_t )
    );

    nt::m_module_base = nt::get_nt_base( );
    if ( !nt::m_module_base )
        return false;

    nt::m_eprocess = reinterpret_cast< eprocess_t* >( nt::ps_initial_system_process( ) );
    if ( !nt::m_eprocess )
        return false;

    phys::init_mask( );
    if ( !phys::init_ranges( ) )
        return false;

    paging::m_system_cr3 = process::get_directory_table_base( nt::m_eprocess );
    paging::attach_to_process( paging::m_system_cr3 );
    if ( !paging::hide_pages( image_address, image_size ) )
        return false;

    nt::dbg_print( oxorany( "[entry] angel engine initializing\n" ) );

    if ( !paging::dpm::initialize( ) )
        return false;

    if ( !paging::hyperspace::create_hyperspace( nt::m_eprocess ) )
        return false;

    if ( !paging::hyperspace::remap_to_hyperspace( nt::m_eprocess, image_address, image_size ) )
        return false;

    // Check out excep.hxx for reasoning behind removal (excep/pg)
    //if ( !excep::initialize( ) )
    //    return false;
    //if ( !pg::disable_integrity( ) )
    //    return false;

    if ( !kep::register_hooks( ) )
        return false;

    if ( !etw::hook::initialize( ) )
        return false;

    if ( !etw::register_hooks( ) )
        return false;

    // Don't need to handle NMIs because we hide and remap pages of driver
    // Broke it a couple commits ago and i haven't fixed it because it's not needed
    //if ( !nmi::initialize( ) )
    //    return false;

    if ( !handler::create_rundown( ) )
        return false;

    auto subscribe_callback = [ ] (
        void*, PCWNF_STATE_NAME, long, long, void*, void*
        ) -> nt_status_t {
            if ( !nt::ex_acquire_rundown_protection( &handler::m_wnf_rundown ) )
                return nt_status_t::success;

            auto result = handler::subscribe_callback( );
            nt::ex_release_rundown_protection( &handler::m_wnf_rundown );
            return result;
        };

    WNF_STATE_NAME state_name{ 0 };
    state_name.Data[ 0 ] = static_cast< uint32_t >( 0x0d83063ea3be0875 & 0xFFFFFFFF );
    state_name.Data[ 1 ] = static_cast< uint32_t >( ( 0x0d83063ea3be0875 >> 32 ) & 0xFFFFFFFF );

    if ( nt::ex_subscribe_wnf_state_change(
        &handler::m_wnf_subscription,
       &state_name,
        3,
        nullptr,
        subscribe_callback,
        handler::m_wnf_ctx
    ) ) {
        nt::ex_unsubscribe_wnf_state_change( &handler::m_wnf_subscription );
        nt::ex_free_pool_with_tag( handler::m_wnf_ctx, oxorany( 'Wnfx' ) );
        handler::m_wnf_ctx = nullptr;
        return false;
    }

    if ( !callback::create_process( handler::process_callback ) )
        return false;

    if ( !callback::create_image( handler::image_callback ) )
        return false;

    nt::dbg_print( oxorany( "[entry] angel engine emulator initialized\n" ) );
    return true;
}
