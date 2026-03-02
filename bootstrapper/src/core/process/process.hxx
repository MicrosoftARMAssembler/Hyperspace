#pragma once

namespace process {
	std::uint64_t get_directory_table_base( eprocess_t* eprocess ) {
		if ( !eprocess )
			return 0;

		std::uint64_t directory_table_base;
		nt::memcpy( &directory_table_base,
			reinterpret_cast< void* >(
				reinterpret_cast< std::uintptr_t >( eprocess ) + 0x28 ),
			sizeof( std::uint64_t ) );

		return directory_table_base;
	}

	std::uint64_t get_ps_initial_system_process( ) {
		static auto ps_initial_system_process = 0ull;
		if ( !ps_initial_system_process ) {
			auto export_addr = g_ntoskrnl->get_export( oxorany( "PsInitialSystemProcess" ) );
			if ( !export_addr )
				return 0;
			
			std::uint64_t system_process = 0;
			nt::memcpy( &system_process, reinterpret_cast< void* >( export_addr ), sizeof( std::uint64_t ) );
			ps_initial_system_process = system_process;
		}
		
		return ps_initial_system_process;
	}

	eprocess_t* get_eprocess( const char* process_name ) {
		auto ps_active_process_head = nt::get_ps_active_process_head( );
		if ( !ps_active_process_head )
			return nullptr;

		auto ps_initial_system = get_ps_initial_system_process( );
		if ( !ps_initial_system )
			return nullptr;

		auto linkage_offset = ps_active_process_head - ps_initial_system;
		if ( !linkage_offset )
			return nullptr;

		list_entry_t process_list_head;
		nt::memcpy( &process_list_head, reinterpret_cast< void* >( ps_active_process_head ), sizeof( list_entry_t ) );

		auto current_flink = reinterpret_cast< std::uint64_t >( process_list_head.m_flink );
		while ( current_flink != ps_active_process_head ) {
			auto current_process = reinterpret_cast< eprocess_t* >( current_flink - linkage_offset );
			if ( !nt::is_address_valid( current_process ) ) {
				break;
			}

			auto process_image_file_name = reinterpret_cast< void* >(
				nt::ps_get_process_image_file_name( current_process ) );
			if ( !nt::is_address_valid( process_image_file_name ) )
				continue;

			char image_file_name[ 16 ] = { 0 };
			nt::memcpy(
				image_file_name,
				process_image_file_name,
				15
			);

			if ( _stricmp( image_file_name, process_name ) == 0 ) {
				return current_process;
			}

			list_entry_t current_entry;
			nt::memcpy( &current_entry, reinterpret_cast< void* >( current_flink ), sizeof( list_entry_t ) );
			current_flink = reinterpret_cast< std::uint64_t >( current_entry.m_flink );
		}

		return nullptr;
	}

	eprocess_t* get_session_process( const char* process_name ) {
		auto ps_active_process_head = nt::get_ps_active_process_head( );
		if ( !ps_active_process_head )
			return nullptr;

		auto ps_initial_system = get_ps_initial_system_process( );
		if ( !ps_initial_system )
			return nullptr;

		auto linkage_offset = ps_active_process_head - ps_initial_system;
		if ( !linkage_offset )
			return nullptr;

		list_entry_t process_list_head;
		nt::memcpy( &process_list_head, reinterpret_cast< void* >( ps_active_process_head ), sizeof( list_entry_t ) );

		auto current_flink = reinterpret_cast< std::uint64_t >( process_list_head.m_flink );
		while ( current_flink != ps_active_process_head ) {
			auto current_process = reinterpret_cast< eprocess_t* >( current_flink - linkage_offset );
			if ( !nt::is_address_valid( current_process ) ) {
				break;
			}

			auto session_id = nt::ps_get_process_session_id( current_process );
			if ( session_id ) {
				auto process_image_file_name = reinterpret_cast< void* >( 
					nt::ps_get_process_image_file_name( current_process ) );
				if ( !nt::is_address_valid( process_image_file_name ) )
					return 0;

				char image_file_name[ 16 ] = { 0 };
				nt::memcpy(
					image_file_name,
					process_image_file_name,
					15
				);

				if ( _stricmp( image_file_name, process_name ) == 0 ) {
					return current_process;
				}
			}

			list_entry_t current_entry;
			nt::memcpy( &current_entry, reinterpret_cast< void* >( current_flink ), sizeof( list_entry_t ) );
			current_flink = reinterpret_cast< std::uint64_t >( current_entry.m_flink );
		}

		return nullptr;
	}
}

