#if !_SERVER

#include "engine/render/interface/render_interface.h"
#include "engine/render/interface/window.h"
#include "input_processor.h"
#include "engine/utility/k_assert.h"
#include "engine/utility/kstring.h"
#include "event.h"
#include "glfw_input.h"
#include "GLFW/glfw3.h"

EMouseMoveType MouseMoveType = EMouseMoveType::Move;

void KGLFWInput::InitGlfwInput()
{
	K_ASSERT(GetRenderInterface(), "no render interface at the time of input initialization");

	GLFWwindow* win = GetRenderInterface()->GetGameWindow()->GetGlfwWindow();

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	else
	{
		// well that sucks lol
	}

	HideMouseCursor();
	
	glfwSetKeyCallback(win, KGLFWInput::KeyCallback);
	glfwSetCharCallback(win, KGLFWInput::CharCallback);
	glfwSetMouseButtonCallback(win, KGLFWInput::MouseCallback);
	glfwSetScrollCallback(win, KGLFWInput::ScrollCallback);
	glfwSetCursorPosCallback(win, KGLFWInput::CursorCallback);
	glfwSetWindowFocusCallback(win, KGLFWInput::FocusCallback);
}

bool KGLFWInput::WaitGlfwEvents()
{
	K_ASSERT(GetRenderInterface(), "no render interface at the time of input polling");

	GLFWwindow* win = GetRenderInterface()->GetGameWindow()->GetGlfwWindow();

	glfwWaitEvents();

	// will signal the game loop to quit
	if (glfwWindowShouldClose(win)) return false; 

	CheckStateKeys(win);

	// if cursor is disabled, reset position
	// this keeps hidden mouse centered and allows us to pass the cursor position as the delta
	if (glfwGetInputMode(win, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		glfwSetCursorPos(win, 0, 0);
	}

	return true;
}

void KGLFWInput::KeyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
	if (action == GLFW_RELEASE) return;
	KInputProcessor::ProcessKeyInput(key, mods);
}

void KGLFWInput::CharCallback(struct GLFWwindow* window, u32 c)
{
	KInputProcessor::ProcessCharInput(c);
}

void KGLFWInput::MouseCallback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
	if (action != GLFW_RELEASE) return;
	KInputProcessor::ProcessKeyInput(button, mods);
}

void KGLFWInput::ScrollCallback(GLFWwindow* window, f64 xoffset, f64 yoffset)
{
	GLFWwindow* win = GetRenderInterface()->GetGameWindow()->GetGlfwWindow();

	i32 mods = 0;
	mods |= (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT)) ? GLFW_MOD_SHIFT : 0;
	mods |= (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL)) ? GLFW_MOD_CONTROL : 0;
	mods |= (glfwGetKey(win, GLFW_KEY_LEFT_ALT) || glfwGetKey(win, GLFW_KEY_RIGHT_ALT)) ? GLFW_MOD_ALT : 0;

	KInputProcessor::ProcessKeyInput(
	  (yoffset > 0) ? 
		KINPUT_SCROLL_UP 
	: (yoffset < 0) ? 
		KINPUT_SCROLL_DOWN 
	: -1, mods);
}

void KGLFWInput::CursorCallback(GLFWwindow* window, f64 xpos, f64 ypos)
{
	// we can represent position as delta because we reset to (0, 0) after each poll
	KInputProcessor::ProcessMouseInput(xpos, ypos, MouseMoveType);
	MouseMoveType = EMouseMoveType::Move;
}

void KGLFWInput::FocusCallback(struct GLFWwindow* window, int focused)
{
	GetRenderInterface()->GetGameWindow()->bIsFocused = focused;
}

void KGLFWInput::CheckStateKeys(GLFWwindow* win)
{
	u8 state = 0;

	for (u8 i = u8(EInputAction::STATE_START) + 1; i < u8(EInputAction::STATE_END); i++)
	{
		if (KInputProcessor::IsActionPressed(EInputAction(i), win))
		{		
			// convert input bindings to key flags
			if (i == 1) state |= i;
			else state |= u8(pow(2, i32(i - 1)));
		}
	}

	KInputProcessor::ProcessKeyState(state, KInputProcessor::IsActionPressed(EInputAction::ShowScoreboard, win));
}

void KGLFWInput::ShowMouseCursor()
{
	MouseMoveType = EMouseMoveType::Show;
	glfwSetInputMode(GetRenderInterface()->GetGameWindow()->GetGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void KGLFWInput::HideMouseCursor()
{
	MouseMoveType = EMouseMoveType::Show;
	glfwSetInputMode(GetRenderInterface()->GetGameWindow()->GetGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void KGLFWInput::CenterMouseCursor()
{
	MouseMoveType = EMouseMoveType::Set;
	glfwSetCursorPos(GetRenderInterface()->GetGameWindow()->GetGlfwWindow(), 
		GetViewportX() / 4, GetViewportY() / 4);
}

bool KGLFWInput::MouseIsShowing()
{
	return glfwGetInputMode(GetRenderInterface()->GetGameWindow()->GetGlfwWindow(), GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

bool KGLFWInput::LeftMousePressed()
{
	return glfwGetMouseButton(GetRenderInterface()->GetGameWindow()->GetGlfwWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

KString KGLFWInput::GetClipboard()
{
	const char* c = glfwGetClipboardString(GetRenderInterface()->GetGameWindow()->GetGlfwWindow());
	if (c) return KString(c);
	return KString("");
}

void KGLFWInput::SetClipboard(const KString& text)
{
	glfwSetClipboardString(GetRenderInterface()->GetGameWindow()->GetGlfwWindow(), text.CStr());
}

#endif
