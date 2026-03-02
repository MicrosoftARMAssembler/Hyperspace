#pragma once

namespace execute {
	class c_execute {
	public:
		c_execute( std::shared_ptr< shellcode_t > shellcode_data, shellcode_data_t init_data ) : m_shellcode_data( shellcode_data ) {
			auto data_size = sizeof( shellcode_data_t );
			this->m_data_va = hyperspace::paging::allocate_virtual( data_size );
			if ( !m_data_va ) {
				logging::print( oxorany( "failed to allocate data va" ) );
				return;
			}

			Sleep( 20 );

			//hyperspace::eac::page_memory( m_data_va, data_size );
			if ( !hyperspace::write_virt( m_data_va, &init_data, sizeof( shellcode_data_t ) ) ) {
				logging::print( oxorany( "failed to write initial data" ) );
				return;
			}
		}

		bool compile( ) {
			if ( !m_data_va ) {
				logging::print( oxorany( "could not allocate data" ) );
				return false;
			}


			std::uint8_t dll_main_shellcode[ ] = {
				// reserve da stack space twan, u kno da drill
								0x48, 0x83, 0xEC, 0x38,  // sub rsp, 0x38

								// load addy of data struct at 6
								0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0x00,  // mov rax, struct_addr, patched dat jawn

								//nops for alignment
								0x48, 0x39, 0xFF,  // cmp rdi, rdi, no op
								0x90,              // nop
								0x39, 0xC0,        // cmp eax, eax, no op
								0x90,              // nop

								// save the struct ptr on stack
								0x48, 0x89, 0x44, 0x24, 0x20,  // mov, rsp 0x20, rax

								//// check if we already executed
								//0x48, 0x8B, 0x44, 0x24, 0x20,  // mov rax, rsp 0x20
								//0x83, 0x38, 0x00,              // cmp dword ptr rax, 0
								//0x75, 0x39,                    // jne exit, we skip dat jawn if executed already

								// set status to executing / 1
								//0x48, 0x8B, 0x44, 0x24, 0x20,        // mov rax, rsp 0x20
								//0xC7, 0x00, 0x01, 0x00, 0x00, 0x00,  // mov dword ptr rax, 1

								// get entrypoint addy from struct
								0x48, 0x8B, 0x44, 0x24, 0x20,  // mov rax rsp 0x20
								0x48, 0x8B, 0x40, 0x08,        // mov rax, rax 8, ts is entrypoint
								0x48, 0x89, 0x44, 0x24, 0x28,  // mov rsp 0x28, rax, save for call

								// get params for x64 calling convention bullshit
								0x45, 0x33, 0xC0,  // xor r8d, r8d, lpReserved = null thingy thang
								0xBA, 0x01, 0x00, 0x00,
								0x00,  // mov edx, 1, fdwReason thingy thang

								// load dll base from struct, first param
								0x48, 0x8B, 0x44, 0x24, 0x20,  // mov rax, rsp 0x20
								0x48, 0x8B, 0x48, 0x10,        // mov rcx, rax 0x10, dllbase

								// call entrypoint
								0xFF, 0x54, 0x24, 0x28,  // call qword ptr, rsp 0x28

								// set status to 2 so its status is completed
								0x48, 0x8B, 0x44, 0x24, 0x20,        // mov rax, rsp 0x20
								0xC7, 0x00, 0x02, 0x00, 0x00, 0x00,  // mov dword ptr rax, 2

								// exit, restore stack and return
								0x48, 0x83, 0xC4, 0x38,  // add rsp, 0x38, restore da stack
								0xC3,                    // ret
								0x48, 0x39, 0xC0,        // cmp rax, rax
								0x90,                    // nop
								0xCC                      // int3, bp
			};

			const unsigned long shell_data_offset = 0x6;

			*reinterpret_cast< std::uint64_t* >( &dll_main_shellcode[ shell_data_offset ] ) = m_data_va;


			m_shellcode_data->m_size = sizeof( dll_main_shellcode );
			m_shellcode_data->m_code.resize( m_shellcode_data->m_size );
			std::memcpy( m_shellcode_data->m_code.data( ), dll_main_shellcode, m_shellcode_data->m_size );

			logging::print( oxorany( "shellcode size %zu bytes" ), m_shellcode_data->m_size );
			logging::print( oxorany( "data structure 0x%llx" ), m_data_va );
			logging::print( oxorany( "original function 0x%llx" ), m_shellcode_data->m_original_fn );

			return !m_shellcode_data->m_code.empty( );
		}

		std::uint64_t map( ) {
				auto shellcode_size = m_shellcode_data->m_size;
			this->m_shellcode_va = hyperspace::paging::allocate_virtual( shellcode_size );
			if ( !m_shellcode_va ) return 0;


			if ( !hyperspace::write_virt( m_shellcode_va, m_shellcode_data->m_code.data( ), shellcode_size ) )
				return 0;

			std::vector<std::uint8_t> verify_shellcode( shellcode_size );
			if ( hyperspace::read_virt( m_shellcode_va, verify_shellcode.data( ), shellcode_size ) ) {
				auto remote_last_8 = *reinterpret_cast< std::uint64_t* >(
					verify_shellcode.data( ) + shellcode_size - 8 );
				logging::print( oxorany( "last 8 bytes 0x%llx" ), remote_last_8 );

				logging::print( oxorany( "first 16 bytes:" ) );
				for ( int i = 0; i < 16 && i < shellcode_size; i++ ) {
					printf( "%02X ", verify_shellcode[ i ] );
				}
				printf( "\n" );
			}

			return m_shellcode_va;
		}


		std::uint64_t get_data( ) const {
			return m_data_va;
		}

		bool cleanup( ) {
			if ( !hyperspace::paging::free_virtual( m_shellcode_va ) ) {
				logging::print( oxorany( "[c_execute] unmap: Could not cleanup shellcode" ) );
				return false;
			}

			if ( !hyperspace::paging::free_virtual( m_data_va ) ) {
				logging::print( oxorany( "[c_execute] unmap: Could not cleanup data" ) );
				return false;
			}

			return true;
		}

	private:
		std::shared_ptr< shellcode_t > m_shellcode_data;
		shellcode_data_t m_init_data;
		std::uint64_t m_shellcode_va;
		std::uint64_t m_data_va;
	};
}