#include <impl/includes.h>

int main( int argc, char** argv ) {
	SetConsoleTitleA( oxorany( "boostrapper" ) );
	SetUnhandledExceptionFilter( exception::exception_filter );

	auto std_handle = GetStdHandle( STD_OUTPUT_HANDLE );
	DWORD mode;
	GetConsoleMode( std_handle, &mode );
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode( std_handle, mode );

	if ( !utility::adjust_privilege( 20 ) )
		return std::getchar( );

	if ( argc < 2 ) {
		logging::print( oxorany( "[main] initialization: Missing parameters" ) );
		return std::getchar( );
	}

	if ( !g_pdb->load( ) ) {
		logging::print( oxorany( "[main] initialization: Could not load PDB" ) );
		return std::getchar( );
	}

	if ( !g_esclation->setup( ) ) {
		logging::print( oxorany( "[main] initialization: Could not setup vulernable driver" ) );
		g_esclation->unload( );
		return std::getchar( );
	}

	g_ntoskrnl = utility::get_kernel_module( oxorany( "ntoskrnl.exe" ) );
	if ( !g_ntoskrnl ) {
		logging::print( oxorany( "[main] initialization: Could not find ntoskrnl.exe" ) );
		g_esclation->unload( );
		return std::getchar( );
	}

	g_paging->init_mask( );
	auto system_cr3 = 0x1ad000;
	if ( !system_cr3 ) {
		logging::print( oxorany( "[main] initialization: Could not get system CR3" ) );
		g_esclation->unload( );
		return std::getchar( );
	}

	g_paging->swap_context( system_cr3 );
	if ( !g_hook->setup( ) ) {
		logging::print( oxorany( "[main] initialization: Could not setup kernel execution" ) );
		g_esclation->unload( );
		return std::getchar( );
	}

	if ( !g_dpm->create( ) ) {
		logging::print( oxorany( "[main] initialization: Could not create dpm" ) );
		g_esclation->unload( );
		g_hook->unhook( );
		return std::getchar( );
	}

	g_paging->m_read_physical = [ ]( std::uint64_t address, void* buffer, std::size_t size ) -> bool {
			return g_dpm->read_physical_memory( address, buffer, size );
		};

	g_paging->m_write_physical = [ ]( std::uint64_t address, void* buffer, std::size_t size ) -> bool {
			return g_dpm->write_physical_memory( address, buffer, size );
		};

	g_esclation->unload( );
	g_driver->unload( );

	auto dependency = std::make_shared< map::c_dependency >( argv[ 1 ] );
	if ( !dependency->load( ) ) {
		logging::print( oxorany( "[main] initialization: Could not load dependency" ) );
		g_hook->unhook( );
		return std::getchar( );
	}

	auto manual_map = std::make_unique< map::c_map >( dependency );
	if ( !manual_map->create( ) ) {
		logging::print( oxorany( "[main] initialization: Could not create manual mapping" ) );
		g_hook->unhook( );
		return std::getchar( );
	}

	manual_map->execute( );
	g_hook->unhook( );
	return std::getchar( );
}
