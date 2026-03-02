#include <impl/includes.h>

template<class T> T __ROL__( T value, int count ) {
	const auto nbits = sizeof( T ) * 8;

	if ( count > 0 ) {
		count %= nbits;
		T high = value >> ( nbits - count );
		if ( T( -1 ) < 0 ) // signed value
			high &= ~( ( T( -1 ) << count ) );
		value <<= count;
		value |= high;
	}
	else {
		count = -count % nbits;
		T low = value << ( nbits - count );
		value >>= count;
		value |= low;
	}
	return value;
}

std::uint64_t __ROL8__( std::uint64_t value, int count ) { return __ROL__( ( std::uint64_t )value, count ); }

int main( ) {
	SetConsoleTitleA( oxorany( "hyperspace" ) );
	SetUnhandledExceptionFilter( exception::exception_filter );

	auto std_handle = GetStdHandle( STD_OUTPUT_HANDLE );
	DWORD mode;
	GetConsoleMode( std_handle, &mode );
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode( std_handle, mode );

	if ( !hyperspace::initialize( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not initialize hyperspace." ) );
		return std::getchar( );
	}

	if ( !hyperspace::is_active( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not ping driver." ) );
		return std::getchar( );
	}

	logging::print( oxorany( "[hyperspace] main: Successfully pinged driver" ) );
	logging::print( oxorany( "[hyperspace] main: Press enter to continue." ) );
	std::getchar( );

	auto result = MessageBoxA(
		0,
		oxorany( "Warning if you unhook the driver you will need to remap the dependency." ),
		oxorany( "Would you like to unload the driver?" ),
		MB_YESNO | MB_ICONQUESTION | MB_SERVICE_NOTIFICATION
	);

	if ( result == IDYES ) {
		hyperspace::unhook( );
		return std::getchar( );
	}

	std::getchar( );

	if ( !hyperspace::target::attach_process( oxorany( L"notepad.exe" ) ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not attach to process." ) );
		hyperspace::unload( );
		return std::getchar( );
	}
	std::getchar( );

	if ( !hyperspace::target::benchmark_read( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not benchmark reads per second." ) );
		hyperspace::target::detach_process( );
		hyperspace::unload( );
		return std::getchar( );
	}
	std::getchar( );


	auto base_address = hyperspace::paging::remap::allocate_virtual( 0x1000 );
	logging::print( oxorany( "[hyperspace] main: Base address: 0x%llx" ), base_address );

	std::uint32_t magic_code = 0x67;
	if ( !hyperspace::write_virt( base_address, &magic_code, 1 ) )
		logging::print( oxorany( "[hyperspace] main: Could not write physical." ) );

	//auto g_world = hyperspace::read_virt( hyperspace::target::m_base_address + 0x172C8CB8 );
	//g_world = __ROL8__( g_world, 55 ) - 1735245483LL;
	//if ( !g_world ) {
	//	logging::print( oxorany( "[hyperspace] main: Could not get GWorld." ) );
	//	hyperspace::target::detach_process( );
	//	hyperspace::unload( );
	//	return std::getchar( );
	//}

	//logging::print( oxorany( "[hyperspace] main: GWorld: 0x%llx" ), g_world );

	//auto game_instance = hyperspace::read_virt( g_world + 0x248 );
	//if ( !game_instance ) {
	//	logging::print( oxorany( "[hyperspace] main: Could not get Game Instance." ) );
	//	hyperspace::target::detach_process( );
	//	hyperspace::unload( );
	//	return std::getchar( );
	//}

	//logging::print( oxorany( "[hyperspace] main: OwningGameInstance: 0x%llx" ), game_instance );

	//if ( !hyperspace::paging::fire_nmi( ) ) {
	//	logging::print( oxorany( "[hyperspace] main: Could not execute NMI." ) );
	//	hyperspace::target::detach_process( );
	//	hyperspace::unload( );
	//	return std::getchar( );
	//}

	result = MessageBoxA(
		0,
		oxorany( "Warning if you unhook the driver you will need to remap the dependency." ),
		oxorany( "Would you like to unload the driver?" ),
		MB_YESNO | MB_ICONQUESTION | MB_SERVICE_NOTIFICATION
	);
	if ( result == IDYES ) {
		hyperspace::target::detach_process( );
		hyperspace::unhook( );
		return std::getchar( );
	}

	hyperspace::target::detach_process( );
	hyperspace::unload( );
	return std::getchar( );
}