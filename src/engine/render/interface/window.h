#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "engine/render/draw_config.h"

#if _WIN32
#include "engine/os/windows/windows.h"
#endif

class KGameWindow
{
protected:

	std::atomic<u16> Width, Height;
	bool bIsShowing = false;
	EWindowState WindowState = EWindowState::Windowed;
	struct GLFWwindow* GlfwWindow = nullptr;
	
#if _WIN32
	HWND hWnd;
#endif

public:

	bool bIsFocused = true;

	virtual ~KGameWindow() {}

	void Show();
	void Hide();
	void SetWindowState(EWindowState state);
	void MoveToCenter();

protected:
	
	// must be called in child class after creating glfwwindow
	void OnCreation();

private:
	
	static void WindowSizeCallback(GLFWwindow* window, i32 width, i32 height);

public:

	virtual void OnResize() = 0;

public:

	inline EWindowState GetWindowState() const { return WindowState; }
	inline bool IsShowing() const { return bIsShowing; }
	inline u16 GetWidth() const { return Width; }
	inline u16 GetHeight() const { return Height; }
	inline struct GLFWwindow* GetGlfwWindow() { return GlfwWindow; }

	bool IsFocused() const { return bIsFocused; }
};

#endif