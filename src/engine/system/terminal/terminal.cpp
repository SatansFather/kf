#include "terminal.h"
#include "engine/run/k_main.h"

void _SYSLOG(const KString& message)
{
	SYSLOG(message);
}
void _SYSLOG_WARNING(const KString& message)
{
	SYSLOG_WARNING(message);
}
void _SYSLOG_ERROR(const KString& message)
{
	SYSLOG_ERROR(message);
}

bool KSystemTerminal::bIsShowing = false;

void KSystemTerminal::SetShowing(bool showing)
{
	if (showing && !bIsShowing)
	{
		Show();
	}
	else if (!showing)
	{
		Hide();
	}
}

bool KSystemTerminal::IsShowing()
{
	return bIsShowing;
}

void KSystemTerminal::Show()
{
	if (bIsShowing) return;

#ifdef WIN32


#if _COMPILER
	if (KApplication::HasCommandLineArg("redirect"))
		return;
#endif
#if _PACK && !_DEBUG
	if (!KApplication::HasCommandLineArg("newconsole"))
		AttachConsole(ATTACH_PARENT_PROCESS);
	else
#endif
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO     cursor_info;
	GetConsoleCursorInfo(out, &cursor_info);
	cursor_info.bVisible = false;
	SetConsoleCursorInfo(out, &cursor_info);
#endif

#ifdef __linux__
	
#endif

	bIsShowing = true;
}

void KSystemTerminal::Hide()
{
	if (!bIsShowing) return;

#ifdef WIN32
	FreeConsole();
#endif

#ifdef __linux__
	
#endif

	bIsShowing = false;

}
