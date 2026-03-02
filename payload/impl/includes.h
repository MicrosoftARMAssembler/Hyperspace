#pragma once
#define current_cr3 __readcr3( )
#define return_address _ReturnAddress( )
#define address_of_return reinterpret_cast< void** >( _AddressOfReturnAddress( ) )
#define max_stack_frame reinterpret_cast< void** >( __readgsqword( 0x1a8 ) )
//#define MY_PROJECT_ROOT

#include <cstdarg>
#include <impl/std/std.h>
#include <impl/std/stdlib.h>

#include <dep/hde/hde.h>
#include <dep/hexrays/hexrays.h>
#include <dep/oxorany/include.h>

//#include <impl/obfus/obfus.hxx>
//#include <impl/obfus/bogus.hxx>

#include <src/ntoskrnl/ia32/ia32.h>
#include <src/ntoskrnl/ntoskrnl.hxx>
#include <src/ntoskrnl/exports/exports.hxx>

#include <src/mmu/mmu.hxx>
#include <src/mmu/core/tlb.hxx>
#include <src/mmu/core/ssdt.hxx>

#include <src/paging/pagetables.h>
#include <src/mmu/core/phys.hxx>

#include <src/paging/hide/hide.hxx>
#include <src/paging/dpm/dpm.hxx>
#include <src/paging/paging.hxx>
#include <src/paging/guard.hxx>

#include <src/paging/ptm/ptm.hxx>
#include <src/paging/pth/pth.hxx>

#include <src/mmu/module/module.hxx>
#include <src/mmu/process/process.hxx>
#include <src/paging/hyperspace/hyperspace.hxx>

#include <src/handler/client/control.h>
#include <src/handler/client/root/root.hxx>
#include <src/handler/client/client.hxx>

#include <src/mmu/core/eac/structs.h>
#include <src/mmu/core/eac/memory.hxx>
#include <src/mmu/core/eac/eac.hxx>

#include <src/mmu/core/pg.hxx>
#include <src/mmu/core/excep/excep.hxx>
#include <src/mmu/core/excep/stack.hxx>

#include <src/mmu/core/nmi/hook.hxx>
#include <src/mmu/core/nmi/nmi.hxx>

#include <src/mmu/core/etw/etw.hxx>
#include <src/mmu/core/etw/hook.hxx>

#include <src/mmu/core/kep/kep.hxx>
#include <src/mmu/core/kep/hook.hxx>

#include <src/mmu/core/lbr.hxx>
#include <src/mmu/core/vad.hxx>
#include <src/mmu/core/callback.hxx>
#include <src/handler/handler.hxx>