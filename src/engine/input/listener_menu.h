#pragma once

#include "listener_keyboard.h"

class KMenuInputListener : public KKeyboardInputListener
{
#if !_SERVER
	friend class KBasicMenu;

	class KBasicMenu* Menu = nullptr;

	void ProcessKey(class KInputKeyEvent e) override;
	void OnSet() override;

	void OnDownArrow(i32 mods) override;
	void OnScrollDown(i32 mods) override;
	void OnPageDown(i32 mods) override;

	void OnUpArrow(i32 mods) override;
	void OnScrollUp(i32 mods) override;
	void OnPageUp(i32 mods) override;

	void OnLeftArrow(i32 mods) override;
	void OnRightArrow(i32 mods) override;
	void OnHome(i32 mods) override;
	void OnEnd(i32 mods) override;

	void OnSubmit(i32 mods) override;
	void OnClicked();

	void OnCursorMoved(f32 x, f32 y, EMouseMoveType type) override;

	void ScrollSelection(i32 count);

	void ExecuteSelection();

	i32 GetScrollJump(i32 mods);

public: 
	bool ShouldUseMouseSeletion() const;
#else
	void ProcessKey(class KInputKeyEvent e) override {}
	void OnSet() override {}
	void OnDownArrow(i32 mods) override {}
	void OnScrollDown(i32 mods) override {}
	void OnPageDown(i32 mods) override {}
	void OnUpArrow(i32 mods) override {}
	void OnScrollUp(i32 mods) override {}
	void OnPageUp(i32 mods) override {}
	void OnLeftArrow(i32 mods) override {}
	void OnRightArrow(i32 mods) override {}
	void OnHome(i32 mods) override {}
	void OnEnd(i32 mods) override {}
	void OnSubmit(i32 mods) override {}
	void OnClicked() {}
	void OnCursorMoved(f32 x, f32 y, EMouseMoveType type) override {}
	void ScrollSelection(i32 count) {}
	void ExecuteSelection() {}
	i32 GetScrollJump(i32 mods) {}
	bool ShouldUseMouseSeletion() const {}
#endif
};
