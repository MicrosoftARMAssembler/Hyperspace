
namespace exception
{
    [[ nodiscard ]] 
    long exception_filter(
        PEXCEPTION_POINTERS p_exception_pointers
    ) {
        const auto* pContext = p_exception_pointers->ContextRecord;
        char message[ 1024 ];

        sprintf( message ,
                 oxorany( "The service has been stopped due to a fatal error.\n\n"
                 "If this is the first time you've seen this error message, restart the service.\n"
                 "If this message appears again, follow these steps:\n\n"
                 "Rollback any newly downloaded Windows Updates.\n\n"
                 "If nothing helps, please contact Support via the tickets section.\n\n"
                 "Technical information:\n"
                 "Build type: 0 (built on %s %s)\n"
                 "Error code: 0x%08X\n"
                 "Address: %p\n"
                 "Attempt to read data at address: 0x0\n\n"
                 "ESP = 0x%016llX\n"
                 "EDI = 0x%016llX\n"
                 "ESI = 0x%016llX\n"
                 "EBX = 0x%016llX\n"
                 "EDX = 0x%016llX\n"
                 "ECX = 0x%016llX\n"
                 "EAX = 0x%016llX\n"
                 "EBP = 0x%016llX" ) ,
                 __DATE__ , __TIME__ ,
                 p_exception_pointers->ExceptionRecord->ExceptionCode ,
                 p_exception_pointers->ExceptionRecord->ExceptionAddress ,
                 pContext->Rsp ,
                 pContext->Rdi ,
                 pContext->Rsi ,
                 pContext->Rbx ,
                 pContext->Rdx ,
                 pContext->Rcx ,
                 pContext->Rax ,
                 pContext->Rbp
        );

        printf( oxorany( "\n" ) );
        logging::print( oxorany( "Stopped due to a fatal error" ) );

        MessageBoxA( 0 , message , "The service has crashed!" , MB_ICONERROR | MB_OK );
        exit( 0 );

        return EXCEPTION_CONTINUE_EXECUTION;
    }
}