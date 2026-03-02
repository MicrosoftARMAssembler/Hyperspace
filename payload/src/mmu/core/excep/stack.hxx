#pragma once

namespace excep {
	extern"C" std::uint64_t get_exception_context( ) {
		bool ke_stall_execution_processor = false,
			ke_freeze_execution = false,
			kdp_report = false,
			kdp_trap = false,
			kd_trap = false,
			ki_dispatch_exception = false;

		nt::dbg_print( oxorany( "[excep] calling exception context\n" ) );

		return mmu::traverse_stack( [ & ] ( void** current_stack ) -> bool {
			auto stack_address = *reinterpret_cast< std::uint64_t* >( current_stack );
			if ( !stack_address )
				return false;

			if ( !ke_stall_execution_processor ) {
				if ( stack_address > nt::m_pdb_symbols.m_ke_stall_execution_processor + 0x50 &&
					stack_address - nt::m_pdb_symbols.m_ke_stall_execution_processor < 0x200 )
					ke_stall_execution_processor = true;
				return false;
			}

			if ( !ke_freeze_execution ) {
				if ( stack_address > nt::m_pdb_symbols.m_ke_freeze_execution + 0x50 &&
					stack_address - nt::m_pdb_symbols.m_ke_freeze_execution < 0x200 )
					ke_freeze_execution = true;
				return false;
			}

			if ( !kdp_report ) {
				if ( stack_address > nt::m_pdb_symbols.m_kdp_report + 0x50 &&
					stack_address - nt::m_pdb_symbols.m_kdp_report < 0x150 )
					kdp_report = true;
				return false;
			}

			if ( !kdp_trap ) {
				if ( stack_address > nt::m_pdb_symbols.m_kdp_trap + 0x50 &&
					stack_address - nt::m_pdb_symbols.m_kdp_trap < 0x250 )
					kdp_trap = true;
				return false;
			}

			if ( !kd_trap ) {
				if ( stack_address > nt::m_pdb_symbols.m_kd_trap &&
					stack_address - nt::m_pdb_symbols.m_kd_trap < 0x50 )
					kd_trap = true;
				return false;
			}

			if ( !ki_dispatch_exception ) {
				if ( stack_address > nt::m_pdb_symbols.m_ki_dispatch_exception + 0x100 &&
					stack_address - nt::m_pdb_symbols.m_ki_dispatch_exception < 0x400 )
					ki_dispatch_exception = true;
				return false;
			}

			return true;
			}, address_of_return, true );
	}

	extern "C" std::uint64_t get_exception_record( ) {
		std::uint64_t exception_record = 0;

		auto corrupt_r12 = get_r12( );
		auto corrupt_context = get_r13( );

		bool exception_context_found = false;
		bool corrupt_context_found = false;

		auto result = mmu::traverse_stack( [ & ] ( void** current_stack )-> bool {
			auto stack_address = *reinterpret_cast< std::uint64_t* >( current_stack );

			if ( !corrupt_context_found ) {
				if ( stack_address > nt::m_pdb_symbols.m_ke_freeze_execution &&
					stack_address - nt::m_pdb_symbols.m_ke_freeze_execution < 0x200 ) {
					if ( nt::get_os_version( ) <= 18363 ) {
						__writecr8( *reinterpret_cast< std::uint64_t* >( current_stack - 4 * 8 ) );
					}
					else {
						if ( corrupt_context ) {
							if ( ( *reinterpret_cast< std::uint64_t* >( current_stack - 3 * 8 ) & ~0xff ) != corrupt_context )
								return false;

							corrupt_context = *reinterpret_cast< std::uint64_t* >( current_stack - 3 * 8 );
						}

						__writecr8( *reinterpret_cast< std::uint64_t* >( current_stack + 0x10 ) );
					}

					corrupt_r12 = *reinterpret_cast< std::uint64_t* >( current_stack - 2 * 8 );
					corrupt_context_found = true;
				}

				return false;
			}

			if ( !exception_context_found ) {
				if ( stack_address > nt::m_pdb_symbols.m_kd_enter_debugger &&
					stack_address - nt::m_pdb_symbols.m_kd_enter_debugger < 0x100 ) {

					if ( *reinterpret_cast< std::uint8_t* >( stack_address - 5 ) == 0xE8 &&
						*reinterpret_cast< std::uint32_t* >( stack_address ) == 0x48f08a44 ) {

						exception_record = *reinterpret_cast< std::uint64_t* >( current_stack - 3 * 8 );
						if ( exception_record ) {
							exception_context_found = true;

							if ( !corrupt_context )
								corrupt_context = *reinterpret_cast< std::uint64_t* >( current_stack + 3 * 8 );
							return true;
						}
					}
				}

				return false;
			}

			return true;
			}, address_of_return, true );

		if ( !result || !corrupt_context ) {
			nt::ke_bug_check( 0 );
		}

		set_r13( corrupt_context );
		set_r12( corrupt_r12 );

		return exception_record;
	}
}