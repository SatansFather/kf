#if !_SERVER

#include "listener_console.h"
#include "engine/console/engine_console.h"
#include "event.h"
#include "input_processor.h"

void KConsoleInputListener::OnEscape(i32 mods)
{
	CurrentString = "";
	Cursor = 0;
	Console->UpdateSubmissionString(CurrentString);
	Console->UpdateCursor(Cursor);
	Console->CloseConsole();
}

void KConsoleInputListener::ProcessKey(KInputKeyEvent e)
{
	if (e.GetEvent() == EInputAction::ShowConsole)
	{
		if (bIsChat)
		{}//	GetEngineConsole()->OpenConsole();
		else
			Console->CloseConsole();
	}
	else
		KTypingInputListener::ProcessKey(e);
}	

void KConsoleInputListener::OnSet()
{
	KInputProcessor::HideMouseCursor();
}

void KConsoleInputListener::OnSubmit(i32 mods)
{
#if !_COMPILER
	//Console->AddMessage(CurrentString);
	Console->ProcessSubmission(CurrentString);
	CurrentString = "";
	Cursor = 0;
	Console->UpdateSubmissionString(CurrentString);
	Console->UpdateCursor(Cursor);
	Console->ResetCommandScroll();
#endif
}

void KConsoleInputListener::OnScrollUp(i32 mods)
{
	Console->AddScroll(1);
}

void KConsoleInputListener::OnScrollDown(i32 mods)
{
	Console->AddScroll(-1);
}

void KConsoleInputListener::OnPageDown(i32 mods)
{
	Console->AddScroll(-8);
}

void KConsoleInputListener::OnPageUp(i32 mods)
{
	Console->AddScroll(8);
}

void KConsoleInputListener::OnUpArrow(i32 mods)
{
	if (mods & EModifierState::Ctrl)
	{
		OnScrollUp(0);
		return;
	}

	if (Console->SubmittedCommandScrollIndex == -1)
		Console->SubmittedCommandScrollIndex = Console->SubmittedCommands.size() - 1;
	else
		Console->SubmittedCommandScrollIndex--;

	KString str = "";
	if (Console->SubmittedCommandScrollIndex >= 0)
		str = Console->SubmittedCommands[Console->SubmittedCommandScrollIndex];

	CurrentString = str;
	Console->UpdateSubmissionString(str);
	UpdateCursor(str.Size());
}

void KConsoleInputListener::OnDownArrow(i32 mods)
{
	if (mods & EModifierState::Ctrl)
	{
		OnScrollDown(0);
		return;
	}

	Console->SubmittedCommandScrollIndex++;
	if (Console->SubmittedCommandScrollIndex == Console->SubmittedCommands.size())
		Console->ResetCommandScroll();

	KString str = "";
	if (Console->SubmittedCommandScrollIndex >= 0)
		str = Console->SubmittedCommands[Console->SubmittedCommandScrollIndex];

	CurrentString = str;
	Console->UpdateSubmissionString(str);
	UpdateCursor(str.Size());
}

void KConsoleInputListener::StringUpdated()
{
	Console->UpdateSubmissionString(CurrentString);
	Console->ResetCommandScroll();
}

void KConsoleInputListener::UpdateCursor(i32 pos)
{
	KTypingInputListener::UpdateCursor(pos);
	Console->UpdateCursor(pos);
}

#endif