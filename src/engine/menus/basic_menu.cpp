#if !_SERVER
#include "basic_menu.h"
#include "engine/input/input_processor.h"
#include "engine/input/listener_menu.h"
#include "../../engine/game/match.h"
#include "../game_instance.h"
#include "../audio/sound_asset.h"
#include "../audio/audio.h"
#include "../net/server_browser.h"


TVector<KBasicMenu*> KBasicMenu::MenuStack;
TVector<KBasicMenu*> KBasicMenu::AllMenus;

KBasicMenu* GetMainMenu()
{
	static KBasicMenu menu;
	return &menu;
}

KBasicMenu* GetIngameMenu()
{
	static KBasicMenu menu;
	return &menu;
}

KBasicMenu* GetOptionsMenu()
{
	static KBasicMenu menu;
	return &menu;
}

KBasicMenu* GetServerBrowser()
{
	class KServerBrowserMenu : public KBasicMenu
	{
	public:
		void OnServersUpdated()
		{
			OptionsLock.lock();
			KServerBrowser::Get().bQueryingMaster = true;

			Options.clear();
			for (auto& result : KServerBrowser::Get().QueryResults)
			{
				auto option = std::make_unique<KMenuOption>();
				option->Text = result.Name;
				auto connect = [&]() -> void 
				{ 
					KGameInstance::Get().ConnectToServer(result.Address);
					KBasicMenu::CloseAllMenus();
				};
				option->RunFunction = connect;
				Options.push_back(std::move(option));
			}
			AddBackButton();

			KServerBrowser::Get().bQueryingMaster = false;
			OptionsLock.unlock();
		}
		
		KServerBrowserMenu()
		{
			KServerBrowser::Get().AddCallback(std::bind(&KServerBrowserMenu::OnServersUpdated, this));
		}

		void OnOpened() override
		{
			OptionsLock.lock();
			Options.clear();
			AddBackButton();
			OptionsLock.unlock();

			KServerBrowser::Get().QueryMasterServer();
		}
	};

	static KServerBrowserMenu menu;
	return &menu;
}

KBasicMenu* GetMapsMenu()
{
	static KBasicMenu menu;
	return &menu;
}

KBasicMenu* GetJoinMenu()
{
	static KBasicMenu menu;
	return &menu;
}

KBasicMenu::KBasicMenu()
{
	Listener = std::make_unique<KMenuInputListener>();
	Listener->Menu = this;
	AllMenus.push_back(this);
}

KBasicMenu::~KBasicMenu() {}

void KBasicMenu::OpenMenu()
{
	if (!bIsShowing)
	{
		KGameInstance::Get().AddPause();
		KInputProcessor::PushListenerStack(Listener.get());
		
		if (SelectedOption >= 0 && SelectedOption < Options.size())
		{
			if (!Options[SelectedOption]->bActive || 
				!Options[SelectedOption]->bSelectable ||
				SelectedOption == Options.size() - 1) // always a back or quit button
				SelectedOption = GetFirstSelectableIndex();
		}
		else
			SelectedOption = GetFirstSelectableIndex();
		
		bIsShowing = true;
		KAudio::PlaySound(KSoundID::Menu_Open, 1);
		MenuStack.push_back(this);
		OnOpened();
	}
}

void KBasicMenu::CloseMenu()
{ 
	if (bIsShowing)
	{
		bIsShowing = false;
		KInputProcessor::RemoveListenerFromStack(Listener.get());
		KGameInstance::Get().RemovePause();
		KAudio::PlaySound(KSoundID::Menu_Close, 1);
		VectorRemove(MenuStack, this);
		OnClosed();
	}
}

void KBasicMenu::RunSelectedOption()
{
	Options[SelectedOption]->ExecuteOption();
}

void KBasicMenu::UpdateSelectedOption(u32 option)
{
	SelectedOption = option;
}

bool KBasicMenu::ShouldUseMouseSelection() const
{
	return Listener->ShouldUseMouseSeletion();
}

void KBasicMenu::GetCursorPosition(f32& x, f32& y) const
{
	Listener->GetCursorPosition(x, y);
}

i32 KBasicMenu::GetFirstSelectableIndex() const
{
	for (i32 i = 0; i < Options.size(); i++)
	  if (Options[i]->bActive && Options[i]->bSelectable)
		return i;

	return -1;
}

KBasicMenu* KBasicMenu::GetTopMenu()
{
	if (MenuStack.size() == 0) return nullptr;
	return MenuStack[MenuStack.size() - 1];
}

void KBasicMenu::CloseAllMenus()
{
	while (KBasicMenu* m = GetTopMenu())
		m->CloseMenu();
}

KMenuOption* KBasicMenu::AddBackButton()
{
	auto option = std::make_unique<KMenuOption>();
	option->FontUsage = EFontUsage::MenuButtonSmall;
	option->Text = "Back";
	const auto back = [&]() -> void { CloseMenu(); };
	option->RunFunction = back;
	KMenuOption* ret = option.get();
	Options.push_back(std::move(option));
	return ret;
}

#endif