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

#pragma warning( disable: 4789 )
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

#include <dep/oxorany/include.h>
#include <src/utilities/logger/logger.hxx>

#include <src/hyperspace/ia32/ia32.h>
#include <src/hyperspace/control.h>
#include <src/hyperspace/hyperspace.h>

#include <src/utilities/exception/exception.hxx>

