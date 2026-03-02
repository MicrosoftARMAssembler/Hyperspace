#pragma once

namespace hyperspace {
	data_request_t* m_data_request{ };
	root_request_t* m_root_request{ };
	WNF_STATE_NAME_INTERNAL m_state_name{ };
	WNF_STATE_NAME* m_state_name_ptr{ nullptr };
	nt_update_wnf_state_data_t m_nt_update_wnf_state_data{ };
	HANDLE m_response_semaphore{ nullptr };
	HANDLE m_request_semaphore{ nullptr };
	SRWLOCK m_request_lock{ };

	namespace driver {
		bool create_instrumentation( ) {
			auto teb_segment = reinterpret_cast< teb_t* >( __readgsqword( 0x30 ) );
			if ( !teb_segment )
				return false;

			m_response_semaphore = CreateSemaphoreA( nullptr, 0, LONG_MAX, nullptr );
			if ( !m_response_semaphore ) {
				logging::print( oxorany( "[hyperspace] create_instrumentation: Failed to create semaphore" ) );
				return false;
			}

			m_request_semaphore = CreateSemaphoreA( nullptr, 0, LONG_MAX, nullptr );
			if ( !m_request_semaphore ) {
				logging::print( oxorany( "[hyperspace] create_instrumentation: Failed to create semaphore" ) );
				return false;
			}

			teb_segment->m_instrumentation[ 11 ] = reinterpret_cast< std::uint8_t* >( &m_response_semaphore );
			teb_segment->m_instrumentation[ 6 ] = reinterpret_cast< std::uint8_t* >( &m_request_semaphore );

			ZeroMemory( &m_state_name, sizeof( m_state_name ) );
			m_state_name.Data[ 0 ] = static_cast< std::uint32_t >( 0x0d83063ea3be0875 & 0xFFFFFFFF );
			m_state_name.Data[ 1 ] = static_cast< std::uint32_t >( ( 0x0d83063ea3be0875 >> 32 ) & 0xFFFFFFFF );
			m_state_name_ptr = reinterpret_cast< WNF_STATE_NAME* >( &m_state_name );

			auto nt_dll = GetModuleHandleA( oxorany( "ntdll.dll" ) );
			if ( !nt_dll ) {
				logging::print( oxorany( "[hyperspace] create_instrumentation: Could not load ntdll" ) );
				return false;
			}

			m_nt_update_wnf_state_data = reinterpret_cast< nt_update_wnf_state_data_t >(
				GetProcAddress( nt_dll, oxorany( "NtUpdateWnfStateData" ) ) );
			if ( !m_nt_update_wnf_state_data ) {
				logging::print( oxorany( "[hyperspace] create_instrumentation: Could not get NtUpdateWnfStateData" ) );
				return false;
			}

			InitializeSRWLock( &m_request_lock );
			return true;
		}

		bool create_process( ) {
			STARTUPINFOA si;
			PROCESS_INFORMATION pi;

			ZeroMemory( &si, sizeof( si ) );
			si.cb = sizeof( si );
			ZeroMemory( &pi, sizeof( pi ) );

			CopyFileA(
				oxorany( "C:\\Windows\\System32\\RuntimeBroker.exe" ),
				oxorany( "hyperspace.exe" ),
				false
			);

			if ( !CreateProcessA(
				oxorany( "hyperspace.exe" ),
				NULL,
				NULL,
				NULL,
				FALSE,
				0,
				NULL,
				NULL,
				&si,
				&pi
			) )
				return false;

			TerminateProcess( pi.hProcess, 0 );
			SuspendThread( pi.hThread );
			DeleteFileA( oxorany( "hyperspace.exe" ) );
			return true;
		}

		bool map_instrumentation( ) {
			auto teb_segment = reinterpret_cast< teb_t* >( __readgsqword( 0x30 ) );
			if ( !teb_segment )
				return false;

			m_data_request = reinterpret_cast
				< data_request_t* >( teb_segment->m_instrumentation[ 9 ] );
			if ( !m_data_request ) {
				return false;
			}

			m_root_request = reinterpret_cast
				< root_request_t* >( teb_segment->m_instrumentation[ 10 ] );
			if ( !m_root_request ) {
				return false;
			}

			return true;
		}

		std::pair< std::uint32_t, std::uint32_t > get_pfn_ranges( ) {
			auto teb_segment = reinterpret_cast< teb_t* >( __readgsqword( 0x30 ) );
			if ( !teb_segment )
				return { };

			return std::make_pair(
				reinterpret_cast< std::uint32_t >( teb_segment->m_instrumentation[ 7 ] ),
				reinterpret_cast< std::uint32_t >( teb_segment->m_instrumentation[ 8 ] )
			);
		}

		void cleanup( ) {
			if ( m_request_semaphore ) {
				CloseHandle( m_request_semaphore );
				m_request_semaphore = nullptr;
			}

			if ( m_response_semaphore ) {
				CloseHandle( m_response_semaphore );
				m_response_semaphore = nullptr;
			}
		}

		bool send_request( int timeout_ms = 1000 ) {	
			AcquireSRWLockExclusive( &m_request_lock );
			ReleaseSemaphore( m_request_semaphore, 1, nullptr );
			
			if ( m_nt_update_wnf_state_data(
				m_state_name_ptr,
				nullptr,
				0,
				nullptr,
				nullptr,
				0,
				false
			) ) {
				ReleaseSRWLockExclusive( &m_request_lock );
				return false;
			}

			if ( WaitForSingleObject( m_response_semaphore, timeout_ms ) != WAIT_OBJECT_0 ) {
				ReleaseSRWLockExclusive( &m_request_lock );
				return false;
			}

			ReleaseSRWLockExclusive( &m_request_lock );
			return m_data_request->m_status;
		}
	}
}