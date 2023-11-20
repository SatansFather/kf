#if !_SERVER

#include "listener_menu.h"
#include "input_processor.h"
#include "../console/engine_console.h"
#include "../game/match.h"
#include "engine/menus/basic_menu.h"
#include "GLFW/glfw3.h"
#include "../audio/audio.h"

void KMenuInputListener::ProcessKey(class KInputKeyEvent e)
{
	if (e.GetEvent() == EInputAction::Pause)
		Menu->CloseMenu();
	else if (e.GetEvent() == EInputAction::ShowConsole)
		GetEngineConsole()->OpenConsole();
	else if (e.GetInput() == GLFW_MOUSE_BUTTON_LEFT)
		OnClicked();
	else if (e.GetEvent() == EInputAction::MoveLeft)
		OnLeftArrow(e.GetMods());
	else if (e.GetEvent() == EInputAction::MoveRight)
		OnRightArrow(e.GetMods());
	else if (e.GetEvent() == EInputAction::MoveForward)
		OnUpArrow(e.GetMods());
	else if (e.GetEvent() == EInputAction::MoveBack)
		OnDownArrow(e.GetMods());
	else
		KKeyboardInputListener::ProcessKey(e);
}

void KMenuInputListener::OnSet()
{
	if (!KInputProcessor::MouseIsShowing())
		KInputProcessor::CenterMouseCursor();

	KInputProcessor::ShowMouseCursor();
}

void KMenuInputListener::OnDownArrow(i32 mods)
{
	ScrollSelection(1);
}

void KMenuInputListener::OnScrollDown(i32 mods)
{
	ScrollSelection(1);
}

void KMenuInputListener::OnPageDown(i32 mods)
{
	ScrollSelection(999);
}

void KMenuInputListener::OnUpArrow(i32 mods)
{
	ScrollSelection(-1);
}

void KMenuInputListener::OnScrollUp(i32 mods)
{
	ScrollSelection(-1);
}

void KMenuInputListener::OnPageUp(i32 mods)
{
	ScrollSelection(-999);
}

void KMenuInputListener::OnLeftArrow(i32 mods)
{
	i32 select = Menu->SelectedOption;
	if (select == -1) return;
	Menu->Options[select]->OnLeftArrow(GetScrollJump(mods));
}

void KMenuInputListener::OnRightArrow(i32 mods)
{
	i32 select = Menu->SelectedOption;
	if (select == -1) return;
	Menu->Options[select]->OnRightArrow(GetScrollJump(mods));
}

void KMenuInputListener::OnHome(i32 mods)
{
	i32 select = Menu->SelectedOption;
	if (select == -1) return;
	Menu->Options[select]->OnHome();
}

void KMenuInputListener::OnEnd(i32 mods)
{
	i32 select = Menu->SelectedOption;
	if (select == -1) return;
	Menu->Options[select]->OnEnd();
}

void KMenuInputListener::OnSubmit(i32 mods)
{
	ExecuteSelection();
}

void KMenuInputListener::OnClicked()
{
	ExecuteSelection();
}

void KMenuInputListener::OnCursorMoved(f32 x, f32 y, EMouseMoveType type)
{
	if (type == EMouseMoveType::Move) KInputProcessor::bUseMousePosition = true;
}

void KMenuInputListener::ScrollSelection(i32 count)
{
	KInputProcessor::bUseMousePosition = false;

	i32 option = Menu->SelectedOption;
	i32 optionStart = option;
	u32 size = Menu->Options.size();

	// 999 means page up/down
	if (count == 999) 
	{
		option = size - 1;
		count = -1;
	}
	else if (count == -999) 
	{
		option = 0;
		count = 1;
	}
	else 
	{
		option += count;
	}

	if (option == -1) option = size - 1;
	else if (option >= size) option = 0;

	// skip inactive options
	while (option != optionStart)
	{
		KMenuOption* op = Menu->Options[option].get();
		if (op->CanBeSelected()) break;
		option = (option + count) % size;
	}
	KAudio::PlaySound(KSoundID::Menu_Scroll, 1);
	Menu->SelectedOption = option;
}

void KMenuInputListener::ExecuteSelection()
{
	i32 select = Menu->SelectedOption;
	if (select == -1) return;
	Menu->Options[select]->ExecuteOption();
}

i32 KMenuInputListener::GetScrollJump(i32 mods)
{
	if (mods & EModifierState::Alt)
		return 4;
	if (mods & EModifierState::Ctrl)
		return 3;
	if (mods & EModifierState::Shift)
		return 2;
	return 1;
}

bool KMenuInputListener::ShouldUseMouseSeletion() const
{
	return KInputProcessor::bUseMousePosition;
}

#endif