#pragma once

/*
	KdTrap Exception Handler
	I haven't finished this yet, but the idea is to catch exceptions in kernel mode caused by PatchGuard
	I've only tested this in a VM so far, so use at your own risk
*/

namespace excep {
	extern "C" std::uint64_t kd_trap_org = 0;
	extern "C" void kd_trap_hk( std::uint64_t );

	extern "C" std::uint64_t get_r12( );
	extern "C" void set_r12( std::uint64_t );
	extern "C" std::uint64_t get_r13( );
	extern "C" void set_r13( std::uint64_t );

	extern "C" void* callout_return( context_return_t* context );
	extern "C" void ke_bug_check( std::uint32_t code ) {
		nt::ke_bug_check( code );	
	}

	void continue_exception( context_t* context, std::uint64_t rip ) {
		context_return_t context_ret{ };
		context_ret.m_stack = reinterpret_cast< void* >( context->m_rsp );
		context_ret.m_function = rip;
		context_ret.m_rax = context->m_rax;

		context_ret.m_rcx = context->m_rcx;
		context_ret.m_rdx = context->m_rdx;
		context_ret.m_r8 = context->m_r8;
		context_ret.m_r9 = context->m_r9;

		context_ret.m_r12 = context->m_r12;
		context_ret.m_r13 = context->m_r13;
		context_ret.m_r14 = context->m_r14;
		context_ret.m_r15 = context->m_r15;

		context_ret.m_rdi = context->m_rdi;
		context_ret.m_rsi = context->m_rsi;
		context_ret.m_rbx = context->m_rbx;
		context_ret.m_rbp = context->m_rbp;

		context_ret.m_rflags = context->m_eflags;

		callout_return( &context_ret );
		nt::ke_bug_check( 0 );
	}

	extern"C" void exception_handler( exception_record_t* exception_record, context_t* context ) {
		nt::dbg_print( oxorany( "[excep] calling exception callback for exception\n" ) );

		if ( !exception_record || !context )
			nt::ke_bug_check( 0 );

		auto exception_address = reinterpret_cast< std::uint64_t >( exception_record->m_exception_address );
		if ( exception_address != context->m_rip )
			nt::ke_bug_check( 0 );

		if ( exception_address > nt::m_pdb_symbols.m_kdp_trap &&
			exception_address - nt::m_pdb_symbols.m_kdp_trap < 0x250 )
			nt::ke_bug_check( 0 );

		//if ( m_exception_callback( exception_record, context ) ) {
		//	continue_exception( context, context->m_rip );
		//	nt::ke_bug_check( 0 );
		//}

		context_return_t ret_context{ };
		ret_context.m_rax = 0;

		bool rbx_found = false,
			non_volatile_reg_found = false,
			stack_found = false,
			rflags_found = false;

		auto result = mmu::traverse_stack( [ & ] ( void** current_stack ) {
			auto stack_current = *reinterpret_cast< std::uint64_t* >( current_stack );

			if ( !rflags_found ) {
				if ( stack_current > nt::m_pdb_symbols.m_kd_enter_debugger &&
					stack_current - nt::m_pdb_symbols.m_kd_enter_debugger < 0x100 ) {
					if ( *reinterpret_cast< std::uint8_t* >( stack_current - 5 ) == 0xE8 &&
						*reinterpret_cast< std::uint32_t* >( stack_current ) == 0x48f08a44 ) {
						ret_context.m_rflags = *reinterpret_cast< std::uint64_t* >( current_stack - 0x8 );
						rflags_found = true;
					}
				}

				return false;
			}

			if ( !non_volatile_reg_found ) {
				if ( stack_current > nt::m_pdb_symbols.m_kdp_trap &&
					stack_current - nt::m_pdb_symbols.m_kdp_trap < 0x250 ) {

					ret_context.m_rbp = *reinterpret_cast< std::uint64_t* >( current_stack + 0x10 );
					ret_context.m_rsi = *reinterpret_cast< std::uint64_t* >( current_stack + 0x18 );
					ret_context.m_rdi = *reinterpret_cast< std::uint64_t* >( current_stack + 0x20 );

					ret_context.m_r13 = *reinterpret_cast< std::uint64_t* >( current_stack - 0x8 );
					ret_context.m_r14 = *reinterpret_cast< std::uint64_t* >( current_stack - 0x10 );
					ret_context.m_r15 = *reinterpret_cast< std::uint64_t* >( current_stack - 0x18 );

					ret_context.m_r12 = get_r12( );
					non_volatile_reg_found = true;
				}

				return false;
			}

			if ( !rbx_found ) {
				if ( stack_current > nt::m_pdb_symbols.m_kdp_trap &&
					stack_current - nt::m_pdb_symbols.m_kdp_trap < 0x250 ) {

					if ( nt::mm_is_address_valid( *reinterpret_cast< void** >( current_stack + 8 ) ) ) {
						ret_context.m_rbx = *reinterpret_cast< std::uint64_t* >( current_stack + 8 );
						rbx_found = true;
					}
				}
				return false;
			}

			if ( !stack_found ) {
				if ( stack_current > nt::m_pdb_symbols.m_ki_dispatch_exception + 0x100 &&
					stack_current - nt::m_pdb_symbols.m_ki_dispatch_exception < 0x400 ) {
					ret_context.m_function = stack_current;
					ret_context.m_stack = reinterpret_cast< void* >( current_stack + 8 );
					stack_found = true;
				}
				return false;
			}

			return true;
			}, address_of_return, true );

		if ( !result )
			nt::ke_bug_check( 0 );

		callout_return( &ret_context );
		nt::ke_bug_check( 0 );
	}

	bool initialize( ) {
		// KiDispatchException -> KdTrap -> KdpDebugRoutineSelect -> KdpTrap
		*reinterpret_cast< std::uint32_t* >( nt::m_pdb_symbols.m_kdp_debug_routine_select ) = 1;
		nt::dbg_print( oxorany( "[exception] m_kdp_debug_routine_select -> 0x%x\n" ),
			*reinterpret_cast< std::uint32_t* >( nt::m_pdb_symbols.m_kdp_debug_routine_select ) );

		// KdpTrap -> KdpReport -> NtGlobalFlag -> KdEnterDebugger -> KeFreezeExecution
		*reinterpret_cast< std::uint32_t* >( nt::m_pdb_symbols.m_nt_global_flag ) = 1;
		nt::dbg_print( oxorany( "[exception] m_nt_global_flag -> 0x%x\n" ),
			*reinterpret_cast< std::uint32_t* >( nt::m_pdb_symbols.m_nt_global_flag ) );

		// KeFreezeExecution -> KdDebuggerLock -> KeStallExecutionProcessor
		*reinterpret_cast< std::uint32_t* >( nt::m_pdb_symbols.m_kd_debugger_lock ) = 1;
		nt::dbg_print( oxorany( "[exception] m_kd_debugger_lock -> 0x%x\n" ),
			*reinterpret_cast< std::uint32_t* >( nt::m_pdb_symbols.m_kd_debugger_lock ) );

		auto halp_stall_counter = nt::m_pdb_symbols.m_halp_stall_counter;
		if ( !halp_stall_counter ) return false;

		// KeStallExecutionProcessor -> (*(__int64 (__fastcall **)(__int64))(v2 + 0x70))(v5)
		auto kd_trap = reinterpret_cast< std::uint64_t* >( halp_stall_counter + 0x70 );
		if ( !kd_trap ) return false;

		kd_trap_org = *kd_trap;
		*kd_trap = reinterpret_cast< std::uint64_t >( kd_trap_hk );

		nt::dbg_print( oxorany( "[exception] installed hook\n" ) );
		return true;
	}
}