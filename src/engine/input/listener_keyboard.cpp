#if !_SERVER

#include "listener_typing.h"
#include "event.h"
#include "engine/utility/kstring.h"
#include "input_processor.h"
#include "GLFW/glfw3.h"

void KKeyboardInputListener::ProcessKey(KInputKeyEvent e)
{
#if !_SERVER
	if (e.GetInput() == GLFW_KEY_ENTER || e.GetInput() == GLFW_KEY_KP_ENTER)
		OnSubmit(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_BACKSPACE)
		OnBackspace(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_LEFT)
		OnLeftArrow(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_RIGHT)
		OnRightArrow(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_UP)
		OnUpArrow(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_DOWN)
		OnDownArrow(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_ESCAPE)
		OnEscape(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_HOME)
		OnHome(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_END)
		OnEnd(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_PAGE_DOWN)
		OnPageDown(e.GetMods());
	else if (e.GetInput() == GLFW_KEY_PAGE_UP)
		OnPageUp(e.GetMods());
	else if (e.GetInput() == KINPUT_SCROLL_UP)
		OnScrollUp(e.GetMods());
	else if (e.GetInput() == KINPUT_SCROLL_DOWN)
		OnScrollDown(e.GetMods());
#endif
}

#endif