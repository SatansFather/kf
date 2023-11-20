#if !_SERVER && _WIN32

#include "d3d11_window.h"

// a glfw window is used for consistent messagepump/scancodes across all APIs
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "../../draw_config.h"

void KGameWindow_D3D11::OnResize()
{
	// https://docs.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers
	
}

UPtr<KGameWindow_D3D11> KGameWindow_D3D11::CreateGameWindowD3D(u32 resX, u32 resY, u8 state)
{
	UPtr<KGameWindow_D3D11> window = std::make_unique<KGameWindow_D3D11>();

	window->Width = resX;
	window->Height = resY;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#if _COMPILER	
	bool full = false;
#else
	bool full = GetDrawConfig()->WindowState == EWindowState::Fullscreen;
#endif
	window->GlfwWindow = glfwCreateWindow(resX, resY, "Karnage Freak", full ? glfwGetPrimaryMonitor() : NULL, NULL);
	window->hWnd = glfwGetWin32Window(window->GlfwWindow);
	glfwSetWindowUserPointer(window->GlfwWindow, window.get());

	if (!full) window->MoveToCenter();
	
	window->SetWindowState((EWindowState)state);
	window->OnCreation();

	return window;
}


#endif
