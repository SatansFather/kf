#if _WIN32

#include "engine/os/windows/windows.h"
#include "engine/run/k_main.h"
#include "engine/system/terminal/terminal.h"
#include "comdef.h"
#include "engine/utility/k_assert.h"

RTL_OSVERSIONINFOW GetRealOSVersion()
{
	typedef LONG NTSTATUS, * PNTSTATUS;
	typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

	HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
	if (hMod)
	{
		RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
		if (fxPtr != nullptr)
		{
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			if (0x00000000 == fxPtr(&rovi))
			{
				return rovi;
			}
		}
	}
	RTL_OSVERSIONINFOW rovi = { 0 };
	return rovi;
}

HINSTANCE GetAppInstanceHandle()
{
	return KApplication::GetInstanceHandle();
}

bool CheckHR(HRESULT hr)
{
	if (FAILED(hr))
	{
		_com_error err(hr);
		LPCTSTR msg = err.ErrorMessage();
		//SYSLOG(msg);
		//K_ASSERT(false, msg);

		return false; // dumb here
	}

	return true;
}

#endif