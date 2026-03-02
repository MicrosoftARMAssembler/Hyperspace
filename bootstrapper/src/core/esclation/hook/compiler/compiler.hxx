#pragma once

namespace compiler {
    struct shellcode_t {
        std::vector<std::uint8_t> m_byte_code;
        std::size_t m_size;
    };

    class c_compiler {
    public:
        bool compile_callback( shellcode_t* result ) {
            using namespace asmjit;
            CodeHolder code;
            code.init( Environment::host( ) );
            x86::Assembler assembler( &code );

            assembler.mov( x86::r11, x86::r9 );
            assembler.sub( x86::rsp, 0x58 );

            assembler.mov( x86::rax, x86::ptr( x86::r11, 0x0 ) );

            assembler.mov( x86::rcx, x86::ptr( x86::r11, 0x8 ) );
            assembler.mov( x86::rdx, x86::ptr( x86::r11, 0x10 ) );
            assembler.mov( x86::r8, x86::ptr( x86::r11, 0x18 ) );
            assembler.mov( x86::r9, x86::ptr( x86::r11, 0x20 ) );

            assembler.mov( x86::r10, x86::ptr( x86::r11, 0x28 ) );
            assembler.mov( x86::ptr( x86::rsp, 0x20 ), x86::r10 );

            assembler.mov( x86::r10, x86::ptr( x86::r11, 0x30 ) );
            assembler.mov( x86::ptr( x86::rsp, 0x28 ), x86::r10 );

            assembler.mov( x86::r10, x86::ptr( x86::r11, 0x38 ) );
            assembler.mov( x86::ptr( x86::rsp, 0x30 ), x86::r10 );

            assembler.mov( x86::r10, x86::ptr( x86::r11, 0x40 ) );
            assembler.mov( x86::ptr( x86::rsp, 0x38 ), x86::r10 );

            assembler.mov( x86::r10, x86::ptr( x86::r11, 0x48 ) );
            assembler.mov( x86::ptr( x86::rsp, 0x40 ), x86::r10 );

            assembler.mov( x86::r10, x86::ptr( x86::r11, 0x50 ) );
            assembler.mov( x86::ptr( x86::rsp, 0x48 ), x86::r10 );

            assembler.call( x86::rax );

            assembler.add( x86::rsp, 0x58 );
            assembler.ret( );

            auto section = code.sectionById( 0 );
            if ( !section ) {
                return false;
            }
            result->m_size = section->bufferSize( );
            result->m_byte_code.resize( result->m_size );
            std::memcpy( result->m_byte_code.data( ), section->data( ), result->m_size );
            return true;
        }
    };
}