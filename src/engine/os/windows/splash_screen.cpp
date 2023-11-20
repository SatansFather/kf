#if _WIN32 && !_SERVER && !_COMPILER

#include "kfglobal.h"
#include "engine/os/windows/gdi.h"
#include "splash_screen.h"
#include "engine/run/k_main.h"
#include "engine/global/paths.h"

#include <fstream>

#if !_DEV
#include "splash_data.h"
#endif

/*#include "engine/render/interface/render_interface.h"
#include "engine/render/interface/window.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"*/

HWND hWnd;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

KSplashScreen::KSplashScreen()
{	
	InitGDI();

	const auto className = L"Karnage Freak";
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = KApplication::GetInstanceHandle();
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;//nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = className;
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);

	u16 resX = 512;
	u16 resY = 512;
	i32 monX = GetSystemMetrics(SM_CXSCREEN);
	i32 monY = GetSystemMetrics(SM_CYSCREEN);

	RECT wr;
	wr.left = 0;
	wr.top = 0;
	wr.right = resX;
	wr.bottom = resY;

	AdjustWindowRect(&wr, WS_POPUP, FALSE);

	int centerX = (monX - resX) / 2;
	int centerY = (monY - resY) / 2;

	hWnd = CreateWindowEx(
		WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
		className,
		L"Karnage Freak",
		WS_POPUP,
		centerX, centerY,
		wr.right - wr.left,
		wr.bottom - wr.top,
		nullptr,
		nullptr,
		KApplication::GetInstanceHandle(),
		nullptr
	);

	HRGN GGG = CreateRectRgn(0, 0, monX, monY);
	InvertRgn(GetDC(hWnd), GGG);
	SetWindowRgn(hWnd, GGG, false);

	COLORREF RRR = RGB(0, 0, 0);
	SetLayeredWindowAttributes(hWnd, RRR, (BYTE)0, LWA_COLORKEY);

	UpdateWindow(hWnd);
	ShowWindow(hWnd, SW_SHOW);
	DeleteObject(GGG);
}

KSplashScreen::~KSplashScreen()
{
	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
}

#if _PACK
void KSplashScreen::WriteImageToHeader()
{
	InitGDI();
	KString tex = TEX_DIR + "splash.png";
	Gdiplus::Bitmap bmp(tex.ToWideStr().c_str());
	Gdiplus::BitmapData bitmapData;
	Gdiplus::Rect r;
	r.X = 0;
	r.Y = 0;
	r.Width = bmp.GetWidth();
	r.Height = bmp.GetHeight();

	TVector<char> buffer(r.Width * r.Height * 4);

	bmp.LockBits(&r, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
	memcpy(buffer.data(), bitmapData.Scan0, buffer.size());
	bmp.UnlockBits(&bitmapData);

	std::ofstream file(SRC_DIR + "splash_data.h", std::ios::out);
	file << ("#pragma once\n#pragma warning(push)\n#pragma warning(disable: 4838)\n#if !_SERVER && !_COMPILER\n");
	file << ("unsigned char SplashData[] = \n{\n\t");

	for (u32 i = 0; i <	buffer.size(); i++)
	{
		file << KString(buffer[i]).CStr() << ", ";
		if (i % 12 == 0 && i != 0)
			file << "\n\t";
	}

	file << ("\n};\n\n#endif\n#pragma warning(pop)\n");
	file.close();
}
#endif

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	switch (message)
	{
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			
			Gdiplus::Graphics graphics(hdc);
#if _DEV
			KString tex = TEX_DIR + "splash.png";
			Gdiplus::Image gdiImage(tex.ToWideStr().c_str());
#else
			Gdiplus::Bitmap gdiImage
			(
				512,
				512,
				PixelFormat32bppARGB
			);

			Gdiplus::BitmapData bitmapData;
			Gdiplus::Rect r;
			r.X = 0;
			r.Y = 0;
			r.Width = gdiImage.GetWidth();
			r.Height = gdiImage.GetHeight();

			gdiImage.LockBits(&r, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);
			memcpy(bitmapData.Scan0, SplashData, 512 * 512 * 4);

			gdiImage.UnlockBits(&bitmapData);
#endif

			Gdiplus::REAL x(0);
			Gdiplus::REAL y(0);
			Gdiplus::REAL w(512);
			Gdiplus::REAL h(512);

			graphics.DrawImage(&gdiImage, x, y, w, h);

			EndPaint(hWnd, &ps);
			break;
		}
		case WM_ERASEBKGND:
		{
			GetClientRect(hWnd, &rect);
			FillRect((HDC)wParam, &rect, CreateSolidBrush(RGB(0, 0, 0)));
			return 0;
		}
		/*case WM_DESTROY:
		{
			SetFocus(glfwGetWin32Window(GetRenderInterface()->GetGameWindow()->GetGlfwWindow()));
		}*/
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif