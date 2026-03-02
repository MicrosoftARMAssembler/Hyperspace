#pragma once

namespace paging {
	namespace pth {
		struct hook_state_t {
			std::uint8_t original_bytes[ 64 ]{ };
			std::size_t hook_size{ };
			std::uint64_t trampoline{ };
		};

		constexpr std::uint8_t pth_shellcode[ ]{
			0x50,
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x48, 0x87, 0x04, 0x24,
			0xC3
		};

		constexpr std::size_t shellcode_size = sizeof( pth_shellcode );

		std::size_t calculate_hook_size( std::uint64_t target_pa, std::size_t min_size ) {
			std::uint8_t buffer[ 64 ];
			if ( !dpm::read_physical( target_pa, buffer, sizeof( buffer ) ) ) {
				return 0;
			}

			std::size_t total_len = 0;
			while ( total_len < min_size && total_len < sizeof( buffer ) ) {
				hde64s hde;
				hde64_disasm( &buffer[ total_len ], &hde );
				if ( hde.len == 0 )
					break;
				total_len += hde.len;
			}

			return total_len;
		}

		std::uint64_t create_trampoline( std::uint64_t target_va, const std::uint8_t* original_bytes, std::size_t hook_size ) {
			auto trampoline = reinterpret_cast< std::uint8_t* >( mmu::alloc_page( 0x100 ) );
			if ( !trampoline ) {
				return 0;
			}

			if ( !nt::mm_set_page_protection( reinterpret_cast< std::uint64_t >( trampoline ),
				0x100, PAGE_EXECUTE_READWRITE ) ) {
				mmu::free_page( reinterpret_cast< std::uint64_t >( trampoline ), 0x100 );
				return 0;
			}

			std::memcpy( trampoline, original_bytes, hook_size );

			auto jmp_back = target_va + hook_size;
			auto jmp_pos = &trampoline[ hook_size ];
			jmp_pos[ 0 ] = 0x50;
			jmp_pos[ 1 ] = 0x48;
			jmp_pos[ 2 ] = 0xB8;
			std::memcpy( &jmp_pos[ 3 ], &jmp_back, 8 );
			jmp_pos[ 11 ] = 0x48;
			jmp_pos[ 12 ] = 0x87;
			jmp_pos[ 13 ] = 0x04;
			jmp_pos[ 14 ] = 0x24;
			jmp_pos[ 15 ] = 0xC3;

			return reinterpret_cast< std::uint64_t >( trampoline );
		}

		template < typename org_t = void* >
		bool create( std::uint64_t target_va, void* hook_function, org_t* original_function = nullptr, hook_state_t* out_state = nullptr ) {
			if ( !target_va || !hook_function ) {
				nt::dbg_print( oxorany( "[pth] Invalid parameters\n" ) );
				return false;
			}

			std::uint64_t target_pa = 0;
			if ( !translate_linear( target_va, &target_pa ) ) {
				nt::dbg_print( oxorany( "[pth] Failed to translate VA 0x%llx to PA\n" ), target_va );
				return false;
			}

			std::uint8_t buffer[ 64 ];
			if ( !dpm::read_physical( target_pa, buffer, sizeof( buffer ) ) ) {
				nt::dbg_print( oxorany( "[pth] Failed to read target bytes\n" ) );
				return false;
			}

			auto hook_size = calculate_hook_size( target_pa, shellcode_size );
			if ( hook_size < shellcode_size ) {
				nt::dbg_print( oxorany( "[pth] Hook size too small: %zu (need %zu)\n" ),
					hook_size, shellcode_size );
				return false;
			}

			auto trampoline = create_trampoline( target_va, buffer, hook_size );
			if ( !trampoline ) {
				nt::dbg_print( oxorany( "[pth] Failed to create trampoline\n" ) );
				return false;
			}

			auto hook_addr = reinterpret_cast< std::uint64_t >( hook_function );
			
			std::uint8_t hook_bytes[ 16 ];
			hook_bytes[ 0 ] = 0x50;
			hook_bytes[ 1 ] = 0x48;
			hook_bytes[ 2 ] = 0xB8;
			std::memcpy( &hook_bytes[ 3 ], &hook_addr, 8 );
			hook_bytes[ 11 ] = 0x48;
			hook_bytes[ 12 ] = 0x87;
			hook_bytes[ 13 ] = 0x04;
			hook_bytes[ 14 ] = 0x24;
			hook_bytes[ 15 ] = 0xC3;

			if ( !dpm::write_physical( target_pa, hook_bytes, shellcode_size ) ) {
				nt::dbg_print( oxorany( "[pth] Failed to write hook bytes\n" ) );
				mmu::free_page( trampoline, 0x100 );
				return false;
			}

			if ( hook_size > shellcode_size ) {
				std::uint8_t nops[ 32 ];
				memset( nops, 0x90, sizeof( nops ) );

				std::size_t nop_count = hook_size - shellcode_size;
				if ( !dpm::write_physical( target_pa + shellcode_size, nops, nop_count ) ) {
					nt::dbg_print( oxorany( "[pth] Warning: Failed to write NOP padding\n" ) );
				}
			}

			tlb::flush_caches( target_va );
			nt::ke_invalidate_all_caches( );

			if ( original_function ) {
				*original_function = reinterpret_cast< org_t >( trampoline );
			}

			if ( out_state ) {
				std::memcpy( out_state->original_bytes, buffer, hook_size );
				out_state->hook_size = hook_size;
				out_state->trampoline = trampoline;
			}

			return true;
		}

		bool is_pattern( const std::uint8_t* bytes, std::size_t size ) {
			if ( size < 16 ) {
				return false;
			}

			if ( bytes[ 0 ] == 0x50 && bytes[ 1 ] == 0x48 && bytes[ 2 ] == 0xB8 &&
				bytes[ 11 ] == 0x48 && bytes[ 12 ] == 0x87 &&
				bytes[ 13 ] == 0x04 && bytes[ 14 ] == 0x24 && bytes[ 15 ] == 0xC3 ) {
				return true;
			}

			return false;
		}

		bool is_hooked( std::uint64_t target_va, const hook_state_t* known_state ) {
			if ( !target_va || !known_state || !known_state->hook_size ) {
				return false;
			}

			std::uint64_t target_pa = 0;
			if ( !translate_linear( target_va, &target_pa ) ) {
				return false;
			}

			std::uint8_t current_bytes[ 64 ];
			if ( !dpm::read_physical( target_pa, current_bytes, sizeof( current_bytes ) ) ) {
				return false;
			}

			return memcmp( current_bytes, known_state->original_bytes, known_state->hook_size ) != 0;
		}

		bool remove( std::uint64_t target_va, hook_state_t& state ) {
			if ( !target_va || !state.hook_size ) {
				nt::dbg_print( oxorany( "[pth] Invalid parameters for remove\n" ) );
				return false;
			}

			std::uint64_t target_pa = 0;
			if ( !translate_linear( target_va, &target_pa ) ) {
				nt::dbg_print( oxorany( "[pth] Failed to translate VA for removal\n" ) );
				return false;
			}

			if ( !dpm::write_physical( target_pa, state.original_bytes, state.hook_size ) ) {
				nt::dbg_print( oxorany( "[pth] Failed to restore original bytes\n" ) );
				return false;
			}

			if ( state.trampoline ) {
				mmu::free_page( state.trampoline, 0x100 );
			}

			tlb::flush_caches( target_va );
			nt::ke_invalidate_all_caches( );
			return true;
		}
	}
}