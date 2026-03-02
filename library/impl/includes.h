#pragma once
#include <chrono>
#include <ctime>
#include <vector>
#include <Windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <vector>
#include <winternl.h>
#include <cstdint>
#include <DbgHelp.h>
#include <thread>
#include <functional>
#include <map>
#include <algorithm>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

#include <dep/oxorany/include.h>

#include <src/utilities/utilities.hxx>
#include <src/utilities/logger/logger.hxx>

#include <src/ntoskrnl/ia32.h>
#include <src/ntoskrnl/pdb/pdb.hxx>
#include <src/ntoskrnl/ntoskrnl.hxx>
auto g_ntoskrnl = std::unique_ptr< kernel::c_module >( );

#include <src/driver/control.h>
#include <src/driver/driver.hxx>
#include <src/driver/exit.hxx>

#pragma init_seg(lib)
static hyperspace::exit::c_exit g_exit_handler;