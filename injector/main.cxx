#include <impl/includes.h>

int main( int argc, char** argv ) {
	SetConsoleTitleA( oxorany( "hyperspace-injector" ) );
	SetUnhandledExceptionFilter( exception::exception_filter );

	auto std_handle = GetStdHandle( STD_OUTPUT_HANDLE );
	DWORD mode;
	GetConsoleMode( std_handle, &mode );
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode( std_handle, mode );

	if ( argc < 2 ) {
		logging::print( oxorany( "[hyperspace] main: No DLL path provided." ) );
		logging::print( oxorany( "[hyperspace] main: Usage: %s <path_to_dll>" ), argv[ 0 ] );
		return std::getchar( );
	}

	if ( !hyperspace::initialize( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not initialize hyperspace." ) );
		return std::getchar( );
	}

	if ( !hyperspace::is_active( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not ping driver." ) );
		hyperspace::unload( );
		return std::getchar( );
	}

	if ( !hyperspace::target::attach_process( oxorany( target_process ) ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not attach to process." ) );
		hyperspace::unload( );
		return std::getchar( );
	}

	std::getchar( );

	auto dependency = std::make_shared< dependency::c_dependency >( argv[ 1 ] );
	if ( !dependency->is_dll( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not load dependency." ) );
		hyperspace::target::detach_process( );
		hyperspace::unload( );
		return std::getchar( );
	}

	auto map = std::make_shared< map::c_map >( dependency );
	if ( !map->inject( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not inject dependency." ) );
		hyperspace::target::detach_process( );
		hyperspace::unload( );
		return std::getchar( );
	}

	if ( !map->execute( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not execute dependency." ) );
		hyperspace::target::detach_process( );
		hyperspace::unload( );
		return std::getchar( );
	}

	hyperspace::target::detach_process( );
	hyperspace::unload( );
	return std::getchar( );
}