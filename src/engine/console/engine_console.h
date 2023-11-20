#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "text_console.h"

class KEngineConsole : public KTextConsole
{
	using KTextConsole::KTextConsole;

public:

	TVector<KString>* AutoSuggestions = nullptr;
	KTimePoint LastAutoCompleteUpdateTime;

	KEngineConsole();

	void InitAutocompleteMaps();

	// the last command was "servers" telling the "connect" command to favor query numbers instead of favorites
	bool bLastWasServers = false;
	
	// prints the return value of the console command
	bool bPrintBack = true;

	void OnTextUpdated() override;

#if !_COMPILER
	void ProcessSubmission(const KString& str) override;
#endif
};

KEngineConsole* GetEngineConsole();
void ExecuteConsoleCommand(const KString& command);

#endif