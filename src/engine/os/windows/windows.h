#pragma once

#if _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

RTL_OSVERSIONINFOW GetRealOSVersion();
HINSTANCE GetAppInstanceHandle();
bool CheckHR(HRESULT hr);

// used for enet
#pragma comment(lib, "ws2_32.lib")

#endif