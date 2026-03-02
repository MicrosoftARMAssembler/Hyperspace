#include <chrono>
#include <ctime>
#include <vector>
#include <Windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <winternl.h>
#include <cstdint>
#include <DbgHelp.h>
#include <thread>
#include <functional>
#include <map>
#include <dia2.h>
#include <diacreate.h>
#include <atlbase.h>
#include <string>
#include <memory>

#pragma comment(lib, "diaguids.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

#include <dep/driver/driver.h>
#include <dep/hexrays/hexrays.h>
#include <dep/oxorany/include.h>
#include <src/utility/logger/logger.hxx>

#include <impl/ia32/ia32.h>
#include <impl/crt/crt.hxx>

#include <impl/pdb/pdb.hxx>
auto g_pdb = std::make_shared<pdb::c_pdb>( oxorany( "ntoskrnl.exe" ) );

#include <impl/asmjit/core.h>
#include <impl/asmjit/a64.h>
#include <impl/asmjit/x86.h>

#include <src/utility/utility.hxx>
#include <src/utility/module/module.hxx>
auto g_ntoskrnl = std::make_shared<utility::c_module>( );
auto g_1394ohci = std::make_shared<utility::c_module>( );

#include <src/core/esclation/driver/driver.hxx>
#include <src/core/esclation/service/service.hxx>
auto g_driver = std::make_shared<driver::c_driver>( );

#include <src/core/ntoskrnl/ntoskrnl.h>
#include <src/core/paging/pagetables.h>

#include <src/core/readwrite/readwrite.hxx>
#include <src/core/physical/physical.hxx>
#include <src/core/process/process.hxx>
#include <src/core/module/module.hxx>

#include <src/core/paging/paging.hxx>
auto g_paging = std::make_shared<paging::c_paging>( );

#include <src/core/paging/dpm/dpm.hxx>
auto g_dpm = std::make_shared<paging::c_dpm>( );

#include <src/core/paging/hyperspace/hyperspace.hxx>
auto g_hyperspace = std::make_shared<paging::c_hyperspace>( );

#include <src/core/ntoskrnl/section/section.hxx>
auto g_allocator = std::make_shared<section::c_section>( );

#include <src/core/esclation/hook/compiler/compiler.hxx>
auto g_compiler = std::make_shared<compiler::c_compiler>( );

#include <src/core/esclation/hook/hook.hxx>
auto g_hook = std::make_shared<hook::c_hook>( );

#include <src/core/ntoskrnl/exports/exports.hxx>

#include <src/map/pe.h>
#include <src/map/dependency/dependency.hxx>
#include <src/map/map.hxx>

#include <src/core/esclation/esclation.hxx>
auto g_esclation = std::make_shared<esclation::c_esclation>( );

#include <src/utility/exception/exception.hxx>