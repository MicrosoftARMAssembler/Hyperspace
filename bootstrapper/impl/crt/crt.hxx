
using nt_load_driver_t = NTSTATUS ( __fastcall* )( PUNICODE_STRING );
using nt_rtl_allocate_heap_t = PVOID( __stdcall* )( PVOID , ULONG , SIZE_T );
using nt_rtl_free_heap_t = BOOLEAN( __stdcall* )( PVOID , ULONG , PVOID );

namespace crt
{
	std::uint64_t rand( ) {
		return __rdtsc( ) * __rdtsc( ) * __rdtsc( );
	}

	template <typename type>
	size_t str_len(type str) {
		if (!str) return 0;

		type str2 = str;
		while (*str2) *str2++;
		return str2 - str;
	}

	template <typename T_Src, typename T_Dst>
	void str_cpy(T_Src src, T_Dst dst, wchar_t null_t = 0) {
		if (!src || !dst) {
			return;
		}

		while ( true ) {
			wchar_t wchar = *src++;
			*dst = wchar;
			if ( wchar == null_t ) {
				break;
			}
			dst++;
		}
	}

	template <typename T_Dest, typename T_Src>
	void str_cat( T_Dest dest, T_Src src ) {
		if ( !dest || !src ) {
			return;
		}

		str_cpy( src, ( T_Dest ) &dest[ str_len( dest ) ] );
	}

	void mem_zero( PVOID dst, DWORD size ) {
		if ( !dst || !size ) return;
		__stosb( ( PBYTE ) dst, 0, size );
	}

	void memcpy( PVOID Dst, PVOID Src, DWORD Size ) {
		if ( !Dst || !Src || !Size ) return;
		__movsb( ( PBYTE ) Dst, ( const PBYTE ) Src, Size );
	}

	PVOID malloc1( ULONG64 size ) {
		auto* nt_rtl_allocate_heap = reinterpret_cast< nt_rtl_allocate_heap_t >(
			GetProcAddress(
			GetModuleHandleA( oxorany( "ntdll.dll" ) ), oxorany( "RtlAllocateHeap" ) )
			);
		if ( !nt_rtl_allocate_heap )
			return nullptr;

		return nt_rtl_allocate_heap( GetProcessHeap( ), HEAP_ZERO_MEMORY, size );
	}

	void free1( PVOID ptr ) {
		auto* nt_rtl_free_heap = reinterpret_cast< nt_rtl_free_heap_t >(
			GetProcAddress(
			GetModuleHandleA( oxorany( "ntdll.dll" ) ), oxorany( "RtlFreeHeap" ) )
			);
		if ( !nt_rtl_free_heap )
			return;

		nt_rtl_free_heap( GetProcessHeap( ), 0, ptr );
	}
}