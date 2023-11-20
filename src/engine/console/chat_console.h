#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "text_console.h"

class KChatConsole : public KTextConsole
{
	using KTextConsole::KTextConsole;

public:

	KChatConsole();

	void OnMessageAdded() override;

#if !_COMPILER
	void ProcessSubmission(const KString& str) override;
#endif
};

KChatConsole* GetChatConsole();

#endif