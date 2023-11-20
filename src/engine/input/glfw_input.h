#pragma once

#if !_SERVER

#include "engine/global/types_numeric.h"

class KGLFWInput
{
	friend class KInputProcessor;
	static void InitGlfwInput();
	static bool WaitGlfwEvents();
	static void KeyCallback(struct GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
	static void CharCallback(struct GLFWwindow* window, u32 c);
	static void MouseCallback(struct GLFWwindow* window, i32 button, i32 action, i32 mods);
	static void ScrollCallback(struct GLFWwindow* window, f64 xoffset, f64 yoffset);
	static void CursorCallback(struct GLFWwindow* window, f64 xpos, f64 ypos);
	static void FocusCallback(struct GLFWwindow* window, int focused);
	static void CheckStateKeys(struct GLFWwindow* win);

	static void ShowMouseCursor();
	static void HideMouseCursor();
	static void CenterMouseCursor();
	static bool MouseIsShowing();
	static bool LeftMousePressed();

public:

	static class KString GetClipboard();
	static void SetClipboard(const class KString& text);
};

#endif