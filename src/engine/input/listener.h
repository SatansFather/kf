#pragma once

#include "kfglobal.h"
#include "event.h"

class KInputListener
{
protected:
	
	bool bMouseControlsCamera = false;

public:

	virtual void OnSet() {}
	virtual void OnUnset() {}
	virtual void OnRemoved() {}
	virtual void ProcessState(u8 state) {}
	virtual void ProcessScoreboard(bool show) {}
	virtual void ProcessKey(class KInputKeyEvent e) {}
	virtual void ProcessCharacter(u32 c) {}
	virtual void OnCursorMoved(f32 x, f32 y, EMouseMoveType type) {}
	
	virtual bool UsesInterfaceControls() { return true; }

	void GetCursorPosition(f32& x, f32& y) const;

	bool MouseControlsCamera() const { return bMouseControlsCamera; }

};
