
namespace pdb {
    struct symbol_context_t {
        std::vector<std::pair<std::string, std::uint64_t>> m_symbols;
        std::uint64_t m_module_base;
    };

    struct module_mapping_t {
        std::string original_name;
        std::string mapped_name;
    };

    std::vector<module_mapping_t> get_module_mappings( const char* file_path ) {
        std::vector<module_mapping_t> mappings;

        if ( !SymInitialize( GetCurrentProcess( ),
            oxorany( "http://msdl.microsoft.com/download/symbols" ),
            FALSE ) ) {
            logging::print( oxorany( "SymInitialize failed with error: %d" ), GetLastError( ) );
            return mappings;
        }

        SymSetOptions(
            SYMOPT_ALLOW_ABSOLUTE_SYMBOLS |
            SYMOPT_DEBUG |
            SYMOPT_LOAD_LINES |
            SYMOPT_UNDNAME |
            SYMOPT_DEFERRED_LOADS |
            SYMOPT_INCLUDE_32BIT_MODULES |
            SYMOPT_NO_UNQUALIFIED_LOADS |
            SYMOPT_AUTO_PUBLICS |
            SYMOPT_LOAD_ANYTHING
        );

        const auto module_base = SymLoadModule64(
            GetCurrentProcess( ),
            nullptr,
            file_path,
            nullptr,
            0,
            0
        );

        if ( !module_base ) {
            logging::print( oxorany( "SymLoadModule64 failed with error: %d" ), GetLastError( ) );
            SymCleanup( GetCurrentProcess( ) );
            return mappings;
        }

        IMAGEHLP_MODULEW64 module_info{};
        module_info.SizeOfStruct = sizeof( IMAGEHLP_MODULEW64 );
        if ( !SymGetModuleInfoW64( GetCurrentProcess( ), module_base, &module_info ) ) {
            logging::print( oxorany( "SymGetModuleInfoW64 failed with error: %d" ), GetLastError( ) );
            SymUnloadModule64( GetCurrentProcess( ), module_base );
            SymCleanup( GetCurrentProcess( ) );
            return mappings;
        }

        // Extract original module name from PDB
        if ( module_info.LoadedPdbName[ 0 ] != 0 ) {
            std::wstring pdb_path = module_info.LoadedPdbName;
            std::wstring pdb_name = pdb_path.substr( pdb_path.find_last_of( L"\\/" ) + 1 );

            // Strip .pdb extension
            if ( pdb_name.length( ) > 4 ) {
                pdb_name = pdb_name.substr( 0, pdb_name.length( ) - 4 );
            }

            // Convert wide to narrow
            char original_name[ MAX_PATH ] = { 0 };
            WideCharToMultiByte( CP_ACP, 0, pdb_name.c_str( ), -1, original_name, MAX_PATH, nullptr, nullptr );

            // Get actual loaded module name
            char loaded_name[ MAX_PATH ] = { 0 };
            WideCharToMultiByte( CP_ACP, 0, module_info.ModuleName, -1, loaded_name, MAX_PATH, nullptr, nullptr );

            module_mapping_t mapping;
            mapping.original_name = original_name;
            mapping.mapped_name = loaded_name;
            mappings.push_back( mapping );

            logging::print( oxorany( "PDB mapping: %s -> %s" ), original_name, loaded_name );
        }

        // Also check ImageName vs ModuleName
        if ( module_info.ImageName[ 0 ] != 0 ) {
            std::wstring image_path = module_info.ImageName;
            std::wstring image_name = image_path.substr( image_path.find_last_of( L"\\/" ) + 1 );

            // Strip extension from image name
            auto dot_pos = image_name.find_last_of( L'.' );
            if ( dot_pos != std::wstring::npos ) {
                image_name = image_name.substr( 0, dot_pos );
            }

            char image_name_narrow[ MAX_PATH ] = { 0 };
            WideCharToMultiByte( CP_ACP, 0, image_name.c_str( ), -1, image_name_narrow, MAX_PATH, nullptr, nullptr );

            char module_name_narrow[ MAX_PATH ] = { 0 };
            WideCharToMultiByte( CP_ACP, 0, module_info.ModuleName, -1, module_name_narrow, MAX_PATH, nullptr, nullptr );

            // Only add if different from PDB mapping
            if ( _stricmp( image_name_narrow, module_name_narrow ) != 0 ) {
                module_mapping_t mapping;
                mapping.original_name = image_name_narrow;
                mapping.mapped_name = module_name_narrow;
                mappings.push_back( mapping );

                logging::print( oxorany( "Image mapping: %s -> %s" ), image_name_narrow, module_name_narrow );
            }
        }

        SymUnloadModule64( GetCurrentProcess( ), module_base );
        SymCleanup( GetCurrentProcess( ) );

        return mappings;
    }

    BOOL CALLBACK sym_enum_callback( PSYMBOL_INFO p_sym_info, ULONG symbol_size, PVOID user_context ) {
        auto* ctx = reinterpret_cast< symbol_context_t* >( user_context );
        if ( p_sym_info && p_sym_info->Name ) {
            ctx->m_symbols.emplace_back(
                std::string( p_sym_info->Name ),
                p_sym_info->Address - ctx->m_module_base
            );
        }
        return TRUE;
    }

    std::vector<std::pair<std::string, std::uint64_t>> get_all_symbols( const char* file_path ) {
        std::vector<std::pair<std::string, std::uint64_t>> results;

        if ( !SymInitialize( GetCurrentProcess( ),
            oxorany( "http://msdl.microsoft.com/download/symbols" ),
            FALSE ) ) {
            logging::print( oxorany( "SymInitialize failed with error: %d" ), GetLastError( ) );
            return results;
        }

        SymSetOptions(
            SYMOPT_ALLOW_ABSOLUTE_SYMBOLS |
            SYMOPT_DEBUG |
            SYMOPT_LOAD_LINES |
            SYMOPT_UNDNAME |
            SYMOPT_DEFERRED_LOADS |
            SYMOPT_INCLUDE_32BIT_MODULES |
            SYMOPT_NO_UNQUALIFIED_LOADS |
            SYMOPT_AUTO_PUBLICS |
            SYMOPT_LOAD_ANYTHING
        );

        const auto module_base = SymLoadModule64(
            GetCurrentProcess( ),
            nullptr,
            file_path,
            nullptr,
            0,
            0
        );

        if ( !module_base ) {
            logging::print( oxorany( "SymLoadModule64 failed with error: %d" ), GetLastError( ) );
            SymCleanup( GetCurrentProcess( ) );
            return results;
        }

        IMAGEHLP_MODULEW64 module_info{};
        module_info.SizeOfStruct = sizeof( IMAGEHLP_MODULEW64 );
        if ( !SymGetModuleInfoW64( GetCurrentProcess( ), module_base, &module_info ) ) {
            logging::print( oxorany( "SymGetModuleInfoW64 failed with error: %d" ), GetLastError( ) );
            SymUnloadModule64( GetCurrentProcess( ), module_base );
            SymCleanup( GetCurrentProcess( ) );
            return results;
        }

        symbol_context_t ctx;
        ctx.m_module_base = module_base;

        if ( !SymEnumSymbols( GetCurrentProcess( ), module_base, "*", sym_enum_callback, &ctx ) ) {
            logging::print( oxorany( "SymEnumSymbols failed with error: %d" ), GetLastError( ) );
        }

        results = std::move( ctx.m_symbols );

        logging::print( oxorany( "Found %zu total symbols in PDB" ), results.size( ) );

        SymUnloadModule64( GetCurrentProcess( ), module_base );
        SymCleanup( GetCurrentProcess( ) );

        return results;
    }

    template<typename T>
    std::uintptr_t get_symbol_address( const char* file_path, T symbol_name ) {
        if ( !SymInitialize( GetCurrentProcess( ),
            "http://msdl.microsoft.com/download/symbols",
            FALSE ) ) {
            logging::print( oxorany( "SymInitialize failed with error: %d" ), GetLastError( ) );
            return 0;
        }

        SymSetOptions(
            SYMOPT_ALLOW_ABSOLUTE_SYMBOLS |
            SYMOPT_DEBUG |
            SYMOPT_LOAD_LINES |
            SYMOPT_UNDNAME |
            SYMOPT_DEFERRED_LOADS |
            SYMOPT_INCLUDE_32BIT_MODULES |
            SYMOPT_AUTO_PUBLICS |
            SYMOPT_LOAD_ANYTHING
        );

        const auto module_base = SymLoadModule64(
            GetCurrentProcess( ),
            nullptr,
            file_path,
            nullptr,
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

        auto* symbol_info = reinterpret_cast< PSYMBOL_INFO_PACKAGE >(
            _alloca( sizeof( SYMBOL_INFO_PACKAGE ) )
            );
        symbol_info->si.MaxNameLen = MAX_SYM_NAME;
        symbol_info->si.SizeOfStruct = sizeof( SYMBOL_INFO );

        std::uint64_t resolved_address = 0;

        if ( SymFromName( GetCurrentProcess( ), symbol_name, &symbol_info->si ) ) {
            resolved_address = symbol_info->si.Address - module_base;
            logging::print( oxorany( "Resolved symbol %s at offset 0x%llx" ),
                symbol_name, resolved_address );
        }
        else {
            logging::print( oxorany( "Could not parse PDB for %s with error: %d" ),
                symbol_name, GetLastError( ) );
        }

        SymUnloadModule64( GetCurrentProcess( ), module_base );
        SymCleanup( GetCurrentProcess( ) );

        return resolved_address;
    }
}