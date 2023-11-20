#if _WIN32 && !_SERVER

#include "gdi.h"
#include "engine/utility/k_assert.h"
#include "engine/utility/kstring.h"
#include "engine/global/paths.h"

void InitGDI()
{
	if (GdiInit) return;
	Gdiplus::GdiplusStartupInput input;
	input.GdiplusVersion = 1;
	input.DebugEventCallback = nullptr;
	input.SuppressBackgroundThread = false;
	Gdiplus::GdiplusStartup(&GDIPlusToken, &input, nullptr);
	GdiInit = true;
}

KGdiBitmap CreateGdiBitmap(const KString& name)
{
	std::wstring wide = KString(TEX_DIR + name + ".png").ToWideStr();
	KGdiBitmap bmp(wide.c_str());
	Gdiplus::Status s = bmp->GetLastStatus();
	K_ASSERT(s == Gdiplus::Status::Ok, "could not load texture \'" + name + "\' with gdiplus - error " + KString(s));
	return std::move(bmp);
}
#endif
