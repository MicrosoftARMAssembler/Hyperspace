#pragma once

namespace map {
	class c_map {
	public:
		c_map( std::shared_ptr< dependency::c_dependency > dependency ) : m_dependency( dependency ) { }

		bool inject( ) {
			this->m_allocation_size = m_dependency->size_of_image( );
			this->m_allocation_base = hyperspace::paging::allocate_virtual( m_allocation_size );
			if ( !m_allocation_base ) {
				logging::print( oxorany( "[c_map] inject: Failed to allocate virtual\n" ) );
				return false;
			}

			if ( !hyperspace::paging::translate_linear( m_allocation_base, &m_allocation_phys ) ) {
				logging::print( oxorany( "[c_map] inject: Failed to translate linear\n" ) );
				return false;
			}

			logging::print( oxorany( "[c_map] inject: Base: 0x%llx (Size: 0x%X)\n" ),
				m_allocation_base, m_allocation_size );

			if ( !m_dependency->map_relocs( m_allocation_base ) ) {
				logging::print( oxorany( "[c_map] inject: Failed to map relocs\n" ) );
				return false;
			}

			logging::print( oxorany( "[c_map] inject: Mapped relocations" ) );

			if ( !m_dependency->map_imports( ) ) {
				logging::print( oxorany( "[c_map] inject: Failed to map imports\n" ) );
				return false;
			}

			logging::print( oxorany( "[c_map] inject: Mapped imports" ) );

			if ( !m_dependency->map_sections( m_allocation_base ) ) {
				logging::print( oxorany( "[c_map] inject: Failed to map sections\n" ) );
				return false;
			}

			logging::print( oxorany( "[c_map] inject: Successfully mapped module!" ) );

			this->m_entry_point = m_dependency->get_export( oxorany( "DllMain" ) );
			if ( !m_entry_point ) {
				logging::print( oxorany( "[c_map] inject: Failed to find DllMain\n" ) );
				return false;
			}

			m_entry_point += m_allocation_base;
			logging::print( oxorany( "[c_map] inject: Entry point: 0x%llx\n" ), m_entry_point );

			return true;
		}

		bool execute( ) {
			execute::shellcode_data_t execute_data { };
			execute_data.m_entry_point = m_entry_point;
			execute_data.m_dll_base = m_allocation_base;

			auto shellcode_data = std::make_shared< execute::shellcode_t >( );
			auto execute = std::make_unique< execute::c_execute >( shellcode_data, execute_data );
			if ( !execute->compile( ) )
				return false;

			auto shellcode_address = execute->map( );
			if ( !shellcode_address )
				return false;

			auto data_address = execute->get_data( );
			if ( !data_address )
				return false;

			logging::print( oxorany( "[c_map] execute: Compiled shellcode" ) );
			logging::print( oxorany( "[c_map] execute: Preparing for execution press enter." ) );
			std::getchar( );

			auto discord_module = hyperspace::target::get_process_module( oxorany( L"DiscordHook64.dll" ) );
			if ( !discord_module ) {
				logging::print( oxorany( "[c_map] execute: Could not find discord module!" ) );
				logging::print( oxorany( "[c_map] execute: Try to enable legacy overlay and try again." ) );
				return false;
			}

			auto peek_message_hk = discord_module + 0x40AD0;
			auto peek_message_org = hyperspace::read_virt( peek_message_hk );
			if (!peek_message_org) {
				logging::print(oxorany("[c_map] execute: Could not find pee message!"));
				return false;
			}

			hyperspace::write_virt( peek_message_hk, shellcode_address );

			logging::print( oxorany( "[c_map] execute: Executed shellcode" ) );

			auto start_time = std::chrono::steady_clock::now( );
			const auto timeout = std::chrono::seconds( 10 );

			while ( true ) {
				if ( !hyperspace::target::get_process_id( oxorany( target_process ) ) ) {
					logging::print( oxorany( "[c_map] execute: Target process terminated" ) );
					return false;
				}

				auto shellcode_data = hyperspace::read_virt<execute::shellcode_data_t>( data_address );
				if ( shellcode_data.m_status == execute::shellcode_status::finished ) {
					logging::print( oxorany( "[c_map] execute: Shellcode finished successfully!" ) );
					//logging::print( oxorany( "[c_map] execute: Return value: 0x%llx" ), shellcode_data.m_return );
					break;
				}

				logging::print( oxorany( "[c_map] execute: Shellcode status: %d" ), shellcode_data.m_status );

				auto elapsed = std::chrono::steady_clock::now( ) - start_time;
				if ( elapsed > timeout ) {
					logging::print( oxorany( "[c_map] execute: Unexpectedly timed out during execution" ) );
					logging::print( oxorany( "[c_map] execute: Last status: %d" ), shellcode_data.m_status );
					return false;
				}

				std::this_thread::sleep_for(
					std::chrono::milliseconds(
						25
					) );
			}

			logging::print( oxorany( "[c_map] execute: Cleaning execution" ) );
			
			hyperspace::write_virt( peek_message_hk, peek_message_org );
			std::getchar( );

			//if ( !execute->cleanup( ) )
			//	return false;

			//if ( !m_dependency->erase_discarded_sec( m_allocation_base ) )
			//	return false;

			logging::print( oxorany( "[c_map] execute: Execution completed successfully" ) );
			return true;
		}

	private:
		std::shared_ptr< dependency::c_dependency > m_dependency;
		std::uint32_t m_allocation_size;
		std::uint64_t m_allocation_base;
		std::uint64_t m_allocation_phys;
		std::uint64_t m_entry_point;
	};
}