#pragma once

namespace paging {
	class c_dpm {
	public:
        c_dpm( ) { InitializeSRWLock( &m_srw_lock ); }
        ~c_dpm( ) { }

        SRWLOCK m_srw_lock { };
        std::uint64_t m_base_address = 0;
        std::uint64_t m_pte_address = 0;

        bool create( ) {
            this->m_base_address = reinterpret_cast
                < std::uint64_t >( nt::mm_allocate_independent_pages( page_4kb_size )  );
            if ( !m_base_address ) {
                logging::print( "[c_dpm] create: Could not allocate dpm virtual page." );
                return false;
            }

            this->m_pte_address = g_paging->get_pte_address( m_base_address );
            if ( !m_pte_address ) {
                logging::print( "[c_dpm] create: Could not get PTE address." );
                return false;
            }

            logging::print( "[c_dpm] create_dpm: Created dpm mapping at VA: 0x%llx, PTE phys: 0x%llx",
                m_base_address, m_pte_address );
            return true;
        }

        bool read_process_memory( std::uint64_t virtual_address, void* buffer,
            std::size_t size, std::uint64_t target_cr3 = 0 ) {
			if ( !target_cr3 )
				target_cr3 = g_paging->m_target_cr3;

			std::uint64_t original_cr3 = 0;
            g_paging->swap_context( target_cr3, &original_cr3 );

            auto current_buffer = static_cast< std::uint8_t* >( buffer );
            auto current_va = virtual_address;
            auto remaining = size;

            while ( remaining > 0 ) {
                std::uint32_t page_size = 0;
                auto pa = g_paging->translate_linear( current_va, &page_size );
                if ( !pa ) {
					g_paging->swap_context( target_cr3 );
                    return false;
                }

                auto page_offset = current_va & ( page_size - 1 );
                auto read_size = min( static_cast< std::size_t >( page_size - page_offset ), remaining );

                if ( !read_physical_memory( pa, current_buffer, read_size ) ) {
                    g_paging->swap_context( original_cr3 );
                    return false;
                }

                current_va += read_size;
                current_buffer += read_size;
                remaining -= read_size;
            }

            g_paging->swap_context( original_cr3 );
            return true;
        }

        bool write_process_memory( std::uint64_t virtual_address, void* buffer,
            std::size_t size, std::uint64_t target_cr3 = 0 ) {
            if ( !target_cr3 )
                target_cr3 = g_paging->m_target_cr3;

            std::uint64_t original_cr3 = 0;
            g_paging->swap_context( target_cr3, &original_cr3 );

            auto current_buffer = static_cast< std::uint8_t* >( buffer );
            auto current_va = virtual_address;

            auto remaining = size;
            while ( remaining > 0 ) {
                std::uint32_t page_size = 0;
                auto pa = g_paging->translate_linear( current_va, &page_size );
                if ( !pa ) {
                    g_paging->swap_context( original_cr3 );
                    return false;
                }

                auto page_offset = current_va & ( page_size - 1 );
                auto write_size = min( static_cast< std::size_t >( page_size - page_offset ), remaining );

                if ( !write_physical_memory( pa, current_buffer, write_size ) ) {
                    g_paging->swap_context( original_cr3 );
                    return false;
                }

                current_va += write_size;
                current_buffer += write_size;
                remaining -= write_size;
            }

            g_paging->swap_context( original_cr3 );
            return true;
        }

        bool read_physical_memory( std::uint64_t physical_address, void* buffer, std::size_t size ) {
            if ( !g_paging->is_physical_address_valid( physical_address, size ) )
                return false;

            AcquireSRWLockExclusive( &m_srw_lock );
            auto mapped_va = map_physical_page( physical_address, e_protection::read );
            if ( !mapped_va ) {
                return false;
            }

            nt::memcpy( buffer, reinterpret_cast< void* >( mapped_va ), size );

            unmap_virtual_page( );
            ReleaseSRWLockExclusive( &m_srw_lock );
            return true;
        }

        bool write_physical_memory( std::uint64_t physical_address, void* buffer, std::size_t size ) {
            if ( !g_paging->is_physical_address_valid( physical_address, size ) )
                return false;

            AcquireSRWLockExclusive( &m_srw_lock );
            auto mapped_va = map_physical_page( physical_address, e_protection::write );
            if ( !mapped_va ) {
                return false;
            }

            nt::memcpy( reinterpret_cast< void* >( mapped_va ), buffer, size );

            unmap_virtual_page( );
            ReleaseSRWLockExclusive( &m_srw_lock );
            return true;
        }

        template <typename ret_t = std::uint64_t, typename addr_t = ret_t>
        ret_t read_virtual( addr_t va ) {
            ret_t data{};
            if ( !read_process_memory(
                va,
                &data,
                sizeof( ret_t )
            ) ) return ret_t{};

            return data;
        }

        template <typename addr_t = std::uint64_t, typename value_t>
        bool write_virtual( addr_t va, value_t value ) {
            return write_process_memory(
                va,
                &value,
                sizeof( value_t ) 
            );
        }

        template <typename ret_t = std::uint64_t, typename addr_t = ret_t>
        ret_t read_physical( addr_t va ) {
            ret_t data{};
            if ( !read_physical_memory(
                va,
                &data,
                sizeof( ret_t )
            ) ) return ret_t{};

            return data;
        }

        template <typename addr_t = std::uint64_t, typename value_t>
        bool write_physical( addr_t va, value_t value ) {
            return write_physical_memory(
                va,
                &value,
                sizeof( value_t )
            );
        }

    private:
        std::uint64_t map_physical_page( std::uint64_t physical_address, e_protection protection ) {
            const auto page_offset = physical_address % page_4kb_size;
            const auto page_start_pa = physical_address - page_offset;

            if ( !g_paging->is_physical_address_valid( page_start_pa, page_4kb_size ) )
                return 0;

            pte new_pte{};
            new_pte.hard.present = 1;
            new_pte.hard.pfn = page_start_pa >> 12;
            new_pte.hard.read_write = 1;
            new_pte.hard.user_supervisor = 1;
            new_pte.hard.no_execute = ( protection == e_protection::read_write_execute ) ? 0 : 1;

            auto pte_va = phys::physical_to_virtual( m_pte_address );
            if ( !pte_va ) {
                logging::print( "[c_dpm] map_physical_page: Could not convert PTE phys 0x%llx to virtual", m_pte_address );
                return 0;
            }

            nt::memcpy( reinterpret_cast< void* >( pte_va ), &new_pte, sizeof( pte ) );
            nt::ke_invalidate_all_caches( );

            return m_base_address + page_offset;
        }

        void unmap_virtual_page( ) {
            if ( !m_pte_address )
                return;

            pte empty_pte{};
            nt::memcpy( 
                reinterpret_cast< void* >( phys::physical_to_virtual( m_pte_address ) ),
                &empty_pte, sizeof( pte )
            );

            nt::ke_invalidate_all_caches( );
        }
	};
}