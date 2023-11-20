#ifdef WIN32
#include "k_main.h"
#include "engine/os/windows/windows.h"

int APIENTRY wWinMain(
	_In_ HINSTANCE		hInstance,
	_In_opt_ HINSTANCE	hPrevInstance,
	_In_ LPWSTR			lpCmdLine,
	_In_ int			nCmdShow)
{
	KApplication::PreMain();
	KApplication::KarnageMain(KString(lpCmdLine), hInstance);
	return 0; 
}
#endif

#if 0
#ifdef WIN32

#include "net/n_interface.h"
#include "core/c_window.h"
#include "core/c_game.h"
#include "core/c_input.h"
#include "core/c_cfg.h"
#include "core/c_console.h"
#include <thread>
#include <atomic>
#include "render/r_font.h"
#include "utility/u_thread.h"
#include "core/c_demo.h"
#include "kf.h"

// stack overflow copypaste to get OS version
typedef LONG NTSTATUS, * PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)
typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
RTL_OSVERSIONINFOW GetRealOSVersion() 
{
	HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
	if (hMod) 
	{
		RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
		if (fxPtr != nullptr) 
		{
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			if (STATUS_SUCCESS == fxPtr(&rovi)) 
			{
				return rovi;
			}
		}
	}
	RTL_OSVERSIONINFOW rovi = { 0 };
	return rovi;
}

int APIENTRY wWinMain(
	_In_ HINSTANCE		hInstance,
	_In_opt_ HINSTANCE	hPrevInstance,
	_In_ LPWSTR			lpCmdLine,
	_In_ int			nCmdShow)
{
#if _DEBUG
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
	
	FilePutContents("startup.log", "Karnage Freak - version " + VersionNumber);

	RTL_OSVERSIONINFOW version = GetRealOSVersion();
	FilePutContents("startup.log", "\nWindows Version " 
		+ STR(version.dwMajorVersion) + "." 
		+ STR(version.dwMinorVersion)
		+ " Build " + STR(version.dwBuildNumber), true);

	const u32 core_count = std::thread::hardware_concurrency();
	FilePutContents("startup.log", "\n" + STR(core_count) + " CPU cores detected", true);

#if !_SERVER
	SetProcessDPIAware();
#endif

	KarnageMain();

	return 0;
}

#endif
#endif