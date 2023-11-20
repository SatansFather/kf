#pragma once

#include "listener_keyboard.h"

class KTypingInputListener : public KKeyboardInputListener
{
protected:

	KString CurrentString;
	i32 Cursor = 0;
	i32 SelectionStart = -1;
	i32 SelectionEnd = -1;

public:

#if !_SERVER

	enum EModifierState
	{
		Shift = 1,
		Ctrl = 2,
		Alt = 4
	};

	void ProcessKey(class KInputKeyEvent e) override;
	void ProcessCharacter(u32 c) override;
	void OnBackspace(i32 mods) override;
	void OnLeftArrow(i32 mods) override;
	void OnRightArrow(i32 mods) override;
	void OnEnd(i32 mods) override;
	void OnHome(i32 mods) override;

	void UpdateSelection(i32 start, i32 end);
	i32 NextWord(bool forward);
	virtual void UpdateCursor(i32 pos);
	void OnTextAdded(const KString& text);

	void PasteClipboard();
	
	virtual void StringUpdated() = 0;

	const KString& GetString() { return CurrentString; }

#else
public:
	void ProcessState(u8 state) override {}
	void ProcessKey(class KInputKeyEvent e) override {}
#endif
};

