#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "menu_option.h"
#include "../render/font_usage.h"

class KBasicMenu
{
	friend class KGameInstance;
	friend class KHudMenu;
	friend class KMenuInputListener;

	static TVector<KBasicMenu*> MenuStack;

	KString Header;
	std::atomic<bool> bIsShowing = { false };
	std::atomic<i32> SelectedOption = { 0 };
	UPtr<class KMenuInputListener> Listener;

protected:

#if !_SERVER
	TVector<UPtr<KMenuOption>> Options;
#endif

public:

	static TVector<KBasicMenu*> AllMenus;

	std::mutex OptionsLock;

	KBasicMenu();
	virtual ~KBasicMenu();

	void OpenMenu();
	void CloseMenu();

	bool IsShowing() const { return bIsShowing; }
	u32 GetSelectedOption() const { return SelectedOption; }
	u32 GetOptionCount() const { return Options.size(); }
	const KString& GetOptionText(u32 option) { return Options[option]->Text; }

	void RunSelectedOption();

	// can be called from render thread during mouse movement
	void UpdateSelectedOption(u32 option);

	bool ShouldUseMouseSelection() const;
	void GetCursorPosition(f32& x, f32& y) const;

	i32 GetFirstSelectableIndex() const;

	static KBasicMenu* GetTopMenu();
	static void CloseAllMenus();

	class KMenuOption* AddBackButton();

	virtual void OnOpened() {}
	virtual void OnClosed() {}
};

KBasicMenu* GetMainMenu();
KBasicMenu* GetIngameMenu();
KBasicMenu* GetOptionsMenu();
KBasicMenu* GetServerBrowser();
KBasicMenu* GetMapsMenu();
KBasicMenu* GetJoinMenu();

#endif