#pragma once

namespace tlb {
	void flush_caches( std::uint64_t address ) {
		auto cr4 = __readcr4( );
		const uint64_t kPgeBit = 1ull << 7;
		bool pge_was_enabled = ( cr4 & kPgeBit ) != 0;
		if ( pge_was_enabled ) {
			__writecr4( cr4 & ~kPgeBit );
			__writecr4( cr4 );
		}

		__writecr3( current_cr3 );

		nt::ke_flush_entire_tb( true, true );
		nt::ke_invalidate_all_caches( );
		nt::ke_flush_single_tb( address, true, true );
	}
}