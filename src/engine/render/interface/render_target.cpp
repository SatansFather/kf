
#if !_SERVER

#include "render_target.h"
#include "engine/render/surface2d.h"


// DELETE
#include "../../system/time.h"
#include "engine/system/terminal/terminal.h"
#include <thread>

KMappedRender::~KMappedRender() {}

void ScreenshotThread(UPtr<KMappedRender> t)
{
	auto s = t->ToSurface();
	s->SavePNG();
}

void KRenderTarget::Screenshot()
{
	auto t = GetMapped();
	std::thread th(&ScreenshotThread, std::move(t));
	th.detach();
}	

void KRenderTarget::ClearToPreferred()
{
	Clear(PreferredClearColor.r, PreferredClearColor.g, PreferredClearColor.b, PreferredClearColor.a);
}

UPtr<KSurface2D> KMappedRender::ToSurface()
{
	auto surface = std::make_unique<KSurface2D>();

	// the surface will take ownership of the data, so we should copy it
	surface->ClearAndResizeData(Width, Height);
	for (u32 x = 0; x < Width; x++)
	  for (u32 y = 0; y < Height; y++)
		surface->SetPixel(x, y, GetAt(x, y));
	  
	return surface;
}

#endif
