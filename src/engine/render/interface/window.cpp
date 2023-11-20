#if !_SERVER

#include "window.h"
#include "GLFW/glfw3.h"
#include "render_interface.h"
#include "engine/game_instance.h"

// DELETE
#include "engine/system/terminal/terminal.h"
#if !_SERVER && !_COMPILER
#include "../../console/engine_console.h"
#endif

void KGameWindow::Show()
{
#if _PACK
	Hide();
#else
	if (!bIsShowing)
	{
		glfwShowWindow(GlfwWindow);
		bIsShowing = true;
		glfwFocusWindow(GlfwWindow);
	}
#endif
}

void KGameWindow::Hide()
{
	if (bIsShowing)
	{
		glfwHideWindow(GlfwWindow);
		bIsShowing = false;
	}
}

void KGameWindow::SetWindowState(EWindowState state)
{
	WindowState = state;
	switch (state)
	{
		case EWindowState::Fullscreen:
			
			break;
		case EWindowState::Windowed:
			break;
		case EWindowState::Borderless:
			break;
	}
}

void KGameWindow::MoveToCenter()
{
	if (WindowState != EWindowState::Fullscreen)
	{
		i32 winX, winY;
		glfwGetWindowSize(GlfwWindow, &winX, &winY);

		i32 monX, monY, monPosX, monPosY;
		glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &monPosX, &monPosY, &monX, &monY);

		i32 monCenterX = monX / 2;
		i32 monCenterY = monY / 2;

		glfwSetWindowPos(GlfwWindow, monCenterX - winX / 2, monCenterY - winY / 2);
	}
}

void KGameWindow::OnCreation()
{
	glfwSetWindowSizeCallback(GlfwWindow, KGameWindow::WindowSizeCallback);
	glfwSetWindowSizeLimits(GlfwWindow, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
	glfwFocusWindow(GlfwWindow);
}

void KGameWindow::WindowSizeCallback(GLFWwindow* window, i32 width, i32 height)
{
	if (width < 1) width = 1;
	if (height < 1) height = 1;
	KGameWindow* win = (KGameWindow*)glfwGetWindowUserPointer(window);
	win->Width = width;
	win->Height = height;

#if !_COMPILER
	KGameInstance::Get().UpdateLastResizeTime();
#endif
}

#endif
