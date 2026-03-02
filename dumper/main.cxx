#pragma once
#include <impl/includes.h>

void display_bytes( const void* page_data, std::uint64_t page_offset ) {
	const auto* bytes = static_cast< const std::uint8_t* >( page_data );

	logging::print( oxorany( "[dumper] main: Dumped page: 0x%llx" ), page_offset );

	for ( std::uint32_t row = 0; row < 256; ++row ) {
		std::uint32_t offset = row * 16;

		printf( "  %04x: ", offset );

		for ( std::uint32_t col = 0; col < 16; ++col ) {
			printf( "%02x ", bytes[ offset + col ] );
		}

		printf( " | " );
		for ( std::uint32_t col = 0; col < 16; ++col ) {
			std::uint8_t byte = bytes[ offset + col ];
			printf( "%c", ( byte >= 32 && byte <= 126 ) ? byte : '.' );
		}
		printf( "\n" );
	}
	printf( "\n" );
}

int main( ) {
	SetConsoleTitleA( oxorany( "hyperspace-dumper" ) );

	if ( !hyperspace::initialize( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not initialize hyperspace." ) );
		return std::getchar( );
	}

	if ( !hyperspace::is_active( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not ping driver." ) );
		return std::getchar( );
	}

	logging::print( oxorany( "[hyperspace] main: Successfully pinged driver" ) );
	std::getchar( );

	if ( !hyperspace::target::attach_process( oxorany( target_process ) ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not attach to process." ) );
		hyperspace::unload( );
		return std::getchar( );
	}

	std::getchar( );

	auto dos_header = hyperspace::read_virt<dos_header_t>(
		hyperspace::target::m_base_address );
	if ( !dos_header.is_valid( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not get MZ headers." ) );
		hyperspace::unload( );
		return std::getchar( );
	}

	auto nt_headers = hyperspace::read_virt<nt_headers_t>(
		hyperspace::target::m_base_address + dos_header.m_lfanew );
	if ( !nt_headers.is_valid( ) ) {
		logging::print( oxorany( "[hyperspace] main: Could not get PE headers." ) );
		hyperspace::unload( );
		return std::getchar( );
	}

	logging::print( oxorany( "[hyperspace] main: F5 initialized image" ) );
	logging::print( oxorany( "[hyperspace] main: Press F5 to stop capturing pages." ) );

	auto display_captured_bytes = MessageBoxA(
		0,
		oxorany( "If you enable this every captured pages bytes will be dumped." ),
		oxorany( "Would you like to display captured bytes?" ),
		MB_YESNO | MB_ICONQUESTION | MB_SERVICE_NOTIFICATION
	) == IDYES;

	auto output_buffer = VirtualAlloc(
		0,
		nt_headers.m_size_of_image,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE );
	if ( !output_buffer ) {
		logging::print( oxorany( "[hyperspace] main: Could not allocate buffer." ) );
		hyperspace::unload( );
		return std::getchar( );
	}

	const auto page_size = 0x1000;
	const auto total_pages = nt_headers.m_size_of_image / page_size;

	std::vector<bool> captured_pages( total_pages, false );
	std::uint64_t encrypted_pages = 0;
	std::uint64_t decrypted_pages = 0;
	std::uint64_t pages_captured = 0;

	while ( pages_captured < total_pages && !( GetAsyncKeyState( VK_F5 ) ) ) {
		for ( std::uint64_t page_index = 0; page_index < total_pages; ++page_index ) {
			if ( captured_pages[ page_index ] )
				continue;

			auto page_offset = page_index * page_size;
			auto process_page = hyperspace::target::m_base_address + page_offset;

			bool is_encrypted_page = false;
			if ( hyperspace::eac::is_active( ) ) {
				memory_basic_information_t memory_info{ };
				if ( !hyperspace::eac::query_virtual( process_page, &memory_info ) ) {
					logging::print( oxorany( "[dumper] main: Could not query memory info for page: 0x%llx" ),
						process_page );
					continue;
				}

				is_encrypted_page = memory_info.m_protect == PAGE_NOACCESS;
				if ( is_encrypted_page ) {
					++encrypted_pages;
					logging::print( oxorany( "[dumper] main: Encrypted page: process_page=0x%llx protect=0x%x" ),
						process_page, memory_info.m_protect );

					hyperspace::eac::read_virtual(
						process_page,
						static_cast< std::uint8_t* >( output_buffer ) + page_offset,
						page_size
					);

					hyperspace::eac::page_memory(
						process_page,
						page_size
					);

					size_t flush_size = page_size;
					hyperspace::eac::flush_virtual(
						&process_page,
						&flush_size
					);

					std::uint32_t old_protect = 0;
					std::uint32_t protect_size = page_size;
					hyperspace::eac::protect_virtual(
						process_page,
						protect_size,
						PAGE_NOACCESS,
						&old_protect
					);

					hyperspace::eac::set_information_virtual(
						process_page,
						flush_size
					);
				}
			}

			hyperspace::paging::pt_entries_t pt_entries;
			if ( !hyperspace::paging::hyperspace_entries( pt_entries, process_page ) )
				continue;

			logging::print(
				oxorany( "[dumper] main: Captured page: pml4e=0x%llx pdpte=0x%llx pde=0x%llx pte=0x%llx" ),
				pt_entries.m_pml4e.value, pt_entries.m_pdpte.value,
				pt_entries.m_pde.value, pt_entries.m_pte.value
			);

			auto physical_page = hyperspace::paging::translate_linear( process_page );
			if ( !hyperspace::paging::is_address_valid( physical_page ) )
				continue;

			auto mapped_page = hyperspace::paging::ptm::map_process_page(
				physical_page,
				hyperspace::paging::page_protection::readwrite,
				true
			);
			if ( !mapped_page )
				continue;

			auto dest = static_cast< std::uint8_t* >( output_buffer ) + page_offset;
			std::memcpy( dest, mapped_page, page_size );

			if ( display_captured_bytes )
				display_bytes( dest, page_offset );

			if ( is_encrypted_page )
				decrypted_pages++;

			captured_pages[ page_index ] = true;
			++pages_captured;
		}

		auto percentage = ( static_cast< double >( pages_captured ) / total_pages ) * 100.0;
		logging::print(
			oxorany( "[dumper] main: Progress: %llu/%llu pages (%.2f%%)" ),
			pages_captured, total_pages, percentage
		);

		auto encrypted_percentage = total_pages > 0
			? ( static_cast< double >( encrypted_pages ) / total_pages ) * 100.0
			: 0.0;
		auto decrypted_percentage = total_pages > 0
			? ( static_cast< double >( decrypted_pages ) / total_pages ) * 100.0
			: 0.0;
		logging::print(
			oxorany( "[dumper] main: Encrypted: %llu (%.2f%%) / Decrypted: %llu (%.2f%%)" ),
			encrypted_pages, encrypted_percentage,
			decrypted_pages, decrypted_percentage
		);

		decrypted_pages = encrypted_pages = 0;
		std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );
	}

	if ( pages_captured == total_pages ) {
		logging::print( oxorany( "[hyperspace] main: Successfully captured all pages!" ) );
	}
	else {
		logging::print(
			oxorany( "[hyperspace] main: Stopped capturing pages (%llu/%llu captured)" ),
			pages_captured, total_pages
		);
	}

	std::getchar( );
	logging::print( oxorany( "[hyperspace] main: Stopped capturing pages" ) );
	hyperspace::unload( );
	return std::getchar( );
}