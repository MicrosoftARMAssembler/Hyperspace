#pragma once
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
#include <string>
#include <algorithm>
#pragma warning( disable: 4789 )
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")
#define target_process L"FortniteClient-Win64-Shipping.exe"

#include <dep/oxorany/include.h>
#include <src/utilities/logger/logger.hxx>
#include <src/utilities/exception/exception.hxx>
#include <src/utilities/utilities.hxx>

#include <src/hyperspace/ia32/ia32.h>
#include <src/hyperspace/control.h>
#include <src/hyperspace/hyperspace.h>

#include <src/core/pdb/pdb.hxx>
#include <src/core/dependency/imports/apiset.hxx>
auto g_apiset = std::make_unique< apiset::c_apiset >( );

#include <src/core/dependency/imports/imports.hxx>
#include <src/core/dependency/dependency.hxx>

#include <src/core/execute/shellcode.h>
#include <src/core/execute/execute.hxx>
#include <src/core/map/map.hxx>