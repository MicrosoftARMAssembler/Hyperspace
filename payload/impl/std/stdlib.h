
namespace std {
    char to_lower( char c ) {
        if ( c >= 'A' && c <= 'Z' )
            return c + ( 'a' - 'A' );
        return c;
    }

    std::size_t wcs_len( const wchar_t* str ) {
        if ( !str ) return 0;
        std::size_t len = 0;
        while ( str[ len ] != L'\0' ) len++;
        return len;
    }

    bool wchar_equal_ignore_case( wchar_t c1, wchar_t c2 ) {
        if ( c1 >= L'A' && c1 <= L'Z' ) c1 += 32;
        if ( c2 >= L'A' && c2 <= L'Z' ) c2 += 32;
        return c1 == c2;
    }

    const wchar_t* wcsstr_lowercase( const wchar_t* str, const wchar_t* substr ) {
        if ( !str || !substr || *substr == L'\0' )
            return str;

        for ( ; *str; ++str ) {
            const wchar_t* h = str;
            const wchar_t* n = substr;

            while ( *h && *n && wchar_equal_ignore_case( *h, *n ) ) {
                h++;
                n++;
            }

            if ( *n == L'\0' )
                return str;
        }

        return nullptr;
    }

    const char* strstr( const char* haystack, const char* needle ) {
        if ( !haystack || !needle )
            return nullptr;

        if ( *needle == '\0' )
            return haystack;

        for ( ; *haystack != '\0'; ++haystack ) {
            const char* h = haystack;
            const char* n = needle;

            while ( *h != '\0' && *n != '\0' && *h == *n ) {
                ++h;
                ++n;
            }

            if ( *n == '\0' )
                return haystack;
        }

        return nullptr;
    }


    unsigned long strtoul(
        const char* str,
        char** endptr,
        int base
    ) {
        while ( *str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' )
            str++;

        bool negative = false;
        if ( *str == '-' ) {
            negative = true;
            str++;
        }
        else if ( *str == '+' ) {
            str++;
        }

        if ( base == 0 ) {
            if ( *str == '0' ) {
                str++;
                if ( *str == 'x' || *str == 'X' ) {
                    base = 16;
                    str++;
                }
                else {
                    base = 8;
                }
            }
            else {
                base = 10;
            }
        }
        else if ( base == 16 ) {
            if ( *str == '0' && ( *( str + 1 ) == 'x' || *( str + 1 ) == 'X' ) ) {
                str += 2;
            }
        }

        unsigned long result = 0;
        bool valid_digit_found = false;

        while ( *str ) {
            int digit;

            if ( *str >= '0' && *str <= '9' ) {
                digit = *str - '0';
            }
            else if ( *str >= 'a' && *str <= 'z' ) {
                digit = *str - 'a' + 10;
            }
            else if ( *str >= 'A' && *str <= 'Z' ) {
                digit = *str - 'A' + 10;
            }
            else {
                break;
            }

            if ( digit >= base ) {
                break;
            }

            valid_digit_found = true;

            if ( result > ( ULONG_MAX - digit ) / base ) {
                result = ULONG_MAX;
                break;
            }

            result = result * base + digit;
            str++;
        }

        if ( endptr ) {
            *endptr = const_cast< char* >( valid_digit_found ? str : str - valid_digit_found );
        }

        return negative ? static_cast< unsigned long >( -static_cast< long >( result ) ) : result;
    }

    const char* str_str(
        const char* haystack,
        const char* needle
    ) {
        if ( !haystack || !needle )
            return nullptr;

        if ( !*needle )
            return haystack;

        const char* p1 = haystack;
        while ( *p1 ) {
            const char* p1_begin = p1;
            const char* p2 = needle;

            while ( *p1 && *p2 && ( *p1 == *p2 ) ) {
                p1++;
                p2++;
            }

            if ( !*p2 )
                return p1_begin;

            p1 = p1_begin + 1;
        }

        return nullptr;
    }

    int tolower( int c ) {
        if ( c >= 'A' && c <= 'Z' ) {
            return c + ( 'a' - 'A' );
        }
        return c;
    }

    char* strtolower( char* str ) {
        for ( char* p = str; *p != '\0'; ++p ) {
            *p = static_cast< char >( tolower( *p ) );
        }
        return str;
    }

    void* memcpy(
        void* dest,
        const void* src,
        size_t len
    ) {
        char* d = ( char* )dest;
        const char* s = ( const char* )src;
        while ( len-- )
            *d++ = *s++;
        return d - reinterpret_cast<char>( dest );
    }

    int wcscmp(
        const wchar_t* s1,
        const wchar_t* s2
    ) {
        while ( *s1 == *s2++ )
            if ( *s1++ == '\0' )
                return ( 0 );

        return ( *( const unsigned int* )s1 - *( const unsigned int* )--s2 );
    }

    std::int32_t strcmp(
        const char* string,
        const char* string_cmp
    ) {
        while ( *string != '\0' ) {
            if ( *string != *string_cmp )
                break;
            string++;
            string_cmp++;
        }
        return *string - *string_cmp;
    }

    std::size_t strlen(
        const char* str
    ) {
        const char* s;
        for ( s = str; *s; ++s );
        return ( s - str );
    }

    static const char null_string[ ] = "(null)";
    static const char hex_digits[ ] = "0123456789abcdef";
#define is_digit(c) ((c) >= '0' && (c) <= '9')

    static inline int format_uint( char* buf, unsigned int val ) {
        if ( val == 0 ) {
            *buf = '0';
            return 1;
        }

        unsigned int temp = val;
        int num_digits = 0;
        do {
            num_digits++;
            temp /= 10;
        } while ( temp > 0 );

        int idx = num_digits - 1;
        do {
            buf[ idx-- ] = '0' + ( val % 10 );
            val /= 10;
        } while ( val > 0 );

        return num_digits;
    }

    static inline int format_int( char* buf, int val, int* is_negative ) {
        unsigned int abs_val;

        if ( val < 0 ) {
            *is_negative = 1;
            abs_val = ( val == -2147483648 ) ? ( unsigned int )( -( -2147483648 + 1 ) ) + 1 : ( unsigned int )-val;
        }
        else {
            *is_negative = 0;
            abs_val = ( unsigned int )val;
        }

        return format_uint( buf, abs_val );
    }

    static inline int format_hex( char* buf, unsigned int val ) {
        if ( val == 0 ) {
            *buf = '0';
            return 1;
        }

        int num_digits = 0;
        unsigned int temp = val;

        while ( temp ) {
            num_digits++;
            temp >>= 4;
        }

        int idx = num_digits - 1;
        temp = val;
        do {
            buf[ idx-- ] = hex_digits[ temp & 0xF ];
            temp >>= 4;
        } while ( temp > 0 );

        return num_digits;
    }

    bool strstr_lowercase( const char* haystack, const char* needle ) {
        if ( !haystack || !needle )
            return false;

        auto haystack_len = std::strlen( haystack );
        auto needle_len = std::strlen( needle );

        if ( needle_len > haystack_len )
            return false;

        for ( std::size_t i = 0; i <= haystack_len - needle_len; i++ ) {
            bool match = true;
            for ( std::size_t j = 0; j < needle_len; j++ ) {
                if ( to_lower( haystack[ i + j ] ) != to_lower( needle[ j ] ) ) {
                    match = false;
                    break;
                }
            }
            if ( match )
                return true;
        }
        return false;
    }

    static inline int fast_strcpy( char* dst, const char* src, size_t max ) {
        const char* s = src;
        int len = 0;
        size_t i = 0;

        if ( dst != NULL && max > 0 ) {
            while ( *s && i < max ) { 
                *dst++ = *s++;
                len++;
                i++;
            }
        }

        while ( *s ) {
            len++;
            s++;
        }

        return len;
    }

    static int core_format( char* str, size_t max_size, const char* format, va_list args ) {
        if ( format == NULL ) {
            if ( max_size > 0 && str != NULL ) {
                str[ 0 ] = '\0';
            }
            return 0;
        }

        int total_length = 0;
        size_t buffer_pos = 0;

        const size_t buffer_limit = ( max_size > 0 ) ? max_size - 1 : 0;
        int writing = ( str != NULL && max_size > 0 );

        while ( *format ) {
            if ( *format != '%' ) {
                if ( writing && buffer_pos < buffer_limit ) {
                    str[ buffer_pos ] = *format;
                }

                buffer_pos++;
                total_length++;
                format++;
                continue;
            }

            format++;

            if ( *format == '%' ) {
                if ( writing && buffer_pos < buffer_limit ) {
                    str[ buffer_pos ] = '%';
                }
                buffer_pos++;
                total_length++;
                format++;
                continue;
            }

            char type = *format++;
            switch ( type ) {
            case 'd': {
                int val = va_arg( args, int );
                int is_negative = 0;

                char digits[ sizeof( int ) * CHAR_BIT / 3 + 2 ];
                int num_digits = format_int( digits, val, &is_negative );

                if ( is_negative ) {
                    if ( writing && buffer_pos < buffer_limit ) {
                        str[ buffer_pos ] = '-';
                    }
                    buffer_pos++;
                    total_length++;
                }

                for ( int i = 0; i < num_digits; i++ ) {
                    if ( writing && buffer_pos < buffer_limit ) {
                        str[ buffer_pos ] = digits[ i ];
                    }
                    buffer_pos++;
                    total_length++;
                }
                break;
            }

            case 'u': {
                unsigned int val = va_arg( args, unsigned int );

                char digits[ 10 ];
                int num_digits = format_uint( digits, val );

                for ( int i = 0; i < num_digits; i++ ) {
                    if ( writing && buffer_pos < buffer_limit ) {
                        str[ buffer_pos ] = digits[ i ];
                    }
                    buffer_pos++;
                    total_length++;
                }
                break;
            }

            case 'x': {
                unsigned int val = va_arg( args, unsigned int );

                char digits[ sizeof( unsigned int ) * CHAR_BIT / 4 + 1 ];
                int num_digits = format_hex( digits, val );

                for ( int i = 0; i < num_digits; i++ ) {
                    if ( writing && buffer_pos < buffer_limit ) {
                        str[ buffer_pos ] = digits[ i ];
                    }
                    buffer_pos++;
                    total_length++;
                }
                break;
            }

            case 's': {
                char* s = va_arg( args, char* );

                if ( !s ) {
                    s = ( char* )null_string;
                }

                size_t remaining = ( writing && buffer_pos < buffer_limit ) ? ( buffer_limit - buffer_pos ) : 0;
                int len = fast_strcpy( writing ? str + buffer_pos : NULL, s, remaining );

                buffer_pos += len;
                total_length += len;
                break;
            }

            case 'c': {
                char c = ( char )va_arg( args, int );
                if ( writing && buffer_pos < buffer_limit ) {
                    str[ buffer_pos ] = c;
                }
                buffer_pos++;
                total_length++;
                break;
            }

            default:
                break;
            }
        }

        if ( writing ) {
            if ( buffer_pos < max_size ) {
                str[ buffer_pos ] = '\0';
            }
            else {
                str[ max_size - 1 ] = '\0';
            }
        }
 
        return total_length;
    }

    int vsprintf( char* str, const char* format, va_list ap ) {
        return core_format( str, ( size_t )-1, format, ap );
    }

    int sprintf( char* str, const char* format, ... ) {
        va_list args;
        va_start( args, format );
        int result = vsprintf( str, format, args );
        va_end( args );
        return result;
    }

    int vsnprintf( char* str, size_t size, const char* format, va_list ap ) {
        return core_format( str, size, format, ap );
    }

    int snprintf( char* str, size_t size, const char* format, ... ) {
        va_list args;
        va_start( args, format );
        int result = vsnprintf( str, size, format, args );
        va_end( args );
        return result;
    }
}