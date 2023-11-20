#pragma once

#include "listener.h"

class KKeyboardInputListener : public KInputListener
{
protected:

public:

	enum EModifierState
	{
		Shift = 1,
		Ctrl = 2,
		Alt = 4
	};

	void ProcessKey(class KInputKeyEvent e) override;

	virtual void ProcessCharacter(u32 c)    {}
	virtual void OnEnd(i32 mods)            {}
	virtual void OnHome(i32 mods)           {}
	virtual void OnBackspace(i32 mods)      {}
	virtual void OnSubmit(i32 mods)         {}
	virtual void OnEscape(i32 mods)         {}
	virtual void OnUpArrow(i32 mods)        {}
	virtual void OnDownArrow(i32 mods)      {}
	virtual void OnLeftArrow(i32 mods)      {}
	virtual void OnRightArrow(i32 mods)     {}
	virtual void OnScrollUp(i32 mods)       {}
	virtual void OnScrollDown(i32 mods)     {}
	virtual void OnPageDown(i32 mods)       {}
	virtual void OnPageUp(i32 mods)         {}
};

