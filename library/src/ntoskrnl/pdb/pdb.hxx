
namespace pdb {
    BOOL CALLBACK SymEnumCallback( PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext ) {
        return TRUE; // Continue enumeration
    }

    template<typename T>
    std::uintptr_t get_symbol_address( const char* file_path, T symbol_name ) {
        if ( !SymInitialize( GetCurrentProcess( ),
            "http://msdl.microsoft.com/download/symbols",
            FALSE ) ) {
            logging::print( oxorany( "SymInitialize failed with error: %d" ), GetLastError( ) );
            return 0;
        }

        SymSetOptions( SYMOPT_ALLOW_ABSOLUTE_SYMBOLS |
            SYMOPT_DEBUG |
            SYMOPT_LOAD_LINES |
            SYMOPT_UNDNAME |
            SYMOPT_DEFERRED_LOADS );

        const auto module_base = SymLoadModule64(
            GetCurrentProcess( ),
            nullptr,
            file_path,
            "nt",
            0,
            0
        );

        if ( !module_base ) {
            logging::print( oxorany( "SymLoadModule64 failed with error: %d" ), GetLastError( ) );
            SymCleanup( GetCurrentProcess( ) );
            return 0;
        }

        IMAGEHLP_MODULEW64 module_info{};
        module_info.SizeOfStruct = sizeof( IMAGEHLP_MODULEW64 );
        if ( !SymGetModuleInfoW64( GetCurrentProcess( ), module_base, &module_info ) ) {
            logging::print( oxorany( "SymGetModuleInfoW64 failed with error: %d" ), GetLastError( ) );
            SymUnloadModule64( GetCurrentProcess( ), module_base );
            SymCleanup( GetCurrentProcess( ) );
            return 0;
        }

        auto* symbol_info = reinterpret_cast< PSYMBOL_INFO_PACKAGE >( _alloca( sizeof( SYMBOL_INFO ) ) );
        symbol_info->si.MaxNameLen = MAX_SYM_NAME;
        symbol_info->si.SizeOfStruct = sizeof( SYMBOL_INFO );

        SymEnumSymbols( GetCurrentProcess( ), module_base, "", SymEnumCallback, 0 );

        std::uint64_t resolved_address = 0;
        if ( SymFromName( GetCurrentProcess( ), symbol_name, &symbol_info->si ) ) {
            resolved_address = symbol_info->si.Address - module_base;
        }
        else {
            logging::print( oxorany( "Could not parse PDB for %s with error: %d" ), symbol_name, GetLastError( ) );
        }

        SymUnloadModule64( GetCurrentProcess( ), module_base );
        SymCleanup( GetCurrentProcess( ) );

        return resolved_address;
    }
}