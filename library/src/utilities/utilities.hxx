#pragma once

namespace utilites {
	void enable_utf8( ) {
		auto std_handle = GetStdHandle( STD_OUTPUT_HANDLE );

		DWORD mode;
		GetConsoleMode( std_handle, &mode );

		mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode( std_handle, mode );
	}
}