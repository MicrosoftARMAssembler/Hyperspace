#include <iostream>
#include <Windows.h>

extern "C" BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    MessageBoxA( 0, 0, 0, 0 );

    return TRUE;
}

