#pragma once

#include "listener_typing.h"

class KConsoleInputListener : public KTypingInputListener
{
	friend class KTextConsole;

#if !_SERVER

	class KTextConsole* Console = nullptr;
	bool bIsChat = false;

	void OnEscape(i32 mods) override;
	void ProcessKey(class KInputKeyEvent e) override;
	void OnSet() override;
	void OnSubmit(i32 mods) override;
	void OnScrollUp(i32 mods) override;
	void OnScrollDown(i32 mods) override;
	void OnPageDown(i32 mods) override; 
	void OnPageUp(i32 mods) override; 
	void OnUpArrow(i32 mods) override;
	void OnDownArrow(i32 mods) override;
public:
	void StringUpdated() override;
	void UpdateCursor(i32 pos) override;

	void SetIsChat(bool chat) { bIsChat = chat; }


#endif
};
