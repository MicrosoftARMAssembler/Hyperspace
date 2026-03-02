#pragma once

namespace client {
	namespace root {
		control::root_request_t m_root_mappings[ control::m_max_messages ]{ 0 };
		std::uint32_t m_head_index{ 0 };
		std::uint32_t m_tail_index{ 0 };

		template <class type>
		char* lukas_itoa( type value, char* result, int base, bool upper = false );
		bool copy_str( char* const buffer, char const* const source, std::uint32_t& index );
		void format( char* const buffer, char const* const format, va_list& args );

		std::uint32_t size( ) {
			return ( m_head_index - m_tail_index + control::m_max_messages ) % control::m_max_messages;
		}

		bool is_full( ) {
			return size( ) == control::m_max_messages - 1;
		}

		void pop( ) {
			if ( m_tail_index != m_head_index ) {
				m_tail_index = ( m_tail_index + 1 ) % control::m_max_messages;
			}
		}

		void push( const char* fmt, ... ) {
			if ( is_full( ) ) {
				pop( );
			}

			auto& current = m_root_mappings[ m_head_index ];

			va_list args;
			va_start( args, fmt );
			format( current.m_root_message, fmt, args );
			va_end( args );

			m_head_index = ( m_head_index + 1 ) % control::m_max_messages;
		}

		const char* front( ) {
			if ( m_tail_index == m_head_index ) {
				return nullptr;
			}
			return m_root_mappings[ m_tail_index ].m_root_message;
		}

		bool is_empty( ) {
			return m_tail_index == m_head_index;
		}

		bool copy_str( char* const buffer, char const* const source, std::uint32_t& index ) {
			for ( std::uint32_t i = 0; source[ i ]; ++i ) {
				buffer[ index++ ] = source[ i ];

				if ( index >= control::m_max_message_size - 1 ) {
					buffer[ control::m_max_message_size - 1 ] = oxorany( '\0' );
					return true;
				}
			}
			return false;
		}

		void format( char* const buffer, char const* const fmt, va_list& args ) {
			std::uint32_t buffer_index = 0;
			std::uint32_t format_index = 0;
			bool last_character_was_percentage = false;

			while ( true ) {
				auto const current_character = fmt[ format_index++ ];
				if ( current_character == oxorany( '\0' ) ) {
					break;
				}

				if ( current_character == oxorany( '%' ) ) {
					last_character_was_percentage = true;
					continue;
				}

				if ( !last_character_was_percentage ) {
					buffer[ buffer_index++ ] = current_character;
					if ( buffer_index >= control::m_max_message_size - 1 )
						break;
					continue;
				}

				char format_buffer[ 128 ]{};

				if ( current_character == oxorany( 'l' ) && fmt[ format_index ] == oxorany( 'l' ) ) {
					format_index++;
					switch ( fmt[ format_index++ ] ) {
					case 'x':
						if ( copy_str( buffer, oxorany( "0x" ), buffer_index ) ) return;
						if ( copy_str( buffer, lukas_itoa( va_arg( args, std::uint64_t ), format_buffer, 16 ), buffer_index ) ) return;
						break;
					}
					last_character_was_percentage = false;
					continue;
				}

				switch ( current_character ) {
				case 's':
					if ( copy_str( buffer, va_arg( args, char const* ), buffer_index ) ) return;
					break;
				case 'd':
				case 'i':
					if ( copy_str( buffer, lukas_itoa( va_arg( args, int ), format_buffer, 10 ), buffer_index ) ) return;
					break;
				case 'u':
					if ( copy_str( buffer, lukas_itoa( va_arg( args, unsigned int ), format_buffer, 10 ), buffer_index ) ) return;
					break;
				case 'x':
					if ( copy_str( buffer, oxorany( "0x" ), buffer_index ) ) return;
					if ( copy_str( buffer, lukas_itoa( va_arg( args, unsigned int ), format_buffer, 16 ), buffer_index ) ) return;
					break;
				case 'X':
					if ( copy_str( buffer, oxorany( "0x" ), buffer_index ) ) return;
					if ( copy_str( buffer, lukas_itoa( va_arg( args, unsigned int ), format_buffer, 16, true ), buffer_index ) ) return;
					break;
				case 'p':
					if ( copy_str( buffer, oxorany( "0x" ), buffer_index ) ) return;
					if ( copy_str( buffer, lukas_itoa( va_arg( args, std::uint64_t ), format_buffer, 16, true ), buffer_index ) ) return;
					break;
				}

				last_character_was_percentage = false;
			}

			buffer[ buffer_index ] = oxorany( '\0' );
		}

		template <class type>
		char* lukas_itoa( type value, char* result, int base, bool upper ) {
			if ( base < 2 || base > 36 ) {
				*result = oxorany( '\0' );
				return result;
			}

			char* ptr = result, * ptr1 = result, tmp_char;
			type tmp_value;

			if ( upper ) {
				do {
					tmp_value = value;
					value /= base;
					*ptr++ = oxorany( "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" ) [ 35 + ( tmp_value - value * base ) ];
				} while ( value );
			}
			else {
				do {
					tmp_value = value;
					value /= base;
					*ptr++ = oxorany( "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" ) [ 35 + ( tmp_value - value * base ) ];
				} while ( value );
			}

			if ( tmp_value < 0 )
				*ptr++ = oxorany( '-' );

			*ptr-- = oxorany( '\0' );
			while ( ptr1 < ptr ) {
				tmp_char = *ptr;
				*ptr-- = *ptr1;
				*ptr1++ = tmp_char;
			}

			return result;
		}
	}
}