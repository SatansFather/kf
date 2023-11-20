#include "k_assert.h"
#include "kstring.h"

#if _WIN32
#include "comdef.h"
#include "engine/os/windows/windows.h"
#endif
#include "../game_instance.h"

KAssertion::KAssertion(const char* msg)
{
#if _SERVER
	LOG(msg);
#elif _WIN32
	MessageBoxA(NULL, msg, "Assertion Failed", MB_OK);
#endif
	
//#if _DEBUG
	throw;
//#endif	

#if !_COMPILER
	exit(0);
	//KGameInstance::Get().ExitGame();
#endif
}

#if _WIN32

KAssertion::KAssertion(HRESULT hresult, const char* msg)
{
	_com_error err(hresult);
	LPCTSTR hr_msg = err.ErrorMessage();

	KString str = KString(msg + KString("\n") + hr_msg);

	MessageBoxA(NULL, str.CStr(), "Assertion Failed", MB_OK);

//#if _DEBUG
	throw;
//#endif	

#if !_COMPILER
	exit(0);
	//KGameInstance::Get().ExitGame();
#endif
}
#endif

