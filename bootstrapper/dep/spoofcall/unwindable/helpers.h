#pragma once

namespace uwd_private {

#define unwind_get_code_entry(info, index) ((info)->code [index])
#define unwind_get_lang_specific_data(info) ((void*) &unwind_get_code_entry ((info),((info)->code_cnt + 1) & ~1))
#define unwind_get_exception_handler(base, info) ((PEXCEPTION_ROUTINE) ((base) + *(PULONG) unwind_get_lang_specific_data (info)))
#define unwind_get_chained_entry(base, info) ((PRUNTIME_FUNCTION) ((base) + *(PULONG) unwind_get_lang_specific_data (info)))
#define unwind_get_exception_ptr(info) ((void*) ((PULONG) unwind_get_lang_specific_data (info) + 1))

#define unwind_hidword(val) ((uint32_t)(((DWORDLONG)(val)>>32)&0xFFFFFFFF))

#define unwind_get_bitval(data, pos) ((data>>pos) & 1)
#define unwind_get_chain_info(data) unwind_get_bitval (data, 2)
#define unwind_get_u_handler(data) unwind_get_bitval (data, 1)
#define unwind_get_e_handler(data) unwind_get_bitval (data, 0)
#define unwind_get_ver(data) (unwind_get_bitval (data, 4) * 2) + unwind_get_bitval (data, 3)

    typedef union {

        struct {
            uint8_t code_off;
            uint8_t unwind_opcode : 4;
            uint8_t opcode_info : 4;
        };

        uint16_t frame_off;

    } unwind_code;

    typedef struct {

        uint8_t     ver : 3;
        uint8_t     flags : 5;
        uint8_t     prolog_sz;
        uint8_t     code_cnt;
        uint8_t     frame_reg : 4;
        uint8_t     frame_off : 4;

        unwind_code code[ 1 ];

        union {

            OPTIONAL uint32_t exc_handler;
            OPTIONAL uint32_t fn_handler;

        };

        OPTIONAL uint32_t exc_data[ ];

    } unwind_info;

    typedef enum {

        mode_1616,
        mode_1632,
        mode_real,
        mode_flat

    } addr_mode;

    typedef struct {

        size_t     off;
        uint16_t   seg;
        addr_mode  mod;

    } addr;

    typedef enum {

        RAX = 0,
        RCX,
        RDX,
        RBX,
        RSP,
        RBP,
        RSI,
        RDI,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15

    } regs;

    typedef struct {

        size_t rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi;
        size_t  r8, r9, r10, r11, r12, r13, r14, r15;

        size_t rip;
        size_t reserved;

        size_t stack_sz;

    } context;

    typedef enum {

        // x86_64. https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64.
        UWOP_PUSH_NONVOL = 0,
        UWOP_ALLOC_LARGE,       // 1
        UWOP_ALLOC_SMALL,       // 2
        UWOP_SET_FPREG,         // 3
        UWOP_SAVE_NONVOL,       // 4
        UWOP_SAVE_NONVOL_BIG,   // 5
        UWOP_EPILOG,            // 6
        UWOP_SPARE_CODE,        // 7
        UWOP_SAVE_XMM128,       // 8
        UWOP_SAVE_XMM128BIG,    // 9
        UWOP_PUSH_MACH_FRAME,   // 10

        // ARM64. https://docs.microsoft.com/en-us/cpp/build/arm64-exception-handling
        UWOP_ALLOC_MEDIUM,
        UWOP_SAVE_R19R20X,
        UWOP_SAVE_FPLRX,
        UWOP_SAVE_FPLR,
        UWOP_SAVE_REG,
        UWOP_SAVE_REGX,
        UWOP_SAVE_REGP,
        UWOP_SAVE_REGPX,
        UWOP_SAVE_LRPAIR,
        UWOP_SAVE_FREG,
        UWOP_SAVE_FREGX,
        UWOP_SAVE_FREGP,
        UWOP_SAVE_FREGPX,
        UWOP_SET_FP,
        UWOP_ADD_FP,
        UWOP_NOP,
        UWOP_END,
        UWOP_SAVE_NEXT,
        UWOP_TRAP_FRAME,
        UWOP_CONTEXT,
        UWOP_CLEAR_UNWOUND_TO_CALL,

        // ARM: https://docs.microsoft.com/en-us/cpp/build/arm-exception-handling
        UWOP_ALLOC_HUGE,
        UWOP_WIDE_ALLOC_MEDIUM,
        UWOP_WIDE_ALLOC_LARGE,
        UWOP_WIDE_ALLOC_HUGE,

        UWOP_WIDE_SAVE_REG_MASK,
        UWOP_WIDE_SAVE_SP,
        UWOP_SAVE_REGS_R4R7LR,
        UWOP_WIDE_SAVE_REGS_R4R11LR,
        UWOP_SAVE_FREG_D8D15,
        UWOP_SAVE_REG_MASK,
        UWOP_SAVE_LR,
        UWOP_SAVE_FREG_D0D15,
        UWOP_SAVE_FREG_D16D31,
        UWOP_WIDE_NOP, // UWOP_NOP
        UWOP_END_NOP,  // UWOP_END
        UWOP_WIDE_END_NOP,

        // Custom implementation opcodes (implementation specific).
        UWOP_CUSTOM,

    } unwind_opcodes;

    typedef struct {

        size_t      thread;
        uint32_t    th_cb_stack, th_cb_restore, next_cb;

        uint32_t    frame_ptr;

        size_t      ki_call_um;
        size_t      ke_um_cb_dispatcher;
        size_t      sys_range_start;
        size_t      ki_um_exc_dispatcher;

        size_t      stack_base, stack_limit;

        uint32_t    build_ver;
        uint32_t    retpoline_stub_table_sz;

        size_t      retpoline_stub_table;
        uint32_t    retpoline_stub_off, retpoline_stub_sz;

        size_t      reserved[ 2 ];

    } kdhelp_64;

    typedef struct {

        addr        pc, ret, frame, stack;
        void* table_entry;
        uint32_t    params[ 4 ];
        uint32_t    is_far, is_virtual;
        uint32_t    reserved[ 3 ];
        kdhelp_64   kd_help;
        addr        bstore;

    } stack_frame;

    static auto get_exception_directory( uint64_t mod, uint32_t* out_sz ) {

        auto dos = reinterpret_cast < PIMAGE_DOS_HEADER > ( mod );
        auto nt = reinterpret_cast < PIMAGE_NT_HEADERS > ( mod + dos->e_lfanew );

        auto exception_dir_va = nt->OptionalHeader.DataDirectory[ 3 ].VirtualAddress;
        *out_sz = nt->OptionalHeader.DataDirectory[ 3 ].Size;

        return mod + exception_dir_va;
    }

    static auto get_export_directory( uint64_t mod ) {

        auto dos = reinterpret_cast < PIMAGE_DOS_HEADER > ( mod );
        auto nt = reinterpret_cast < PIMAGE_NT_HEADERS > ( mod + dos->e_lfanew );

        return mod + nt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress;
    }

    static PIMAGE_RUNTIME_FUNCTION_ENTRY get_function_entry( uint64_t mod, uint64_t offset ) {

        uint32_t    exception_sz = 0;
        auto        exception_table = reinterpret_cast < PRUNTIME_FUNCTION >       ( get_exception_directory( mod, &exception_sz ) );
        auto        export_table = reinterpret_cast < PIMAGE_EXPORT_DIRECTORY > ( get_export_directory( mod ) );

        for ( uint32_t idx = 0; idx < export_table->NumberOfFunctions; idx++ ) {

            if ( exception_table[ idx ].BeginAddress == offset ) {
                return exception_table + idx;
            }

        }

        return NULL;
    }

}