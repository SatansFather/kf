#pragma once

#include "engine/utility/kstring.h"
#include "engine/global/types_container.h"

#if _WIN32
#include "engine/os/windows/windows.h"
#include "engine/os/windows/splash_screen.h"
#include "engine/global/types_ptr.h"
#endif

class KApplication
{
private:
	

#if _WIN32 && !_COMPILER && !_SERVER
	static UPtr<KSplashScreen> SplashScreen;
#endif

	static bool bMainCalled;
	static TMap<KString, KString> ParsedCommandLine;

	static u32 CoreCount;

#if _WIN32
	static HINSTANCE hInstance;
#endif

	static void ParseCommandLine(const KString& cmd_line);

public:

#if _WIN32
	static void KarnageMain(const KString& command_line, HINSTANCE instance);
	static HINSTANCE GetInstanceHandle() { return hInstance; }
#endif

	static void KarnageMain(const KString& commandLine);

	static void PreMain();

	static bool HasCommandLineArg(const KString& arg);
	static KString GetCommandLineArg(const KString& arg);
	static u32 GetCoreCount() { return CoreCount; }

#if _WIN32 && !_COMPILER && !_SERVER
	static void ClearSplashScreen() 
	{ 
		SplashScreen.reset();
	}
#endif
};