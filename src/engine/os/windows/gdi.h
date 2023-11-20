#pragma once

#if !_SERVER && _WIN32

// GDI is windows only, but is faster than stb
#include "engine/os/windows/windows.h"
#include "engine/global/types_ptr.h"
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

struct KGdiBitmap
{
	UPtr<Gdiplus::Bitmap> Bitmap;

	KGdiBitmap(const wchar_t* path)
	{
		Bitmap = std::make_unique<Gdiplus::Bitmap>(path);
	}

	Gdiplus::Bitmap* operator->() 
	{
		return Bitmap.get();
	}
};

static bool GdiInit = false;
static ULONG_PTR GDIPlusToken;

void InitGDI();

// returns a Gdiplus::Bitmap from a png in the TEX_DIR folder
KGdiBitmap CreateGdiBitmap(const class KString& filename);

#endif
