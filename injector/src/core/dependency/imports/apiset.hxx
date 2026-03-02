#pragma once

namespace apiset {
    struct api_set_value_entry_10_t {
        ULONG m_flags;
        ULONG m_name_offset;
        ULONG m_name_length;
        ULONG m_value_offset;
        ULONG m_value_length;
    };
    using papi_set_value_entry_10_t = api_set_value_entry_10_t*;

    struct api_set_value_array_10_t {
        ULONG m_flags;
        ULONG m_name_offset;
        ULONG m_unk;
        ULONG m_name_length;
        ULONG m_data_offset;
        ULONG m_count;

        inline papi_set_value_entry_10_t entry( void* p_api_set, DWORD i ) {
            return ( papi_set_value_entry_10_t )( ( BYTE* )p_api_set + m_data_offset + i * sizeof( api_set_value_entry_10_t ) );
        }
    };
    using papi_set_value_array_10_t = api_set_value_array_10_t*;

    struct api_set_namespace_entry_10_t {
        ULONG m_limit;
        ULONG m_size;
    };
    using papi_set_namespace_entry_10_t = api_set_namespace_entry_10_t*;

    struct api_set_namespace_array_10_t {
        ULONG m_version;
        ULONG m_size;
        ULONG m_flags;
        ULONG m_count;
        ULONG m_start;
        ULONG m_end;
        ULONG m_unk[ 2 ];

        inline papi_set_namespace_entry_10_t entry( DWORD i ) {
            return ( papi_set_namespace_entry_10_t )( ( BYTE* )this + m_end + i * sizeof( api_set_namespace_entry_10_t ) );
        }

        inline papi_set_value_array_10_t val_array( papi_set_namespace_entry_10_t p_entry ) {
            return ( papi_set_value_array_10_t )( ( BYTE* )this + m_start + sizeof( api_set_value_array_10_t ) * p_entry->m_size );
        }

        inline ULONG api_name( papi_set_namespace_entry_10_t p_entry, wchar_t* output ) {
            auto p_array = val_array( p_entry );
            memcpy( output, ( char* )this + p_array->m_name_offset, p_array->m_name_length );
            return p_array->m_name_length;
        }
    };
    using papi_set_namespace_array_10_t = api_set_namespace_array_10_t*;

    class c_apiset {
    private:
        bool m_initialized;

        bool initialize( ) {
            if ( m_initialized )
                return !m_api_schema.empty( );

            m_initialized = true;

            auto peb = reinterpret_cast< PEB* >( NtCurrentTeb( )->ProcessEnvironmentBlock );
            auto api_set_map_ptr = *reinterpret_cast< void** >( reinterpret_cast< BYTE* >( peb ) + 0x68 );
            if ( !api_set_map_ptr )
                return false;

            auto set_map = reinterpret_cast< api_set_namespace_array_10_t* >( api_set_map_ptr );
            if ( set_map->m_version < 2 || set_map->m_count == 0 )
                return false;

            for ( auto i = 0; i < set_map->m_count; i++ ) {
                auto descriptor = set_map->entry( i );

                wchar_t dll_name[ MAX_PATH ] = { 0 };
                auto name_size = set_map->api_name( descriptor, dll_name );
                std::transform( dll_name, dll_name + name_size / sizeof( wchar_t ), dll_name, ::towlower );

                std::vector<std::wstring> vhosts;
                auto host_data = set_map->val_array( descriptor ); // ✅ FIXED: was pDescriptor
                for ( auto j = 0; j < host_data->m_count; j++ ) {
                    auto host = host_data->entry( set_map, j );

                    std::wstring host_name(
                        reinterpret_cast< wchar_t* >( reinterpret_cast< uint8_t* >( set_map ) + host->m_value_offset ),
                        host->m_value_length / sizeof( wchar_t )
                    );

                    if ( !host_name.empty( ) )
                        vhosts.emplace_back( std::move( host_name ) );
                }

                m_api_schema.emplace( dll_name, std::move( vhosts ) );
            }

            return !m_api_schema.empty( );
        }

        std::wstring normalize_name( const std::wstring& name ) {
            std::wstring normalized = name;

            std::transform( normalized.begin( ), normalized.end( ), normalized.begin( ),
                [ ]( wchar_t c ) { return static_cast< wchar_t >( ::towlower( c ) ); } );

            size_t dll_pos = normalized.rfind( L".dll" );
            if ( dll_pos != std::wstring::npos && dll_pos == normalized.length( ) - 4 ) {
                normalized = normalized.substr( 0, dll_pos );
            }

            return normalized;
        }

    public:
        c_apiset( ) : m_initialized( false ) { }
        std::map<std::wstring, std::vector<std::wstring>> m_api_schema;

        std::wstring resolve_api_set( const std::wstring& filename, const std::wstring& base_name = L"" ) {
            if ( !initialize( ) )
                return filename;

            std::wstring normalized_filename = normalize_name( filename );
            if ( normalized_filename.find( L"ext-ms-" ) == 0 ) {
                normalized_filename.replace( 0, 7, L"api-ms-" );
            }

            auto iter = std::find_if( m_api_schema.begin( ), m_api_schema.end( ),
                [ &normalized_filename ]( const auto& val ) {
                    return normalized_filename.find( val.first.c_str( ) ) != normalized_filename.npos;
                } );

            if ( iter != m_api_schema.end( ) ) {
                if ( !iter->second.empty( ) ) {
                    if ( !base_name.empty( ) && iter->second.front( ) != base_name ) {
                        return iter->second.front( );
                    }
                    else if ( iter->second.size( ) > 1 ) {
                        return iter->second.back( );
                    }
                    else {
                        return iter->second.front( );
                    }
                }
                else if ( !base_name.empty( ) ) {
                    return base_name;
                }
            }

            return filename;
        }

        std::vector<std::wstring> get_all_hosts( const std::wstring& name ) {
            if ( !initialize( ) )
                return {};

            std::wstring normalized = normalize_name( name );

            auto it = m_api_schema.find( normalized );
            if ( it != m_api_schema.end( ) ) {
                return it->second;
            }

            return {};
        }

        bool is_initialized( ) const {
            return m_initialized && !m_api_schema.empty( );
        }

        size_t get_count( ) const {
            return m_api_schema.size( );
        }
    };
}