#pragma once

namespace paging {
    class c_paging {
    public:
        c_paging( ) { }
        ~c_paging( ) { }

        std::uint64_t m_target_cr3{ };
        std::uint64_t m_physical_mask{ };

        read_func_t m_read_physical = [ ]( std::uint64_t address, void* buffer,
            std::size_t size ) -> bool {
                return g_driver->read_physical_memory( address, buffer, size );
            };

        write_func_t m_write_physical = [ ]( std::uint64_t address, void* buffer,
            std::size_t size ) -> bool {
                return g_driver->write_physical_memory( address, buffer, size );
            };

        void init_mask( ) {
            int regs[ 4 ]{};
            __cpuid( regs, 0x80000000 );
            unsigned max_ext = static_cast< unsigned >( regs[ 0 ] );
            unsigned phys_bits = 36;
            if ( max_ext >= 0x80000008 ) {
                __cpuid( regs, 0x80000008 );
                unsigned b = regs[ 0 ] & 0xFF;
                if ( b >= 32 && b <= 52 )
                    phys_bits = b;
            }

            m_physical_mask = ( phys_bits >= 63 ) ? ~0ULL : ( ( 1ULL << phys_bits ) - 1ULL );
        }

        bool is_physical_address_valid( std::uint64_t pa, std::size_t size ) {
            if ( ( pa & ~m_physical_mask ) != 0 )
                return false;

            const auto end_pa = pa + ( size - 1 );
            if ( ( end_pa & ~m_physical_mask ) != 0 )
                return false;

            return true;
        }

        std::uint64_t get_system_cr3( ) {
            auto page_buffer = std::make_unique<uint8_t[ ]>( 0x1000 );
            for ( auto page = 0; page < 0x200000000; page += 0x1000 ) {
                if ( !g_driver->read_physical_memory( page, page_buffer.get( ), 0x1000 ) )
                    continue;

                if ( page_buffer[ 0 ] == 0xE9 && page_buffer[ 1 ] == 0x4D &&
                    page_buffer[ 2 ] == 0x06 && page_buffer[ 3 ] == 0x00 ) {
                    return *( std::uint64_t* )( page_buffer.get( ) + 0xA0 );
                }
            }
        }

        std::uint64_t get_process_cr3( std::uint64_t module_base ) {
            MEMORYSTATUSEX mem_status{ };
            mem_status.dwLength = sizeof( mem_status );
            GlobalMemoryStatusEx( &mem_status );

            const auto total_pages = mem_status.ullTotalPhys >> 12;
            const auto max_threads = max( 1u, std::thread::hardware_concurrency( ) );
            const auto pages_per_thread = total_pages / max_threads;

            virt_addr_t virt_base{ .value = module_base };

            std::atomic<std::uint64_t> target_cr3{ 0 };
            std::atomic<bool> found{ false };
            std::vector<std::thread> threads;

            const auto start_time = std::chrono::high_resolution_clock::now( );
            for ( auto idx = 0; idx < max_threads; ++idx ) {
                const auto start = idx * pages_per_thread;
                const auto end = ( idx == max_threads - 1 ) ? total_pages : ( idx + 1 ) * pages_per_thread;

                threads.emplace_back( [ =, &target_cr3, &found, &virt_base ] {
                    this->thread_worker( start, end, virt_base, target_cr3, found );
                    } );
            }

            for ( auto& t : threads )
                t.join( );

            const auto end_time = std::chrono::high_resolution_clock::now( );
            const auto duration = duration_cast< std::chrono::milliseconds >( end_time - start_time ).count( );
            logging::print( oxorany( "[c_paging] get_process_cr3: Found CR3: 0x%llx (%d ms)" ), target_cr3.load( ), duration );

            return target_cr3.load( );
        }

        void swap_context( std::uint64_t new_cr3, std::uint64_t* old_cr3 = nullptr ) {
            if ( old_cr3 )
                *old_cr3 = m_target_cr3;
            m_target_cr3 = new_cr3;
        }

        bool swap_context( eprocess_t* target_process, std::uint64_t* old_cr3 = nullptr ) {
            auto new_cr3 = process::get_directory_table_base( target_process );
            if ( !new_cr3 )
                return false;

            if ( old_cr3 )
                *old_cr3 = m_target_cr3;
            m_target_cr3 = new_cr3;
        }

        std::uint64_t translate_linear( std::uint64_t virtual_address, std::uint32_t* page_size = 0 ) {
            auto pdpt_entry = get_pdpte( virtual_address );
            if ( !pdpt_entry || !pdpt_entry->hard.present )
                return 0;

            if ( pdpt_entry->hard.page_size ) {
                if ( page_size ) *page_size = page_1gb_size;
                return ( pdpt_entry->hard.pfn << 12 ) + ( virtual_address & page_1gb_mask );
            }

            auto pd_entry = get_pde( virtual_address );
            if ( !pd_entry || !pd_entry->hard.present )
                return 0;

            if ( pd_entry->hard.page_size ) {
                if ( page_size ) *page_size = page_2mb_size;
                return ( pd_entry->hard.pfn << 12 ) + ( virtual_address & page_2mb_mask );
            }

            auto pt_entry = get_pte( virtual_address );
            if ( !pt_entry || !pt_entry->hard.present )
                return 0;

            if ( page_size ) *page_size = page_4kb_size;
            return ( pt_entry->hard.pfn << 12 ) + ( virtual_address & page_4kb_mask );
        }

        std::uint64_t get_pte_address( std::uint64_t virtual_address ) {
            auto pdpt_entry = get_pdpte( virtual_address );
            if ( !pdpt_entry || !pdpt_entry->hard.present )
                return 0;

            if ( pdpt_entry->hard.page_size ) {
                logging::print( oxorany( "[c_paging] get_pte_address: 1GB Large Pages are not supported!\n" ) );
                return 0;
            }

            auto pd_entry = get_pde( virtual_address );
            if ( !pd_entry || !pd_entry->hard.present )
                return 0;

            if ( pd_entry->hard.page_size ) {
                logging::print( oxorany( "[c_paging] get_pte_address: 2MB Large Pages are not supported!\n" ) );
                return 0;
            }

            _virt_addr_t va{ virtual_address };
            return ( pd_entry->hard.pfn << 12 ) + ( va.pte_index * sizeof( pte ) );
        }

        void spoof_range( std::uintptr_t virtual_address, std::uint32_t size ) {
            const auto pages = ( size + 0xFFF ) >> 12;
            for ( auto i = 0ull; i < pages; ++i ) {
                const auto current_page = virtual_address + i * 0x1000;
                virt_addr_t current_va{ current_page };

                auto pdpt_entry = get_pdpte( current_page );
                if ( pdpt_entry && pdpt_entry->hard.present ) {
                    logging::print( "[c_dpm] spoof_range: pdpt entry: %llx", pdpt_entry->value );

                    pde existing_pde;
                    auto pd_phys = pdpt_entry->hard.pfn << 12;
                    auto pde_phys = pd_phys + ( current_va.pde_index * sizeof( pde ) );
                    if ( m_read_physical( pde_phys, &existing_pde, sizeof( pde ) ) ) {
                        logging::print( "[c_dpm] spoof_range: Existing pde: %llx", existing_pde.value );

                        auto new_pde = existing_pde;
                        new_pde.hard.present = 1;
                        new_pde.hard.read_write = 1;
                        new_pde.hard.user_supervisor = 1;
                        new_pde.hard.no_execute = 0;
                        if ( m_write_physical( pde_phys, &new_pde, sizeof( pde ) ) ) {
                            logging::print( "[c_dpm] spoof_range: Overwrited PDE: %llx", pde_phys );
                        }
                    }
                }

                auto pd_entry = get_pde( current_page );
                if ( pd_entry && pd_entry->hard.present ) {
                    logging::print( "[c_dpm] spoof_range: pd entry: %llx", pd_entry->value );

                    pte existing_pte;
                    auto pt_phys = pd_entry->hard.pfn << 12;
                    auto pte_phys = pt_phys + ( current_va.pte_index * sizeof( pte ) );
                    if ( m_read_physical( pte_phys, &existing_pte, sizeof( pte ) ) ) {
                        logging::print( "[c_dpm] spoof_range: existing pte: %llx", existing_pte.value );

                        pte new_pte = existing_pte;
                        new_pte.hard.read_write = 1;
                        new_pte.hard.user_supervisor = 1;
                        new_pte.hard.no_execute = 0;
                        if ( m_write_physical( pte_phys, &new_pte, sizeof( pte ) ) ) {
                            logging::print( "[c_dpm] spoof_range: Overwrited PTE: %llx", pte_phys );
                        }
                    }
                }
            }
        }

        bool map_page_tables( std::uint64_t base_va, std::size_t page_count ) {
            const auto page_mask = page_4kb_size - 1;
            const auto aligned_size = ( page_count + page_mask ) & ~page_mask;
            if ( !aligned_size ) {
                return false;
            }

            const auto page_size = page_4kb_size;
            for ( std::uint64_t current_va = base_va; current_va < base_va + aligned_size; current_va += page_size ) {
                virt_addr_t va{ current_va };

                pml4e pml4_entry;
                if ( !m_read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
                    &pml4_entry, sizeof( pml4e ) ) )
                    return false;

                if ( !pml4_entry.hard.present ) {
                    auto pdpt_va = nt::mm_allocate_independent_pages( page_4kb_size );
                    if ( !pdpt_va ) return false;
                    nt::memset( pdpt_va, 0, page_4kb_size );

                    auto pdpt_pa = phys::virtual_to_physical(
                        reinterpret_cast< std::uint64_t >( pdpt_va ) );
                    if ( !pdpt_pa ) {
                        return false;
                    }

                    pml4e new_pml4e{ 0 };
                    new_pml4e.hard.present = 1;
                    new_pml4e.hard.read_write = 1;
                    new_pml4e.hard.user_supervisor = 1;
                    new_pml4e.hard.no_execute = 0;
                    new_pml4e.hard.pfn = pdpt_pa >> page_shift;

                    auto pml4_phys = m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) );
                    if ( !m_write_physical( pml4_phys, &new_pml4e, sizeof( pml4e ) ) ) {
                        return false;
                    }

                    nt::flush_caches( pdpt_va );
                    pml4_entry = new_pml4e;
                }

                pdpte pdpt_entry;
                if ( !m_read_physical(
                    ( pml4_entry.hard.pfn << 12 ) + ( sizeof( pdpte ) * va.pdpte_index ),
                    &pdpt_entry, sizeof( pdpte ) ) )
                    return false;

                if ( !pdpt_entry.hard.present ) {
                    auto pd_va = nt::mm_allocate_independent_pages( page_4kb_size );
                    if ( !pd_va ) return false;
                    nt::memset( pd_va, 0, page_4kb_size );

                    auto pd_pa = phys::virtual_to_physical(
                        reinterpret_cast< std::uint64_t >( pd_va ) );
                    if ( !pd_pa ) {
                        return false;
                    }

                    pdpte new_pdpte{ 0 };
                    new_pdpte.hard.present = 1;
                    new_pdpte.hard.read_write = 1;
                    new_pdpte.hard.user_supervisor = 1;
                    new_pdpte.hard.no_execute = 0;
                    new_pdpte.hard.pfn = pd_pa >> page_shift;

                    auto pdpt_phys = ( pml4_entry.hard.pfn << 12 ) + ( va.pdpte_index * sizeof( pdpte ) );
                    if ( !m_write_physical( pdpt_phys, &new_pdpte, sizeof( pdpte ) ) ) {
                        return false;
                    }

                    nt::flush_caches( pd_va );
                    pdpt_entry = new_pdpte;
                }

                pde pd_entry;
                if ( !m_read_physical(
                    ( pdpt_entry.hard.pfn << 12 ) + ( sizeof( pde ) * va.pde_index ),
                    &pd_entry, sizeof( pde ) ) )
                    return false;

                if ( !pd_entry.hard.present ) {
                    auto pt_va = nt::mm_allocate_independent_pages( page_4kb_size );
                    if ( !pt_va ) return false;
                    nt::memset( pt_va, 0, page_4kb_size );

                    auto pt_pa = phys::virtual_to_physical(
                        reinterpret_cast< std::uint64_t >( pt_va ) );
                    if ( !pt_pa ) {
                        return false;
                    }

                    pde new_pde{ 0 };
                    new_pde.hard.present = 1;
                    new_pde.hard.read_write = 1;
                    new_pde.hard.user_supervisor = 1;
                    new_pde.hard.no_execute = 0;
                    new_pde.hard.pfn = pt_pa >> page_shift;

                    auto pd_phys = ( pdpt_entry.hard.pfn << 12 ) + ( va.pde_index * sizeof( pde ) );
                    if ( !m_write_physical( pd_phys, &new_pde, sizeof( pde ) ) ) {
                        return false;
                    }

                    nt::flush_caches( pt_va );
                    pd_entry = new_pde;
                }

                pte pt_entry{ };
                if ( !m_read_physical( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
                    &pt_entry, sizeof( pte ) ) )
                    return false;

                if ( pt_entry.hard.present ) {
                    return false;
                }

                auto page_va = nt::mm_allocate_independent_pages( page_4kb_size );
                if ( !page_va ) return false;
                nt::memset( page_va, 0, page_4kb_size );

                auto page_pa = phys::virtual_to_physical(
                    reinterpret_cast< std::uint64_t >( page_va ) );
                if ( !page_pa ) {
                    return false;
                }

                pte new_pte{ 0 };
                new_pte.hard.present = 1;
                new_pte.hard.read_write = 1;
                new_pte.hard.user_supervisor = 1;
                new_pte.hard.no_execute = 0;
                new_pte.hard.pfn = page_pa >> page_shift;

                auto pt_phys = ( pd_entry.hard.pfn << 12 ) + ( va.pte_index * sizeof( pte ) );
                if ( !m_write_physical( pt_phys, &new_pte, sizeof( pte ) ) ) {
                    return false;
                }

                nt::flush_caches( page_va );
            }

            return true;
        }

        bool map_page_entries( std::uint64_t base_va, std::size_t page_count ) {
            const auto page_mask = page_4kb_size - 1;
            const auto aligned_size = ( page_count + page_mask ) & ~page_mask;
            if ( !aligned_size ) {
                return false;
            }

            const auto page_size = page_4kb_size;
            for ( std::uint64_t current_va = base_va; current_va < base_va + aligned_size; current_va += page_size ) {
                virt_addr_t va{ current_va };

                pml4e pml4_entry;
                if ( !m_read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
                    &pml4_entry, sizeof( pml4e ) ) )
                    return false;

                if ( !pml4_entry.hard.present ) {
                    return false;
                }

                pdpte pdpt_entry;
                if ( !m_read_physical(
                    ( pml4_entry.hard.pfn << 12 ) + ( sizeof( pdpte ) * va.pdpte_index ),
                    &pdpt_entry, sizeof( pdpte ) ) )
                    return false;

                if ( !pdpt_entry.hard.present ) {
                    return false;
                }

                pde pd_entry;
                if ( !m_read_physical(
                    ( pdpt_entry.hard.pfn << 12 ) + ( sizeof( pde ) * va.pde_index ),
                    &pd_entry, sizeof( pde ) ) )
                    return false;

                if ( !pd_entry.hard.present ) {
                    return false;
                }

                pte pt_entry{ };
                if ( !m_read_physical( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
                    &pt_entry, sizeof( pte ) ) )
                    return false;

                auto page_va = nt::mm_allocate_independent_pages( page_4kb_size );
                if ( !page_va ) return false;
                nt::memset( page_va, 0, page_4kb_size );

                auto page_pa = phys::virtual_to_physical(
                    reinterpret_cast< std::uint64_t >( page_va ) );
                if ( !page_pa ) {
                    return false;
                }

                pte new_pte{ 0 };
                new_pte.hard.present = 1;
                new_pte.hard.read_write = 1;
                new_pte.hard.user_supervisor = 1;
                new_pte.hard.no_execute = 0;
                new_pte.hard.pfn = page_pa >> page_shift;

                auto pt_phys = ( pd_entry.hard.pfn << 12 ) + ( va.pte_index * sizeof( pte ) );
                if ( !m_write_physical( pt_phys, &new_pte, sizeof( pte ) ) ) {
                    return false;
                }

                nt::flush_caches( page_va );
            }

            return true;
        }

        bool validate_page_tables( std::uint64_t base_va, std::size_t page_count ) {
            const auto page_mask = page_4kb_size - 1;
            const auto aligned_size = ( page_count + page_mask ) & ~page_mask;
            if ( !aligned_size ) {
                return false;
            }

            const auto page_size = page_4kb_size;
            for ( std::uint64_t current_va = base_va; current_va < base_va + aligned_size; current_va += page_size ) {
                virt_addr_t va{ current_va };

                pml4e pml4_entry;
                if ( !m_read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
                    &pml4_entry, sizeof( pml4e ) ) ) {
                    logging::print( oxorany( "[c_paging] validate_page_tables: Could not read PML4E" ) );
                    return false;
                }

                if ( !pml4_entry.hard.present ) {
                    logging::print( oxorany( "[c_paging] validate_page_tables: PML4E is not present" ) );
                    return false;
                }

                pdpte pdpt_entry;
                if ( !m_read_physical(
                    ( pml4_entry.hard.pfn << 12 ) + ( sizeof( pdpte ) * va.pdpte_index ),
                    &pdpt_entry, sizeof( pdpte ) ) ) {
                    logging::print( oxorany( "[c_paging] validate_page_tables: Could not read PDPTE" ) );
                    return false;
                }

                if ( !pdpt_entry.hard.present ) {
                    logging::print( oxorany( "[c_paging] validate_page_tables: PDPTE is not present" ) );
                    return false;
                }

                pde pd_entry;
                if ( !m_read_physical(
                    ( pdpt_entry.hard.pfn << 12 ) + ( sizeof( pde ) * va.pde_index ),
                    &pd_entry, sizeof( pde ) ) ) {
                    logging::print( oxorany( "[c_paging] validate_page_tables: Could not read PDE" ) );
                    return false;
                }

                if ( !pd_entry.hard.present ) {
                    logging::print( oxorany( "[c_paging] validate_page_tables: PDE is not present" ) );
                    return false;
                }

                pte pt_entry{ };
                if ( !m_read_physical( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
                    &pt_entry, sizeof( pte ) ) ) {
                    logging::print( oxorany( "[c_paging] validate_page_tables: Could not read PTE" ) );
                    return false;
                }

                if ( !pt_entry.hard.present ) {
                    logging::print( oxorany( "[c_paging] validate_page_tables: PTE is not present" ) );
                    return false;
                }
            }

            return true;
        }

        std::uint32_t find_non_present_pml4e( bool use_high_address ) {
            std::uint32_t start_idx = 0;
            std::uint32_t end_idx = 0;

            if ( use_high_address ) {
                start_idx = 256;
                end_idx = 512;
            }
            else {
                start_idx = 1;
                end_idx = 256;
            }

            for ( auto idx = start_idx; idx < end_idx; idx++ ) {
                pml4e pml4_entry{};
                if ( !m_read_physical( m_target_cr3 + ( idx * sizeof( pml4e ) ), &pml4_entry, sizeof( pml4e ) ) )
                    continue;

                if ( !pml4_entry.hard.present ) {
                    logging::print( oxorany( "[c_hyperspace] Found non-present PML4E at index %d (%s space)" ),
                        idx, use_high_address ? oxorany( "kernel" ) : oxorany( "user" ) );
                    return idx;
                }
            }

            logging::print( oxorany( "[c_hyperspace] No non-present PML4E found in %s space" ),
                use_high_address ? oxorany( "kernel" ) : oxorany( "user" ) );
            return static_cast< std::uint32_t >( -1 );
        }

        std::uint64_t remap_physical_page( std::uint64_t new_cr3, std::uint64_t base_va, bool high_address = false ) {
            virt_addr_t va{ base_va };

            pml4e pml4_entry{ };
            if ( !m_read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
                &pml4_entry, sizeof( pml4e ) ) )
                return 0;

            std::uint64_t orig_cr3;
            swap_context( new_cr3, &orig_cr3 );

            auto pml4_index = find_non_present_pml4e( high_address );
            if ( pml4_index == -1 ) {
                swap_context( orig_cr3 );
                return 0;
            }

            if ( !m_write_physical( m_target_cr3 + ( pml4_index * sizeof( pml4e ) ),
                &pml4_entry, sizeof( pml4e ) ) ) {
                swap_context( orig_cr3 );
                return 0;
            }

            swap_context( orig_cr3 );

            auto new_va = va;
            new_va.pml4e_index = pml4_index;
            nt::flush_caches( ( void* ) new_va.value );
            return new_va.value;
        }

        std::unique_ptr<pml4e> get_pml4e( std::uintptr_t address ) {
            virt_addr_t va{ address };

            auto pml4_entry = std::make_unique<pml4e>( );
            if ( !m_read_physical( m_target_cr3 + ( va.pml4e_index * sizeof( pml4e ) ),
                pml4_entry.get( ), sizeof( pml4e ) ) )
                return nullptr;

            return pml4_entry;
        }

        std::unique_ptr<pdpte> get_pdpte( std::uintptr_t address ) {
            virt_addr_t va{ address };
            auto pml4_entry = get_pml4e( address );
            if ( !pml4_entry || !pml4_entry->hard.present ) {
                logging::print( "[c_dpm] get_pdpte: pml4_entry is nullptr for VA 0x%llx", address );
                return nullptr;
            }

            auto pdpt_entry = std::make_unique<pdpte>( );
            auto result = m_read_physical(
                ( pml4_entry->hard.pfn << 12 ) + ( sizeof( pdpte ) * va.pdpte_index ),
                pdpt_entry.get( ),
                sizeof( pdpte )
            );

            if ( !result ) {
                logging::print( "[c_dpm] get_pdpte: FAILED to retrieve PDPTE, invalid present or failed to read" );
                return nullptr;
            }

            return pdpt_entry;
        }

        std::unique_ptr<pde> get_pde( std::uintptr_t address ) {
            virt_addr_t va{ address };

            auto pml4_entry = get_pml4e( address );
            if ( !pml4_entry || !pml4_entry->hard.present )
                return nullptr;

            auto pdpt_entry = std::make_unique<pdpte>( );
            auto result = m_read_physical(
                ( pml4_entry->hard.pfn << 12 ) + ( sizeof( pdpte ) * va.pdpte_index ),
                pdpt_entry.get( ),
                sizeof( pdpte )
            );

            if ( !result || !pdpt_entry->hard.present ) {
                return nullptr;
            }

            auto pd_entry = std::make_unique<pde>( );
            result = m_read_physical(
                ( pdpt_entry->hard.pfn << 12 ) + ( sizeof( pde ) * va.pde_index ),
                pd_entry.get( ),
                sizeof( pde )
            );

            if ( !result )
                return nullptr;

            return pd_entry;
        }

        std::unique_ptr<pte> get_pte( std::uintptr_t address ) {
            virt_addr_t va{ address };

            auto pml4_entry = get_pml4e( address );
            if ( !pml4_entry )
                return nullptr;

            auto pdpt_entry = std::make_unique<pdpte>( );
            auto result = m_read_physical(
                ( pml4_entry->hard.pfn << 12 ) + ( sizeof( pdpte ) * va.pdpte_index ),
                pdpt_entry.get( ),
                sizeof( pdpte )
            );

            if ( !result || !pdpt_entry->hard.present ) {
                return nullptr;
            }

            auto pd_entry = std::make_unique<pde>( );
            result = m_read_physical(
                ( pdpt_entry->hard.pfn << 12 ) + ( sizeof( pde ) * va.pde_index ),
                pd_entry.get( ),
                sizeof( pde )
            );

            if ( !result || !pd_entry->hard.present ) {
                return nullptr;
            }

            auto pt_entry = std::make_unique<pte>( );
            result = m_read_physical(
                ( pd_entry->hard.pfn << 12 ) + ( sizeof( pte ) * va.pte_index ),
                pt_entry.get( ),
                sizeof( pte )
            );

            if ( !result )
                return nullptr;

            return pt_entry;
        }

    private:
        void thread_worker( const std::uint64_t& start, const std::uint64_t& end,
            const virt_addr_t& va, std::atomic<std::uint64_t>& found_cr3,
            std::atomic<bool>& found ) {

            if ( found.load( std::memory_order_acquire ) )
                return;

            const auto count = end - start;
            for ( auto idx = 0; idx < count; ++idx ) {
                if ( found.load( std::memory_order_acquire ) )
                    return;

                const auto current_pfn = start + idx;
                const auto current_pa = current_pfn << 12;

                if ( !current_pa )
                    continue;

                ::pml4e pml4e;
                if ( !m_read_physical( current_pa + ( va.pml4e_index * sizeof( pml4e ) ),
                    &pml4e, sizeof( pml4e ) ) )
                    continue;

                if ( !pml4e.hard.present )
                    continue;

                if ( !pml4e.hard.pfn || pml4e.hard.pfn >= end )
                    continue;

                auto pdpte_pa = pml4e.hard.pfn << 12;
                if ( !is_physical_address_valid( pdpte_pa, sizeof( ::pdpte ) ) )
                    continue;

                ::pdpte pdpte;
                if ( !m_read_physical( pdpte_pa + ( va.pdpte_index * sizeof( pdpte ) ),
                    &pdpte, sizeof( pdpte ) ) )
                    continue;

                if ( !pdpte.hard.present )
                    continue;

                if ( !pdpte.hard.pfn || pdpte.hard.pfn >= end )
                    continue;

                auto pde_pa = pdpte.hard.pfn << 12;
                if ( !is_physical_address_valid( pde_pa, sizeof( ::pde ) ) )
                    continue;

                ::pde pde;
                if ( !m_read_physical( pde_pa + ( va.pde_index * sizeof( pde ) ),
                    &pde, sizeof( pde ) ) )
                    continue;

                if ( !pde.hard.present )
                    continue;

                if ( !pde.hard.pfn || pde.hard.pfn >= end )
                    continue;

                auto pte_pa = pde.hard.pfn << 12;
                if ( !is_physical_address_valid( pte_pa, sizeof( ::pte ) ) )
                    continue;

                ::pte pte;
                if ( !m_read_physical( pte_pa + ( va.pte_index * sizeof( pte ) ),
                    &pte, sizeof( pte ) ) )
                    continue;

                if ( !pte.hard.present )
                    continue;

                if ( !pte.hard.pfn || pte.hard.pfn >= end )
                    continue;

                std::uint64_t expected = 0;
                if ( found_cr3.compare_exchange_strong( expected, current_pa,
                    std::memory_order_release, std::memory_order_relaxed ) ) {
                    found.store( true, std::memory_order_release );
                }

                return;
            }
        }
    };
}