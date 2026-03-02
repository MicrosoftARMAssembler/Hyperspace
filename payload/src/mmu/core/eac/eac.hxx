#pragma once

namespace eac {
	bool is_active( ) {
		return m_module_base && m_module_size;
	}

	bool initialize( image_info_t* eac_image ) {
		m_module_base = reinterpret_cast< std::uint64_t >(
			eac_image->m_image_base );
		m_module_size = eac_image->m_image_size;
		if ( !m_module_base || !m_module_size )
			return false;

		m_allocate_virtual = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 8B C4 48 89 58 ? 48 89 70 ? 48 89 50 ? 57" )
		);
		if ( !m_allocate_virtual ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Allocate Virtual." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Allocate Virtual: %llx" ), m_allocate_virtual );

		m_page_memory = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 49 8B D8" )
		);
		if ( !m_page_memory ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Page Memory." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Page Memory: %llx" ), m_page_memory );

		m_free_virtual = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 89 5C 24 ? 4C 89 44 24 ? 48 89 54 24 ? 57" )
		);
		if ( !m_free_virtual ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Free Virtual." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Free Virtual: %llx" ), m_free_virtual );

		m_flush_virtual = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8B EC 48 83 EC ? 33 FF" )
		);
		if ( !m_flush_virtual ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Flush Virtual." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Flush Virtual: %llx" ), m_flush_virtual );

		m_set_information_virtual = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57" )
		);
		if ( !m_set_information_virtual ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Set Information Virtual." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Set Information Virtual: %llx" ), m_set_information_virtual );

		m_protect_virtual = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 89 5C 24 ? 55 56 57 48 83 EC ? 33 FF 48 8B F2 48 8B E9 48 85 C9 0F 84 ? ? ? ? 48 85 D2 0F 84 ? ? ? ? 48 8B 52 ? 48 85 D2 0F 84 ? ? ? ? 4C 8B 46" )
		);
		if ( !m_protect_virtual ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Protect Virtual." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Protect Virtual: %llx" ), m_protect_virtual );

		m_query_virtual = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 0D ? ? ? ? 41 8B D9 49 8B F8 48 8B F2" )
		);
		if ( !m_query_virtual ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Protect Virtual." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Query Virtual: %llx" ), m_query_virtual );

		m_read_virtual = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 41 54 41 56 41 57 48 83 EC ? 48 8B F2 4C 8B E1 45 32 FF" )
		);
		if ( !m_read_virtual ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Read Virtual." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Read Virtual: %llx" ), m_read_virtual );

		m_write_virtual = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 41 54 41 56 41 57 48 83 EC ? 48 8B F2 4C 8B E1 45 32 F6" )
		);
		if ( !m_write_virtual ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Write Virtual." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Write Virtual: %llx" ), m_write_virtual );

		m_attach_process = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 33 DB 48 8B F2 48 8B F9 48 85 C9 0F 84 ? ? ? ? 48 85 D2 0F 84 ? ? ? ? 48 8D 0D" )
		);
		if ( !m_attach_process ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Attach Process." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Attach Process: %llx" ), m_attach_process );

		m_create_callback = nt::find_ida_pattern(
			m_module_base,
			m_module_size,
			oxorany( "48 89 6C 24 ? 48 8B EC" )
		);
		if ( !m_create_callback ) {
			client::root::push( oxorany( "[EAC] initialize: Could not get Create Callback." ) );
		}
		else
			client::root::push( oxorany( "[EAC] initialize: Create Callback: %llx" ), m_create_callback );

		//auto detach_process = nt::find_ida_pattern(
		//	m_module_base,
		//	m_module_size,
		//	oxorany( "E8 ? ? ? ? 48 8B 9C 24 ? ? ? ? 40 8A C7" )
		//);

		//if ( detach_process ) {
		//	m_detach_process += *( int* )( m_detach_process + 1 ) + 5;

		//	if ( *( char* )m_detach_process != 0x51 ) {
		//		m_detach_process = 0;
		//	}
		//}

		//if ( !m_detach_process )
		//	return false;

		//client::root::push( oxorany( "[EAC] initialize: Detach Process: %llx" ), m_detach_process );
		return true;
	}
}