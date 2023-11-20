#include "k_main.h"
#include "engine/game_instance.h"
#include "engine/utility/k_assert.h"
#include "engine/system/terminal/terminal.h"

#if _WIN32 && !_COMPILER && !_SERVER
UPtr<KSplashScreen> KApplication::SplashScreen;
#endif

#if _COMPILER
#include "compiler/compiler.h"
#endif

#if !_SERVER
// a glfw window is used for consistent input tracking across all rendering APIs
#include "GLFW/glfw3.h"
#endif

/////////////////////////////////
// DELETE
//#include "sol/sol.hpp"
#include "../global/paths.h"
#include "functional"
#include <ctime>
/////////////////////////////////

bool KApplication::bMainCalled = false;
TMap<KString, KString> KApplication::ParsedCommandLine;
u32 KApplication::CoreCount;

#if _WIN32
HINSTANCE KApplication::hInstance;
void KApplication::KarnageMain(const KString& commandLine, HINSTANCE instance)
{
#if !_COMPILER && !_SERVER && !_PACK && !_DEBUG
	SplashScreen = std::make_unique<KSplashScreen>();
#elif _PACK
	KSplashScreen::WriteImageToHeader();
#endif

	RTL_OSVERSIONINFOW version = GetRealOSVersion();
	LOG("Windows Version "
		+ KString((u32)version.dwMajorVersion) + "."
		+ KString((u32)version.dwMinorVersion)
		+ " Build " + KString((u32)version.dwBuildNumber));
		
	hInstance = instance;
	SetProcessDPIAware(); // d2d does weird shit with desktop dpi scaling otherwise
	KarnageMain(commandLine);
}
#endif

void KApplication::KarnageMain(const KString& commandLine)
{
	K_ASSERT(!bMainCalled, "Tried calling KarnageMain() a second time");
	bMainCalled = true;

	CoreCount = std::thread::hardware_concurrency();
	if (CoreCount == 0) CoreCount = 1;
	LOG("Detected " + KString(CoreCount) + " CPU cores");

	ParseCommandLine(commandLine);

#if !_SERVER && !_COMPILER && !_PACK
	if (HasCommandLineArg("log"))
		KSystemTerminal::Show();
#endif

#if !_SERVER
	// init (and terminate) glfw here so it happens only once and lasts the life of the application
	// map compiler will need glfw for global illumination (needs a renderer) even though nothing will be shown
	glfwInit();
#endif

#if !_COMPILER
	KGameInstance::Get().RunGame();
#else

#if _DEBUG
	KMapCompiler::Get().CompileMap("D:\\karnagefreak\\res\\maps\\pingtest.map");
#else
	KMapCompiler::Get().CompileMap(KApplication::GetCommandLineArg("map"));
#endif

#endif

#if !_SERVER
	glfwTerminate();
#endif
}

void KApplication::PreMain()
{
#if _SERVER || _COMPILER || _PACK
	KSystemTerminal::Show();
#else
	//KGameInstance::Get().InitConsoles(); // need this to start logging immediately
#endif

	LOG("Karnage Freak v0.0 - Compiled on " + KString(__DATE__) + " at " + KString(__TIME__), 1, 1, 0);

	auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	KString time = KString(std::ctime(&now));
	time.ReplaceCharInline('\n', '\0');
	LOG(time);
}

bool KApplication::HasCommandLineArg(const KString& arg)
{
	return ParsedCommandLine.count(arg);
}

KString KApplication::GetCommandLineArg(const KString& arg)
{
	if (HasCommandLineArg(arg)) return ParsedCommandLine[arg];
	else return "";
}

void KApplication::ParseCommandLine(const KString& cmd_line)
{
	if (cmd_line.IsEmpty())
	{
		LOG("Command line is empty");
		return;
	}

	TVector<KString> args;
	KString buffer;
	for (char c : cmd_line)
	{
		if (c == '-')
		{
			args.push_back(buffer);
			buffer = "";
		}
		else
		{
			buffer += c;
		}
	}
	args.push_back(buffer);

	for (KString& str : args)
	{
		str.TrimInPlace();

		if (str == "") continue;

		TVector<KString> keyval;
		if (str.Contains("="))
		{
			str.SplitByChar(keyval, '=');
		}

		switch (keyval.size())
		{
			case 0:
			{
				ParsedCommandLine[str] = "";
				break;
			}
			case 1:
			{
				ParsedCommandLine[keyval[0]] = "";
				break;
			}
			case 2:
			{
				ParsedCommandLine[keyval[0]] = keyval[1];
				break;
			}
			default:
			{
				// improperly formed command
				break;
			}
		}
	}

	LOG("Command line args:");
	for (const KString& arg : args)
	{
		LOG("\t" + arg);
	}
}


