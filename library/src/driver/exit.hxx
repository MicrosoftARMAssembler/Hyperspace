#pragma once

namespace hyperspace {
	void unload( );

	namespace exit {
		bool control_handler( std::uint32_t signal ) {
			logging::print( oxorany( "[hyperspace] exit::control_handler: Caught exit signal: %d" ), signal );
			unload( );
			return true;
		}

		long crash_handler( EXCEPTION_POINTERS* exception_pointers ) {
            const auto* context = exception_pointers->ContextRecord;
            char message[ 1024 ];
            sprintf( message,
                oxorany( "Oops! Something went wrong!\n\n"
                    "The service encountered an unexpected error and needs to close.\n\n"
                    "Quick fixes to try:\n"
                    "  • Restart the service\n"
                    "  • Rollback recent updates to the service\n"
                    "  • Check if your antivirus is interfering\n\n"
                    "Still having trouble? We're here to help!\n"
                    "Contact support through the tickets section.\n\n"
                    "Crash Details:\n"
                    "Build: %s %s\n"
                    "Error: 0x%08X at %p\n"
                    "Registers: RSP=%016llX RDI=%016llX\n"
                    "           RSI=%016llX RBX=%016llX\n"
                    "           RDX=%016llX RCX=%016llX\n"
                    "           RAX=%016llX RBP=%016llX" ),
                __DATE__, __TIME__,
                exception_pointers->ExceptionRecord->ExceptionCode,
                exception_pointers->ExceptionRecord->ExceptionAddress,
                context->Rsp, context->Rdi,
                context->Rsi, context->Rbx,
                context->Rdx, context->Rcx,
                context->Rax, context->Rbp
            );

            printf( oxorany( "\n" ) );
            logging::print( oxorany( "[hyperspace] exit::crash_handler: Caught exception" ) );
            MessageBoxA( 0, message, "exit::crash_handler - Unexpected Error", MB_ICONERROR | MB_OK );
			unload( );
			return true;
		}

		class c_exit {
		public:
			c_exit( ) {
				utilites::enable_utf8( );

				SetConsoleCtrlHandler(
					reinterpret_cast< PHANDLER_ROUTINE >( control_handler ),
					true
				);
				SetUnhandledExceptionFilter(
					reinterpret_cast< LPTOP_LEVEL_EXCEPTION_FILTER >( crash_handler )
				);

				logging::print( oxorany( "[hyperspace] exit::initialize: Created handlers" ) );
			}
		};
	}
}