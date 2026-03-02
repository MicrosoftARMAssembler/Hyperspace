#include <impl/includes.h>

namespace hyperspace {
	namespace target {
		extern std::uint32_t m_thread_id{ };
		extern std::uint32_t m_process_id{ };
		extern HWND m_window_handle{ };
		extern eprocess_t* m_eprocess{ };
		extern peb_t* m_process_peb{ };
		extern std::uint64_t m_base_address{ };
		std::uint32_t get_thread_id( );
		std::uint32_t get_process_id( std::wstring process_name );
		eprocess_t* get_eprocess( std::uint32_t process_id );
	}

	namespace paging {
		std::unordered_map< std::uint64_t,
			std::pair<pt_entries_t, std::uint64_t> > m_virtual_address_space;
		std::pair< std::uint32_t, std::uint32_t > m_pfn_ranges;
		extern std::uint64_t m_process_cr3{ };
		extern std::uint64_t m_system_cr3{ };
	}

	bool initialize( ) {
		if ( !driver::create_instrumentation( ) )
			return false;

		if ( !driver::create_process( ) ) {
			driver::cleanup( );
			return false;
		}

		if ( !driver::map_instrumentation( ) ) {
			driver::cleanup( );
			return false;
		}

		auto pfn_ranges = driver::get_pfn_ranges( );
		paging::m_pfn_ranges.swap( pfn_ranges );
		return true;
	}

	bool is_active( ) {
		m_data_request->m_request_type = data_type::ping;
		return driver::send_request( );
	}

	void unload( ) {
		if ( !m_data_request ) return;
		m_data_request->m_request_type = data_type::unload_driver;
		driver::send_request( );
		driver::cleanup( );
	}

	void unhook( ) {
		if ( !m_data_request ) return;
		m_data_request->m_request_type = data_type::unload_hook;
		driver::send_request( );
		driver::cleanup( );
	}

	namespace root {
		void poll_logs( ) {
			m_data_request->m_request_type = data_type::update_root;
			driver::send_request( );

			if ( m_data_request->m_message_count > 0 ) {
				for ( auto idx = 0; idx < m_data_request->m_message_count; idx++ ) {
					if ( idx >= m_max_messages ) {
						break;
					}

					logging::print( oxorany( "%s" ), m_root_request[ idx ].m_root_message );
				}

				m_data_request->m_message_count = 0;
			}
		}
	}

	namespace eac {
		bool is_active( ) {
			m_data_request->m_request_type = data_type::eac_ping;
			if ( !driver::send_request( ) )
				return 0;

			return m_data_request->m_status;
		}

		bool write_virtual( std::uint64_t source, void* destination, std::size_t size ) {
			m_data_request->m_request_type = data_type::eac_write_virtual;
			m_data_request->m_process = target::m_eprocess;
			m_data_request->m_address = source;
			m_data_request->m_address2 = destination;
			m_data_request->m_size = size;

			return driver::send_request( );
		}

		bool read_virtual( std::uint64_t source, void* destination, std::size_t size ) {
			m_data_request->m_request_type = data_type::eac_read_virtual;
			m_data_request->m_process = target::m_eprocess;
			m_data_request->m_address = source;
			m_data_request->m_address2 = destination;
			m_data_request->m_size = size;

			return driver::send_request( );
		}

		std::uint64_t allocate_virtual( std::size_t size, std::uint32_t protection ) {
			m_data_request->m_request_type = data_type::eac_allocate_virtual;
			m_data_request->m_process_id = target::m_process_id;
			m_data_request->m_protection = protection;
			m_data_request->m_size = size;

			if ( !driver::send_request( ) )
				return { };

			return m_data_request->m_address;
		}

		bool free_virtual( std::uint64_t allocation_base, std::size_t size ) {
			m_data_request->m_request_type = data_type::eac_free_virtual;
			m_data_request->m_process_id = target::m_process_id;
			m_data_request->m_address = allocation_base;
			m_data_request->m_size = size;

			if ( !driver::send_request( ) )
				return { };

			return m_data_request->m_status;
		}

		bool attach_process( eprocess_t* process, kapc_state_t* apc_state ) {
			m_data_request->m_request_type = data_type::eac_attach_process;
			m_data_request->m_process = target::m_eprocess;

			if ( !driver::send_request( ) )
				return false;

			if ( apc_state )
				*apc_state = m_data_request->m_apc_state;

			return m_data_request->m_status;
		}

		bool detach_process( eprocess_t* process, kapc_state_t apc_state ) {
			m_data_request->m_request_type = data_type::eac_detach_process;
			m_data_request->m_process = target::m_eprocess;
			m_data_request->m_apc_state = apc_state;

			if ( !driver::send_request( ) )
				return false;

			return m_data_request->m_status;
		}

		bool protect_virtual( std::uint64_t& address, std::uint32_t& size, std::uint32_t protection, std::uint32_t* old_protection ) {
			if ( protection & 0xF0 ) {
				logging::print( oxorany( "[c_driver] protect_virtual: Page protection is too high." ) );
				return false;
			}
			
			m_data_request->m_request_type = data_type::eac_protect_virtual;
			m_data_request->m_process = target::m_eprocess;
			m_data_request->m_address = address;
			m_data_request->m_protection = protection;
			m_data_request->m_size = size;

			if ( !driver::send_request( ) )
				return false;

			if ( old_protection )
				*old_protection = m_data_request->m_old_protection;

			address = m_data_request->m_address;
			size = m_data_request->m_size;
			return m_data_request->m_status;
		}

		bool query_virtual( std::uint64_t address, memory_basic_information_t* memory_info ) {
			m_data_request->m_request_type = data_type::eac_query_virtual;
			m_data_request->m_process = target::m_eprocess;
			m_data_request->m_address = address;
			if ( !driver::send_request( ) )
				return false;

			if ( memory_info )
				*memory_info = m_data_request->m_memory_info;

			return m_data_request->m_status;
		}
		
		bool set_information_virtual( std::uint64_t address, std::size_t size ) {
			m_data_request->m_request_type = data_type::eac_set_information;
			m_data_request->m_process = target::m_eprocess;
			m_data_request->m_address = address;
			m_data_request->m_size = size;
			return driver::send_request( );
		}

		bool flush_virtual( std::uint64_t* address, std::size_t* size ) {
			m_data_request->m_request_type = data_type::eac_query_virtual;
			m_data_request->m_process = target::m_eprocess;
			m_data_request->m_address = *address;
			m_data_request->m_size = *size;
			if ( !driver::send_request( ) )
				return false;

			*address = m_data_request->m_address;
			*size = m_data_request->m_size;

			return m_data_request->m_status;
		}

		bool page_memory( std::uint64_t address, std::uint64_t size ) {
			m_data_request->m_request_type = data_type::eac_page_memory;
			m_data_request->m_process_id = target::m_process_id;
			m_data_request->m_address = address;
			m_data_request->m_size = size;

			return driver::send_request( );
		}
	}

	namespace paging {
		enum class allocator_type {
			eac,
			remapping
		};

		struct allocation_data_t {
			allocator_type m_allocator_type;
			std::size_t m_size;
		};

		std::unordered_map< std::uint64_t, allocation_data_t > m_allocations;

		std::uint64_t swap_context( std::uint64_t process_cr3 ) {
			m_data_request->m_request_type = data_type::swap_context;
			m_data_request->m_address = process_cr3;

			if ( !driver::send_request( ) )
				return 0;

			return m_data_request->m_address1;
		}

		bool attach_process( eprocess_t* eprocess, eprocess_t** old_eprocess = nullptr ) {
			m_data_request->m_request_type = data_type::attach_process;
			m_data_request->m_process = eprocess;

			if ( !driver::send_request( ) )
				return false;

			if ( old_eprocess )
				*old_eprocess = m_data_request->m_old_process;

			return true;
		}

		bool detach_process( eprocess_t* old_eprocess ) {
			m_data_request->m_request_type = data_type::detach_process;
			m_data_request->m_old_process = old_eprocess;
			return driver::send_request( );
		}

		bool hyperspace_entries( pt_entries_t& pt_entries, std::uint64_t va ) {
			m_data_request->m_request_type = data_type::hyperspace_entries;
			m_data_request->m_address = va;

			if ( !driver::send_request( ) )
				return false;

			pt_entries = std::move( m_data_request->m_pt_entries );
			return true;
		}

		bool set_page_protection( std::uint64_t va, page_protection protection, bool supervisor = false ) {
			m_data_request->m_request_type = data_type::set_page_protection;
			m_data_request->m_address = va;
			m_data_request->m_page_protection = protection;
			m_data_request->m_user_supervisor = supervisor;

			if ( !driver::send_request( ) )
				return false;

			return true;
		}

		bool is_address_valid( std::uint64_t physical_address ) {
			auto pfn = physical_address >> paging::page_shift;
			return pfn >= m_pfn_ranges.first && pfn <= m_pfn_ranges.second;
		}

		std::uint64_t get_process_cr3( eprocess_t* eprocess ) {
			m_data_request->m_request_type = data_type::get_process_cr3;
			m_data_request->m_process = eprocess;

			if ( !driver::send_request( 5000 ) )
				return { };

			return m_data_request->m_address1;
		}

		bool translate_linear( std::uint64_t va, std::uint64_t* pa ) {
			m_data_request->m_request_type = data_type::translate_linear;
			m_data_request->m_address = va;

			if ( !driver::send_request( ) )
				return { };

			if ( pa )
				*pa = m_data_request->m_address1;
			return *pa ? is_address_valid( *pa ) : true;
		}

		std::uint64_t translate_linear( std::uint64_t va ) {
			auto it = m_virtual_address_space.find( va );
			if ( it != m_virtual_address_space.end( ) ) {
				if ( is_address_valid( it->second.second ) )
					return it->second.second;
			}

			paging::pt_entries_t pt_entries;
			if ( !hyperspace_entries( pt_entries, va ) )
				return 0;

			std::uint64_t pa = 0;
			if ( pt_entries.m_pdpte.hard.page_size ) {
				pa = ( pt_entries.m_pdpte.hard.pfn << paging::page_shift ) + ( va & paging::page_1gb_mask );
			}
			else if ( pt_entries.m_pde.hard.page_size ) {
				pa = ( pt_entries.m_pde.hard.pfn << paging::page_shift ) + ( va & paging::page_2mb_mask );
			}
			else {
				pa = ( pt_entries.m_pte.hard.pfn << paging::page_shift ) + ( va & paging::page_4kb_mask );
			}

			m_virtual_address_space[ va ] = std::make_pair( pt_entries, pa );
			if ( is_address_valid( pa ) )
				return pa;
			return 0;
		}

		std::uint64_t remap_physical_page( std::uint64_t va ) {
			m_data_request->m_request_type = data_type::remap_physical_page;
			m_data_request->m_process = target::m_eprocess;
			m_data_request->m_address = m_process_cr3;
			m_data_request->m_address2 = reinterpret_cast< void* >( va );
			if ( !driver::send_request( ) )
				return 0;

			return m_data_request->m_address1;
		}

		namespace cow {
			bool create_exception( std::uint64_t va ) {
				m_data_request->m_request_type = data_type::cow_create_exception;
				m_data_request->m_process = target::m_eprocess;
				m_data_request->m_address = va;
				return driver::send_request( );
			}
		}

		namespace context {
			bool create_hyperspace( ) {
				m_data_request->m_request_type = data_type::hyperspace_create;
				m_data_request->m_process = target::m_eprocess;
				return driver::send_request( );
			}

			bool allocate_virtual( std::uint32_t size, bool high_address = false ) {
				m_data_request->m_request_type = data_type::hyperspace_allocate;
				m_data_request->m_high_address = high_address;
				m_data_request->m_size = size;

				return driver::send_request( );
			}

			bool switch_to_hyperspace( ) {
				m_data_request->m_request_type = data_type::threads_to_hyperspace;
				m_data_request->m_thread_id = target::m_thread_id;

				return driver::send_request( );
			}

			bool switch_from_hyperspace( ) {
				m_data_request->m_request_type = data_type::threads_from_hyperspace;
				m_data_request->m_thread_id = target::m_thread_id;

				return driver::send_request( );
			}
		}

		namespace dpm {
			bool read_physical( void* dst, std::uint64_t pa, std::size_t size ) {
				m_data_request->m_request_type = data_type::read_physical;
				m_data_request->m_address = pa;
				m_data_request->m_address2 = dst;
				m_data_request->m_size = size;
				return driver::send_request( );
			}

			bool write_physical( void* dst, std::uint64_t pa, std::size_t size ) {
				m_data_request->m_request_type = data_type::write_physical;
				m_data_request->m_address = pa;
				m_data_request->m_address2 = dst;
				m_data_request->m_size = size;
				return driver::send_request( );
			}

			bool read_virtual( void* dst, std::uint64_t pa, std::size_t size ) {
				m_data_request->m_request_type = data_type::read_virtual;
				m_data_request->m_address = pa;
				m_data_request->m_address2 = dst;
				m_data_request->m_size = size;
				return driver::send_request( );
			}

			bool write_virtual( void* dst, std::uint64_t pa, std::size_t size ) {
				m_data_request->m_request_type = data_type::write_virtual;
				m_data_request->m_address = pa;
				m_data_request->m_address2 = dst;
				m_data_request->m_size = size;
				return driver::send_request( );
			}
		}

		namespace ptm {
			void* map_process_page( std::uint64_t pa, page_protection protection, bool supervisor = false ) {
				m_data_request->m_request_type = data_type::map_process_page;
				m_data_request->m_address = pa;
				m_data_request->m_page_protection = protection;
				m_data_request->m_user_supervisor = supervisor;

				if ( !driver::send_request( ) )
					return nullptr;

				return m_data_request->m_address2;
			}

			bool read_physical( void* dst, std::uint64_t pa, std::size_t size ) {
				auto offset_in_page = pa & page_4kb_mask;
				auto bytes_to_read = min( size, ( std::size_t )( page_4kb_size - offset_in_page ) );

				auto mapped_page = map_process_page( pa, page_protection::readwrite, true );
				if ( !mapped_page ) {
					logging::print( oxorany( "[dpm] read_physical: Failed to map page at PA: 0x%llx\n" ), pa );
					return false;
				}

				__movsb(
					reinterpret_cast< std::uint8_t* >( dst ),
					reinterpret_cast< std::uint8_t* >( mapped_page ), 
					bytes_to_read
				);

				return true;
			}

			bool write_physical( std::uint64_t pa, void* dst, std::size_t size ) {
				auto offset_in_page = pa & page_4kb_mask;
				auto bytes_to_read = min( size, ( std::size_t )( page_4kb_size - offset_in_page ) );

				auto mapped_page = map_process_page( pa, page_protection::readwrite, true );
				if ( !mapped_page ) {
					logging::print( oxorany( "[dpm] read_physical: Failed to map page at PA: 0x%llx\n" ), pa );
					return false;
				}

				__movsb(
					reinterpret_cast< std::uint8_t* >( mapped_page ),
					reinterpret_cast< std::uint8_t* >( dst ),
					bytes_to_read
				);

				return true;
			}
		}

		namespace vad {
			std::uint64_t allocate_virtual( std::size_t size ) {
				m_data_request->m_request_type = data_type::vad_allocate_virtual;
				m_data_request->m_process_id = target::m_process_id;
				m_data_request->m_size = size;
				if ( !driver::send_request( ) )
					return false;

				return m_data_request->m_address;
			}
		}

		namespace remap {
			std::uint64_t allocate_virtual( std::size_t size ) {
				auto process_id = target::get_process_id( oxorany( L"csrss.exe" ) );
				if ( !process_id )
					return { };

				auto process = target::get_eprocess( process_id );
				if ( !process )
					return { };

				m_data_request->m_request_type = data_type::remap_allocate_virtual;
				m_data_request->m_address = m_process_cr3;
				m_data_request->m_process = process;
				m_data_request->m_size = size;

				if ( !driver::send_request( ) )
					return { };

				return m_data_request->m_address1;
			}
		}

		std::uint64_t allocate_virtual( std::size_t size, std::uint32_t protection = 0x40 ) {
			const bool use_eac = eac::is_active( );
			const auto allocation_base = use_eac
				? eac::allocate_virtual( size, protection )
				: remap::allocate_virtual( size );
			if ( !allocation_base )
				return { };

			if ( use_eac ) {
				if ( !eac::page_memory( allocation_base, size ) ) {
					return { };
				}
			}

			const auto type = use_eac ? allocator_type::eac : allocator_type::remapping;
			m_allocations.emplace( allocation_base, allocation_data_t{ type, size } );
			logging::print( oxorany( "[c_library] allocate_virtual: Allocated using %s at: 0x%llx" ),
				use_eac ? oxorany( "EAC" ) : oxorany( "Remaping" ), allocation_base );
			return allocation_base;
		}

		bool free_virtual( std::uint64_t allocation_base ) {
			auto it = m_allocations.find( allocation_base );
			if ( it == m_allocations.end( ) )
				return false;

			const auto& allocation_data = it->second;
			bool result = false;
			if ( allocation_data.m_allocator_type == allocator_type::eac ) {
				result = eac::free_virtual( allocation_base, allocation_data.m_size );
			}
			else if ( allocation_data.m_allocator_type == allocator_type::remapping ) {
				result = dpm::write_physical( nullptr, allocation_base, allocation_data.m_size );
			}

			if ( result ) {
				m_allocations.erase( it );
			}

			return result;
		}
	}

	template <typename addr_t>
	bool read_virt( addr_t va, void* buffer, size_t size ) {
		std::uint64_t va64;
		if constexpr ( std::is_pointer_v<addr_t> ) {
			va64 = reinterpret_cast< std::uint64_t >( va );
		}
		else if constexpr ( std::is_integral_v<addr_t> ) {
			va64 = static_cast< std::uint64_t >( va );
		}
		else {
			static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
				"addr_t must be pointer or integral" );
		}

		auto pa = paging::translate_linear( va64 );
		if ( !pa )
			return false;

		if ( !paging::dpm::read_physical( buffer, pa, size ) )
			return false;

		return true;
	}

	template <typename ret_t = std::uint64_t, typename addr_t>
	ret_t read_virt( addr_t va ) {
		std::uint64_t va64;
		if constexpr ( std::is_pointer_v<addr_t> ) {
			va64 = reinterpret_cast< std::uint64_t >( va );
		}
		else if constexpr ( std::is_integral_v<addr_t> ) {
			va64 = static_cast< std::uint64_t >( va );
		}
		else {
			static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
				"addr_t must be pointer or integral" );
		}

		auto pa = paging::translate_linear( va64 );
		if ( !pa )
			return {};

		ret_t buffer{ };
		if ( !paging::dpm::read_physical( &buffer, pa, sizeof( ret_t ) ) )
			return {};
		return buffer;
	}

	template <typename addr_t>
	bool read_phys( addr_t pa, void* buffer, size_t size ) {
		std::uint64_t pa64;
		if constexpr ( std::is_pointer_v<addr_t> ) {
			pa64 = reinterpret_cast< std::uint64_t >( pa );
		}
		else if constexpr ( std::is_integral_v<addr_t> ) {
			pa64 = static_cast< std::uint64_t >( pa );
		}
		else {
			static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
				"addr_t must be pointer or integral" );
		}

		if ( !paging::dpm::read_physical( buffer, pa64, size ) )
			return false;

		return true;
	}

	template <typename ret_t = std::uint64_t, typename addr_t>
	ret_t read_phys( addr_t pa ) {
		std::uint64_t pa64;
		if constexpr ( std::is_pointer_v<addr_t> ) {
			pa64 = reinterpret_cast< std::uint64_t >( pa );
		}
		else if constexpr ( std::is_integral_v<addr_t> ) {
			pa64 = static_cast< std::uint64_t >( pa );
		}
		else {
			static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
				"addr_t must be pointer or integral" );
		}

		ret_t buffer{ };
		if ( !paging::dpm::read_physical( &buffer, pa64, sizeof( ret_t ) ) )
			return {};
		return buffer;
	}

	template <typename addr_t>
	bool write_virt( addr_t va, void* buffer, size_t size ) {
		std::uint64_t va64;
		if constexpr ( std::is_pointer_v<addr_t> ) {
			va64 = reinterpret_cast< std::uint64_t >( va );
		}
		else if constexpr ( std::is_integral_v<addr_t> ) {
			va64 = static_cast< std::uint64_t >( va );
		}
		else {
			static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
				"addr_t must be pointer or integral" );
		}

		auto pa = paging::translate_linear( va64 );
		if ( !pa )
			return false;

		if ( !paging::ptm::write_physical( pa, buffer, size ) )
			return false;

		return true;
	}

	template <typename value_t, typename addr_t>
	bool write_virt( addr_t va, const value_t& value ) {
		std::uint64_t va64;
		if constexpr ( std::is_pointer_v<addr_t> ) {
			va64 = reinterpret_cast< std::uint64_t >( va );
		}
		else if constexpr ( std::is_integral_v<addr_t> ) {
			va64 = static_cast< std::uint64_t >( va );
		}
		else {
			static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
				"addr_t must be pointer or integral" );
		}

		auto pa = paging::translate_linear( va64 );
		if ( !pa )
			return false;

		if ( !paging::ptm::write_physical( pa, const_cast< value_t* >( &value ), sizeof( value_t ) ) )
			return false;

		return true;
	}

	template <typename addr_t>
	bool write_phys( addr_t pa, void* buffer, size_t size ) {
		std::uint64_t pa64;
		if constexpr ( std::is_pointer_v<addr_t> ) {
			pa64 = reinterpret_cast< std::uint64_t >( pa );
		}
		else if constexpr ( std::is_integral_v<addr_t> ) {
			pa64 = static_cast< std::uint64_t >( pa );
		}
		else {
			static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
				"addr_t must be pointer or integral" );
		}

		if ( !paging::ptm::write_physical( pa64, buffer, size ) )
			return false;

		return true;
	}

	template <typename value_t, typename addr_t>
	bool write_phys( addr_t pa, const value_t& value ) {
		std::uint64_t pa64;
		if constexpr ( std::is_pointer_v<addr_t> ) {
			pa64 = reinterpret_cast< std::uint64_t >( pa );
		}
		else if constexpr ( std::is_integral_v<addr_t> ) {
			pa64 = static_cast< std::uint64_t >( pa );
		}
		else {
			static_assert( std::is_pointer_v<addr_t> || std::is_integral_v<addr_t>,
				"addr_t must be pointer or integral" );
		}

		if ( !paging::ptm::write_physical( pa64, const_cast< value_t* >( &value ), sizeof( value_t ) ) )
			return false;

		return true;
	}

	namespace target {
		std::uint32_t get_process_id( std::wstring process_name ) {
			auto snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
			if ( snapshot == INVALID_HANDLE_VALUE )
				return false;

			PROCESSENTRY32W process_entry{ };
			process_entry.dwSize = sizeof( process_entry );
			Process32FirstW( snapshot, &process_entry );
			do {
				if ( !process_name.compare( process_entry.szExeFile ) )
					return process_entry.th32ProcessID;
			} while ( Process32NextW( snapshot, &process_entry ) );

			return 0;
		}

		HWND get_window_handle( std::uint32_t process_pid ) {
			std::pair<HWND, DWORD> params = { 0, process_pid };
			auto result = EnumWindows( [ ]( HWND hwnd, LPARAM lParam ) -> int {
				auto params = ( std::pair<HWND, DWORD>* )( lParam );

				DWORD process_id;
				if ( GetWindowThreadProcessId( hwnd, &process_id ) && process_id == params->second ) {
					SetLastError( -1 );
					params->first = hwnd;
					return false;
				}

				return true;
				}, reinterpret_cast< LPARAM >( &params ) );

			if ( !result && GetLastError( ) == -1 && params.first ) {
				return params.first;
			}

			return 0;
		}

		std::uint32_t get_thread_id( HWND window_handle ) {
			DWORD process_id;
			auto thread_id = ::GetWindowThreadProcessId( window_handle, &process_id );
			return thread_id;
		}

		eprocess_t* get_eprocess( std::uint32_t process_id ) {
			m_data_request->m_request_type = data_type::get_eprocess;
			m_data_request->m_process_id = process_id;

			if ( !driver::send_request( ) )
				return nullptr;

			return m_data_request->m_process;
		}

		std::uint64_t get_base_address( eprocess_t* eprocess ) {
			m_data_request->m_request_type = data_type::get_base_address;
			m_data_request->m_process = eprocess;

			if ( !driver::send_request( ) )
				return 0;

			return reinterpret_cast< std::uint64_t >( m_data_request->m_address2 );
		}

		peb_t* get_process_peb( eprocess_t* eprocess ) {
			m_data_request->m_request_type = data_type::get_process_peb;
			m_data_request->m_process = eprocess;

			if ( !driver::send_request( ) )
				return nullptr;

			return m_data_request->m_process_peb;
		}

		std::uintptr_t get_process_module( const wchar_t* module_name ) {
			if ( !m_process_peb )
				return 0;

			auto peb_data = read_virt< peb_t >( m_process_peb );
			if ( !peb_data.m_ldr )
				return {};

			auto ldr_data = read_virt< peb_ldr_data_t >( peb_data.m_ldr );
			auto current_entry = ldr_data.m_module_list_load_order.m_flink;
			auto first_entry = current_entry;

			do {
				auto entry = read_virt< ldr_data_table_entry_t >( current_entry );
				if ( entry.m_base_dll_name.m_length > 0 && entry.m_base_dll_name.m_length < MAX_PATH * 2 ) {
					wchar_t module_name_buffer[ MAX_PATH ]{};
					if ( read_virt( entry.m_base_dll_name.m_buffer, module_name_buffer, entry.m_base_dll_name.m_length ) ) {
						module_name_buffer[ entry.m_base_dll_name.m_length / 2 ] = L'\0';
						if ( !_wcsicmp( module_name_buffer, module_name ) ) {
							return reinterpret_cast< std::uintptr_t >( entry.m_dll_base );
						}
					}
				}

				current_entry = entry.m_in_load_order_module_list.m_flink;
			} while ( current_entry && current_entry != first_entry );

			return 0;
		}

		std::uintptr_t get_module_export( std::uintptr_t module_base, const char* export_name ) {
			if ( !module_base || !export_name )
				return 0;

			dos_header_t dos_header;
			if ( !read_virt( module_base, &dos_header, sizeof( dos_header_t ) ) )
				return 0;

			if ( !dos_header.is_valid( ) )
				return 0;

			nt_headers_t nt_headers;
			if ( !read_virt( module_base + dos_header.m_lfanew, &nt_headers, sizeof( nt_headers_t ) ) )
				return 0;

			if ( !nt_headers.is_valid( ) )
				return 0;

			export_directory_t export_directory;
			if ( !read_virt( module_base + nt_headers.m_export_table.m_virtual_address, &export_directory, sizeof( export_directory_t ) ) )
				return 0;

			auto function_count = export_directory.m_number_of_functions;
			std::vector<DWORD> functions( function_count );
			if ( !read_virt(
				module_base + export_directory.m_address_of_functions,
				functions.data( ),
				function_count * sizeof( DWORD )
			) )
				return 0;

			if ( functions.empty( ) )
				return 0;

			auto name_count = export_directory.m_number_of_names;
			std::vector<DWORD> names( name_count );
			if ( !read_virt(
				module_base + export_directory.m_address_of_names,
				names.data( ),
				name_count * sizeof( DWORD )
			) )
				return 0;

			if ( names.empty( ) )
				return 0;

			std::vector<WORD> ordinals( name_count );
			if ( !read_virt(
				module_base + export_directory.m_address_of_names_ordinals,
				ordinals.data( ),
				name_count * sizeof( WORD )
			) )
				return 0;

			if ( ordinals.empty( ) )
				return 0;

			for ( auto idx = 0; idx < name_count; idx++ ) {
				char name_buffer[ 256 ] = { 0 };
				if ( !read_virt( module_base + names[ idx ], name_buffer, sizeof( name_buffer ) - 1 ) )
					continue;

				if ( strcmp( name_buffer, export_name ) == 0 ) {
					auto ordinal = ordinals[ idx ];
					if ( ordinal >= function_count ) {
						return 0;
					}

					return module_base + functions[ ordinal ];
				}
			}

			return 0;
		}

		bool benchmark_read( std::chrono::milliseconds duration = std::chrono::seconds( 1 ) ) {
			if ( !m_base_address )
				return false;

			std::uint64_t call_count = 0;
			LARGE_INTEGER freq, start, end;
			QueryPerformanceFrequency( &freq );
			QueryPerformanceCounter( &start );

			auto duration_ms = duration.count( );
			auto test_end = start.QuadPart + ( freq.QuadPart * duration_ms / 1000 );

			auto pa = paging::translate_linear( m_base_address );
			if ( !pa )
				return false;

			while ( true ) {
				QueryPerformanceCounter( &end );
				if ( end.QuadPart >= test_end )
					break;
				is_active( );
				call_count++;
			}

			auto actual_duration = static_cast< double >( end.QuadPart - start.QuadPart ) / freq.QuadPart;
			auto reads_per_sec = static_cast< double >( call_count ) / actual_duration;

			logging::print( oxorany( "[c_driver] benchmark_read: Completed %llu calls in %.6f seconds" ), call_count, actual_duration );
			logging::print( oxorany( "[c_driver] benchmark_read: Rate: %.2f reads per second" ), reads_per_sec );
			return true;
		}

		bool attach_process( const wchar_t* process_name ) {
			logging::print( oxorany( "[c_driver] attach_process: Seaching for %ls" ), process_name );

			while ( !m_process_id ) {
				m_process_id = get_process_id( process_name );
				Sleep( 5 );
			}

			logging::print( oxorany( "[c_driver] attach_process: Found Process PID: 0x%x" ), m_process_id );
			std::getchar( );

			m_window_handle = get_window_handle( m_process_id );
			if ( !m_window_handle ) {
				logging::print( oxorany( "[c_driver] attach_process: Could not get HWND." ) );
				return false;
			}

			logging::print( oxorany( "[c_driver] attach_process: Found Window Handle: 0x%llx" ), m_window_handle );
			std::getchar( );

			m_thread_id = get_thread_id( m_window_handle );
			if ( !m_thread_id ) {
				logging::print( oxorany( "[c_driver] attach_process: Could not get Thread TID." ) );
				return false;
			}

			logging::print( oxorany( "[c_driver] attach_process: Found Thread TID: 0x%x" ), m_thread_id );
			std::getchar( );

			m_eprocess = get_eprocess( m_process_id );
			if ( !m_eprocess ) {
				logging::print( oxorany( "[c_driver] attach_process: Could not get EProcess." ) );
				return false;
			}

			logging::print( oxorany( "[c_driver] attach_process: Found EProcess: 0x%llx" ), m_eprocess );
			std::getchar( );

			paging::m_process_cr3 = paging::get_process_cr3( m_eprocess );
			if ( !paging::m_process_cr3 ) {
				logging::print( oxorany( "[c_driver] attach_process: Could not get Process CR3." ) );
				return false;
			}

			paging::m_system_cr3 = paging::swap_context( paging::m_process_cr3 );
			logging::print( oxorany( "[c_driver] attach_process: Found System CR3: 0x%llx" ), paging::m_system_cr3 );
			logging::print( oxorany( "[c_driver] attach_process: Found Process CR3: 0x%llx" ), paging::m_process_cr3 );
			std::getchar( );

			m_process_peb = get_process_peb( m_eprocess );
			if ( !m_process_peb ) {
				logging::print( oxorany( "[c_driver] attach_process: Could not get Process PEB." ) );
				return false;
			}

			logging::print( oxorany( "[c_driver] attach_process: Found Process PEB: 0x%llx" ), m_process_peb );
			std::getchar( );

			m_base_address = get_base_address( m_eprocess );
			if ( !m_base_address ) {
				logging::print( oxorany( "[c_driver] attach_process: Could not get base address." ) );
				return false;
			}

			logging::print( oxorany( "[c_driver] attach_process: Found Base Address: 0x%llx" ), m_base_address );
			std::getchar( );

			root::poll_logs( );
			std::getchar( );

			return true;
		}

		void detach_process( ) {
			m_process_id = 0;
			m_eprocess = nullptr;
			m_process_peb = nullptr;
			m_base_address = 0;
			paging::swap_context( paging::m_system_cr3 );
		}
	}
}