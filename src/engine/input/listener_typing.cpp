#if !_SERVER

#include "listener_typing.h"
#include "event.h"
#include "engine/utility/kstring.h"
#include "input_processor.h"
#include "GLFW/glfw3.h"
#include "glfw_input.h"

void KTypingInputListener::ProcessKey(KInputKeyEvent e)
{
#if !_SERVER

	KKeyboardInputListener::ProcessKey(e);

	if (e.GetInput() == 'V' && e.GetMods() & GLFW_MOD_CONTROL)
		PasteClipboard();
#endif
}

void KTypingInputListener::ProcessCharacter(u32 c)
{
	OnTextAdded(KString::FromChar(c));
}

void KTypingInputListener::OnTextAdded(const class KString& text)
{
	KString t = text;
	t.ReplaceCharInline('\n', ' ');
	CurrentString.Insert(Cursor, t);
	StringUpdated();
	UpdateCursor(Cursor + t.Size());
}

void KTypingInputListener::PasteClipboard()
{
	KString clipboard = KGLFWInput::GetClipboard().Trim();
	if (!clipboard.IsEmpty())
		OnTextAdded(clipboard);
}

i32 KTypingInputListener::NextWord(bool forward)
{
	// cursor target position is AFTER a space
	// if going backward and already sitting in front of a space, go over that one

	if (forward)
	{
		bool found_space = false;
		for (i32 i = Cursor; i < CurrentString.Size(); i++)
		{
			if (!found_space)
			{
				if (CurrentString[i] == ' ') found_space = true;
			}
			else
			{
				if (CurrentString[i] != ' ') return i;
			}
		}
		return CurrentString.Size();
	}
	else
	{
		bool found_letter = false;
		for (i32 i = Cursor; i >= 0; i--)
		{
			if (i == 0) return 0;

			if (!found_letter)
			{
				if (CurrentString[i - 1] != ' ') found_letter = true;
			}
			else
			{
				if (CurrentString[i] == ' ') return i + 1;
			}
		}
	}

	return 0;
};

void KTypingInputListener::UpdateCursor(i32 pos)
{
	Cursor = std::clamp(pos, 0, i32(CurrentString.Size()));
}

void KTypingInputListener::OnBackspace(i32 mods)
{
	if (CurrentString.IsEmpty()) return;
	if (Cursor == 0) return;

	if (mods & Ctrl)
	{
		i32 start = Cursor;
		UpdateCursor(NextWord(false));
		CurrentString.Erase(Cursor, start - Cursor);
		StringUpdated();
	}
	else
	{
		CurrentString.Erase(Cursor - 1, 1);
		UpdateCursor(Cursor - 1);
		StringUpdated();
		return;
	}
}

void KTypingInputListener::OnLeftArrow(i32 mods)
{
	if (mods == 0)
		UpdateCursor(Cursor - 1);
	else
		UpdateCursor(NextWord(false));
}

void KTypingInputListener::OnRightArrow(i32 mods)
{
	if (mods == 0)
		UpdateCursor(Cursor + 1);
	else
		UpdateCursor(NextWord(true));
}

void KTypingInputListener::OnEnd(i32 mods)
{
	i32 pre = Cursor;
	UpdateCursor(CurrentString.Size());
	if (mods & Shift)
		UpdateSelection(pre, CurrentString.Size());
}

void KTypingInputListener::OnHome(i32 mods)
{
	i32 pre = Cursor;
	UpdateCursor(0);
	if (mods & Shift)
		UpdateSelection(0, pre);
}

void KTypingInputListener::UpdateSelection(i32 start, i32 end)
{
	if (start == end)
	{
		SelectionStart = -1;
		SelectionEnd = -1;
		return;
	}

	SelectionStart = start;
	SelectionEnd = end;
}

#endif