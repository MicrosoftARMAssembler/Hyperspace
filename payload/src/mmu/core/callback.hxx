#pragma once

namespace callback {
    std::uint64_t m_image_handle = 0;
    std::uint64_t m_process_handle = 0;
    std::uint64_t m_thread_handle = 0;

    constexpr std::uint8_t callback_shellcode[ ] = {
        // 1. Save all volatile registers (Parameters & Scratch)
        0x50,                                                    // push rax
        0x51,                                                    // push rcx
        0x52,                                                    // push rdx
        0x41, 0x50,                                             // push r8
        0x41, 0x51,                                             // push r9
        0x41, 0x52,                                             // push r10
        0x41, 0x53,                                             // push r11

        // 2. Align Stack & Allocate Shadow Space
        0x48, 0x83, 0xEC, 0x20,                                 // sub rsp, 0x20

        // 3. Load Handler Address
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rax, handler (Offset 17)

        // 4. Call Handler
        0xFF, 0xD0,                                             // call rax

        // 5. Restore Stack
        0x48, 0x83, 0xC4, 0x20,                                 // add rsp, 0x20

        // 6. Restore Registers
        0x41, 0x5B,                                             // pop r11
        0x41, 0x5A,                                             // pop r10
        0x41, 0x59,                                             // pop r9
        0x41, 0x58,                                             // pop r8
        0x5A,                                                    // pop rdx
        0x59,                                                    // pop rcx
        0x58,                                                    // pop rax

        // 7. Return to Caller
        0xC3                                                     // ret
    };

    bool create_image( void* image_callback ) {
        if ( m_image_handle ) {
            return false;
        }

        std::uint8_t shellcode[ sizeof( callback_shellcode ) ];
        std::memcpy( shellcode, callback_shellcode, sizeof( shellcode ) );
        *reinterpret_cast< std::uint64_t* >( shellcode + 17 ) =
            reinterpret_cast< std::uint64_t >( image_callback );

        auto target_address = module::find_unused_space( nt::m_module_base, sizeof( shellcode ) );
        if ( !target_address ) {
            nt::dbg_print( oxorany( "[callback] create_image: Could not allocate GAP\n" ) );
            return false;
        }

        std::memcpy(
            reinterpret_cast< void* >( target_address ),
            shellcode,
            sizeof( shellcode )
        );

        if ( auto result = nt::ps_set_load_image_notify_routine(
            reinterpret_cast< p_load_image_notify_routine >
            ( target_address ) ) ) {
            nt::dbg_print( oxorany( "[callback] create_image: Could not create callback: %i\n" ), result );
            return false;
        }

        m_image_handle = target_address;
        return true;
    }

    bool create_process( void* process_callback ) {
        if ( m_process_handle ) {
            return false;
        }

        std::uint8_t shellcode[ sizeof( callback_shellcode ) ];
        std::memcpy( shellcode, callback_shellcode, sizeof( shellcode ) );
        *reinterpret_cast< std::uint64_t* >( shellcode + 17 ) =
            reinterpret_cast< std::uint64_t >( process_callback );

        auto target_address = module::find_unused_space( nt::m_module_base, sizeof( shellcode ) );
        if ( !target_address ) {
            nt::dbg_print( oxorany( "[callback] create_process: Could not allocate GAP\n" ) );
            return false;
        }

        std::memcpy(
            reinterpret_cast< void* >( target_address ),
            shellcode,
            sizeof( shellcode )
        );

        if ( auto result = nt::ps_set_create_process_notify_routine(
            reinterpret_cast< p_create_process_notify_routine >
            ( target_address ), false ) ) {
            nt::dbg_print( oxorany( "[callback] create_process: Could not create callback: %i\n" ), result );
            return false;
        }

        m_process_handle = target_address;
        return true;
    }

    bool create_thread( void* thread_callback ) {
        if ( m_thread_handle ) {
            return false;
        }

        std::uint8_t shellcode[ sizeof( callback_shellcode ) ];
        std::memcpy( shellcode, callback_shellcode, sizeof( shellcode ) );
        *reinterpret_cast< std::uint64_t* >( shellcode + 17 ) =
            reinterpret_cast< std::uint64_t >( thread_callback );

        auto target_address = module::find_unused_space( nt::m_module_base, sizeof( shellcode ) );
        if ( !target_address ) {
            nt::dbg_print( oxorany( "[callback] create_image: Could not allocate GAP\n" ) );
            return false;
        }

        std::memcpy(
            reinterpret_cast< void* >( target_address ),
            shellcode,
            sizeof( shellcode )
        );

        if ( nt::ps_set_create_thread_notify_routine( 
            reinterpret_cast< p_create_thread_notify_routine >
            ( target_address ) ) )
            return false;

        m_thread_handle = target_address;
        return true;
    }

    bool remove_image( ) {
        if ( !m_image_handle ) {
            return false;
        }

        if ( nt::ps_remove_load_image_notify_routine(
            reinterpret_cast< p_load_image_notify_routine >
            ( m_image_handle ) ) )
            return false;
 
        nt::dbg_print( oxorany( "[callback] remove_image: Successfully removed callback\n" ) );
        m_image_handle = 0;
        return true;
    }

    bool remove_process( ) {
        if ( !m_process_handle ) {
            return false;
        }

        if ( nt::ps_set_create_process_notify_routine(
            reinterpret_cast< p_create_process_notify_routine >
            ( m_process_handle ), true ) )
            return false;

        nt::dbg_print( oxorany( "[callback] remove_process: Successfully removed callback\n" ) );
        m_process_handle = 0;
        return true;
    }

    bool remove_thread( ) {
        if ( !m_thread_handle ) {
            return false;
        }

        if ( nt::ps_remove_create_thread_notify_routine( 
            reinterpret_cast< p_create_thread_notify_routine >
            ( m_thread_handle ) ) )
            return false;

        nt::dbg_print( oxorany( "[callback] remove_thread: Successfully removed callback\n" ) );
        m_thread_handle = 0;
        return true;
    }
}