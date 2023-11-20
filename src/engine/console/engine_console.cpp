#if !_SERVER

#include "engine_console.h"
#include "engine/game_instance.h"
#include "engine/input/listener_console.h"
#include "engine/input/input_processor.h"
#include "engine/game_instance.h"

KEngineConsole* GetEngineConsole()
{
	static KEngineConsole console;
	return &console;
}

void ExecuteConsoleCommand(const KString& command)
{
	GetEngineConsole()->bPrintBack = false;
	GetEngineConsole()->ProcessSubmission(command);
	GetEngineConsole()->bPrintBack = true;
}

KString CCOM_Clear(const KString& val)
{
	GetEngineConsole()->ClearConsole();
	return "";
}

#if !_PACK
void LOG(const class KString& message, GFlt r /*= 1*/, GFlt g /*= 1*/, GFlt b /*= 1*/)
{
#if !_SERVER && !_COMPILER
	GetEngineConsole()->AddMessage(message, r, g, b);
#else
	_SYSLOG(message);
#endif
}
#endif


KEngineConsole::KEngineConsole()
{	
	InitAutocompleteMaps();
}

#endif