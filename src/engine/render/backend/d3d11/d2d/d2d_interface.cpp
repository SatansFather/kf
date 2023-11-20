#if !_SERVER && _WIN32

#include "wincodec.h"
#include "engine/os/windows/gdi.h"
#include "d2d_interface.h"
#include "engine/utility/k_assert.h"
#include "engine/utility/kstring.h"
#include "engine/render/interface/render_interface.h"
#include "engine/render/surface2d.h"
#include "../d3d11_bitmap.h"
#include "engine/global/paths.h"
#include "../d3d11_interface.h"
#include <filesystem>



//#include "font_loader.h"
#include "../../../fonts/fonts.h"

#include "engine/kwad/kwad_chunk_image_hud.h"
#include "engine/game_instance.h"
#include "engine/kwad/kwad_file.h"
#include "engine/kwad/kwad_reader.h"
#include "engine/kwad/kwad_chunk_font.h"
#include "suffering/ResourceFontContext.h"

#pragma comment(lib, "windowscodecs.lib")

IWICImagingFactory* pIWICFactory;

// microsoft docs copy paste
void __bmpload(
	ID2D1RenderTarget* pRenderTarget,
	IWICImagingFactory* pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	KHudBitmap_D3D11* bitmap,
	Gdiplus::Bitmap& gdibmp
)
{
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICBitmapScaler* pScaler = NULL;


	// Convert the image format to 32bppPBGRA
	// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
	K_ASSERT_HR(pIWICFactory->CreateFormatConverter(&bitmap->Source), "could not create format converter");

	HBITMAP hbm = nullptr;
	Gdiplus::Color bg;
	gdibmp.GetHBITMAP(bg, &hbm);

	IWICBitmap* wbmp = nullptr;
	pIWICFactory->CreateBitmapFromHBITMAP(hbm, NULL, WICBitmapUsePremultipliedAlpha, &wbmp);

	//pIWICFactory->CreateBitmapFromHBITMAP(gdibmp.GetHBITMAP()
	K_ASSERT_HR(bitmap->Source->Initialize(
		//pSource,
		wbmp,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeMedianCut
	), "could not initialize bmp source");
}

KD2DInterface::~KD2DInterface()
{
	if (FontContext) delete FontContext;

	//for (const auto& kv : Layouts)
	//	DeleteTextLayout(kv.first);
}

void KD2DInterface::InitD2D()
{
	IDXGIFactory1* fact;
	K_ASSERT_HR(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&fact),
		"could not initialize DGXI factory");

	D2D1_FACTORY_OPTIONS dbg =
#if _DEBUG
	{ D2D1_DEBUG_LEVEL_INFORMATION };
#else
	{ D2D1_DEBUG_LEVEL_NONE };
#endif


	K_ASSERT_HR(D2D1CreateFactory
	(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		dbg, D2DFactory.GetAddressOf()
	), "could not create D2D1 factory");
	K_ASSERT_HR(DWriteCreateFactory
	(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(WriteFactory.GetAddressOf())
	), "could not create DirectWrite factory");

	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**>(&pIWICFactory));

	//ComPtr<IDXGIDevice1> dxgiDevice;
	//GetD3D11Interface()->GetDevice()->QueryInterface(__uuidof(IDXGIDevice), (&dxgiDevice));
	//D2DFactory->CreateDevice(dxgiDevice.Get(), &Device);
	//Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &DeviceContext);

	CreateFontCollection();

	LOG("D2D Initialized");
}

void KD2DInterface::CreateD2DSurfaceForD3D11(IDXGISwapChain* swapchain)
{
	RenderTarget.Reset();

	IDXGISurface* dxgi_surface;
	swapchain->GetBuffer(0, IID_PPV_ARGS(&dxgi_surface));

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties
	(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	K_ASSERT_HR(D2DFactory->CreateDxgiSurfaceRenderTarget(dxgi_surface, props, &RenderTarget),
		"could not create D2D surface from D3D11 back buffer")

		dxgi_surface->Release();

	RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &ColorBrush);
	RenderTarget->SetTransform(D2D1::IdentityMatrix());

	RenderTarget->CreateBitmapBrush(nullptr, &BitmapBrush);

	PowerupRadialGradients.resize(EPowerupID::NumPowerups + 2);

	CreatePowerupGradients();

	/*LinearGradients.resize(1);

	TVector<D2D1_GRADIENT_STOP> stops;
	stops.resize(2);
	stops[0].color = D2D1::ColorF(0, 0, 0, 0);
	stops[0].position = 0.0f;
	stops[1].color = D2D1::ColorF(0, 0, 0, 1);
	stops[1].position = 1.f;
	CreateLinearGradient(LinearGradients[0], stops, {0 ,.5}, {200, .5});*/
}

void KD2DInterface::Begin()
{
	RenderTarget->BeginDraw();
}

void* KD2DInterface::LoadFont(const class KString& name, f32 size, EFontUsage usage, bool bold /*= false*/, bool italic /*= false*/)
{
#if _PACK
	static THashSet<std::string> loadedFonts;

	TVector<KString> fonts;
	if (!loadedFonts.contains(name.Get()))
	{
		// find all fonts that start with the same name
		for (const auto& entry : std::filesystem::directory_iterator(FONT_DIR))
		{
			KString path(entry.path());
			TVector<KString> splits;
			path.SplitByChar(splits, '/');
			if (splits.size() == 0) return nullptr;
			KString fileName = splits[splits.size() - 1].Get();

			if (fileName.StartsWith(name))
			{
				KGameInstance::Get().MainWadFile.AddFont(fileName);
				LOG("added font: " + fileName);
			}
		}

		loadedFonts.insert(name.Get());
	}
#endif

	IDWriteFontCollection* col = NULL;
#if !_DEV
	col = GameFontCollection.Get();
#endif
	ComPtr<IDWriteTextFormat> font;
	K_ASSERT_HR(WriteFactory->CreateTextFormat(
		name.ToWideStr().c_str(),
		col,
		bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
		italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		size,
		L"en-us",
		&font
	), KString("could not load font: " + name));

	font->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	font->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	IDWriteTextFormat* f = font.Get();
	Fonts[usage] = std::move(font);

	/*for (const KString& font : fonts)
	{
		void* out = nullptr;
		RemoveFontResourceEx(font.ToWideStr().c_str(), 0, out);
	}*/

	return f;
}

void KD2DInterface::LoadBitmapFromFile(const KString& name, class KHudBitmap_D3D11* bitmap)
{
	// use gdiplus to get file size

#if _DEV
	KString path = KString(TEX_DIR + name + ".png");
	Gdiplus::Bitmap bmp(path.ToWideStr().c_str());
#else

	KWadReader<KWadChunk_ImageHud> reader("kfdata", name);
	KWadChunk_ImageHud* chunk = reader.GetChunk();

	Gdiplus::Bitmap bmp
	(
		chunk->Width,
		chunk->Height,
		PixelFormat32bppARGB
	);

	Gdiplus::BitmapData bitmapData;
	Gdiplus::Rect r;
	r.X = 0;
	r.Y = 0;
	r.Width = bmp.GetWidth();
	r.Height = bmp.GetHeight();

	bmp.LockBits(&r, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);
	memcpy(bitmapData.Scan0, chunk->Data.data(), chunk->Data.size());

	bmp.UnlockBits(&bitmapData);

#endif

	Gdiplus::Status s = bmp.GetLastStatus();
	K_ASSERT(s == Gdiplus::Status::Ok, "could not load hud texture: " + name + "\n" + s);

	u32 width = bmp.GetWidth();
	u32 height = bmp.GetHeight();

	__bmpload(RenderTarget.Get(), pIWICFactory, KString(TEX_DIR + name + ".png").ToWideStr().c_str(), width, height, bitmap, bmp);
	ComPtr<IWICBitmapScaler> scaler;
	GetD3D11Interface()->CreateBitmapScaler(scaler);
	scaler->Initialize(bitmap->Source.Get(), width, height, WICBitmapInterpolationModeFant);
	GetD3D11Interface()->CreateBitmapFromWicBitmap(scaler.Get(), bitmap);

#if _PACK
	KGameInstance::Get().MainWadFile.AddImageHUD(bmp, name);
#endif

	//bitmap->UpdateScale(width, height);
}

void KD2DInterface::DrawBitmap(class KHudBitmap_D3D11* bmp, f32 x, f32 y, f32 w, f32 h, f32 alpha)
{
	//ComPtr<ID2D1Effect> eff = nullptr;
	//DeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &eff);
	//eff->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, bmp->WIC.Get());
	//D2D1_RECT_F r = { x, y, x + w, y + h };
	//
	//DeviceContext->DrawImage(eff.Get(), NULL, &r);
	bmp->UpdateScale(w, h);
	RenderTarget->DrawBitmap(bmp->Bitmap.Get(), { x, y, x + w, y + h }, alpha);
}

void KD2DInterface::DrawCircleProgressBar(f32 centerX, f32 centerY, f32 radius, f32 progress)
{
	progress = KSaturate(progress);

	// rotate p1 around p0 by theta degrees

	D2D1_POINT_2F p0;
	p0.x = centerX;
	p0.y = centerY;

	D2D1_POINT_2F p1;
	p1.x = centerX;
	p1.y = centerY - radius;

	D2D1_POINT_2F p2 = D2D1::Matrix3x2F::Rotation(-progress * 359.999, p0).TransformPoint(p1);

	ComPtr<ID2D1PathGeometry> pathGeometry;
	D2DFactory->CreatePathGeometry(&pathGeometry);

	ComPtr<ID2D1GeometrySink> geometrySink;
	pathGeometry->Open(&geometrySink);

	// begin figure at p0
	geometrySink->BeginFigure(p0, D2D1_FIGURE_BEGIN_FILLED);

	// draw a line between p0 and p1
	geometrySink->AddLine(p1);

	// draw an arc segment between p1 and p2
	geometrySink->AddArc(D2D1::ArcSegment(
		p2,
		D2D1::SizeF(radius, radius),
		0.f,
		D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
		progress > .5 ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL
	));

	// end the figure in a closed state (automatically adds a line from p2 back to p0)
	geometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
	geometrySink->Close();

	// fill the interior of the geometry with the given brush
	ID2D1RadialGradientBrush* radial = nullptr;
	CurrentBrush->QueryInterface(&radial);
	if (radial)
	{
		radial->SetCenter({ centerX, centerY });
		RenderTarget->FillGeometry(pathGeometry.Get(), radial);
		radial->Release();
	}
}

void KD2DInterface::LoadFontsFromWad()
{
#if !_COMPILER
	FontContext = new ResourceFontContext;
	K_ASSERT_HR(FontContext->Initialize(), "could not init font context");

	auto& fontData = KGameInstance::Get().MainWadFile.IndexedData[EWadFileChunkType::Font];
	TVector<u32> keys;
	u32 nextKey = 0;

	for (auto& kv : fontData)
	{
		KWadReader<KWadChunk_Font> reader("kfdata", kv.first, false);
		FontChunks.push_back(reader.GetChunk());
		keys.push_back(nextKey);
		nextKey++;
	}
	
	K_ASSERT_HR(FontContext->CreateFontCollection(
		keys.data(), keys.size() * 4,
		&GameFontCollection), "could not create font collection - " + KString(keys.size()) + " keys");	

	// fonts should never be deleted
#endif
}

void* KD2DInterface::GetFont(EFontUsage usage)
{
	return Fonts[usage].Get();
}

f32 KD2DInterface::GetFontSize(u32 handle)
{
	return Layouts[handle].FontSize;
}

f32 KD2DInterface::GetFontSize(EFontUsage font)
{
	return Fonts[font]->GetFontSize();
}

EFontUsage KD2DInterface::GetFontFromHandle(u32 handle)
{
	return Layouts[handle].FontUsage;
}

u32 KD2DInterface::CreateTextLayout(const KString& text, EFontUsage font, f32 maxW /*= 0*/, f32 maxH /*= 0*/, u32 existingHandle /*= 0*/)
{
	f32 scaledX = GetScaledX();
	f32 scaledY = GetScaledY();
	f32 viewX = GetViewportX();
	f32 viewY = GetViewportY();

	maxW *= viewX;
	maxH *= viewY;

	ComPtr<IDWriteTextLayout> layout;
	K_ASSERT_HR(WriteFactory->CreateTextLayout(
		text.ToWideStr().c_str(),
		text.Length(),
		Fonts[font].Get(),
		maxW, maxH,
		&layout
	), "could not create text layout");

	TextLayout* l = nullptr;
	if (existingHandle != 0)
	{
		l = &Layouts[existingHandle];
		l->LayoutIndex = existingHandle; // really shouldnt change but ok
	}
	else
	{
		LayoutIndex++;
		l = &Layouts[LayoutIndex];
		l->LayoutIndex = LayoutIndex;
	}

	l->LastX = viewX;
	l->LastY = viewY;
	l->String = text;
	l->FontSize = Fonts[font]->GetFontSize();
	l->Length = text.Length();
	l->FontUsage = font;
	l->MaxH = maxH;
	l->MaxW = maxW;

	DWRITE_TEXT_METRICS met;
	layout->GetMetrics(&met);
	l->LineCount = met.lineCount;
	l->TotalHeight = met.height * scaledY;
	l->TotalWidth = met.widthIncludingTrailingWhitespace * scaledY;
	layout->SetFontSize(l->FontSize * scaledY, { 0, l->Length });
	layout->SetMaxHeight(maxH);
	layout->SetMaxWidth(maxW);
	layout->SetWordWrapping(maxH == 0 ? DWRITE_WORD_WRAPPING_NO_WRAP : DWRITE_WORD_WRAPPING_WRAP);

	l->Layout = std::move(layout);

	return existingHandle != 0 ? existingHandle : LayoutIndex;
}

void KD2DInterface::DrawTextLayout(u32 handle, KHudPointF origin)
{
	if (!Layouts.contains(handle)) return;

	TextLayout& l = Layouts[handle];
	if (!l.Layout) return;

	UpdateTextLayout(l);

	RenderTarget->DrawTextLayout(
		*((D2D1_POINT_2F*)(&origin)),
		l.Layout.Get(),
		CurrentBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE);
}

void KD2DInterface::SetTextLayoutFontSize(u32 handle, f32 size, i32 startPosition /*= 0*/, i32 length /*= -1*/)
{
	if (!Layouts.contains(handle)) return;

	TextLayout& l = Layouts[handle];
	if (!l.Layout) return;

	DWRITE_TEXT_RANGE met;
	met.startPosition = startPosition;
	met.length = length == -1 ? l.Length : length;
	l.Layout->SetFontSize(size, met);
}

void KD2DInterface::DeleteTextLayout(u32 handle)
{
	Layouts.erase(handle);
}

void KD2DInterface::SetDrawColor(const FColor32& color)
{
	ColorBrush->SetColor(*((D2D1::ColorF*)(&color)));
	CurrentBrush = ColorBrush.Get();
}

void KD2DInterface::SetDrawColor(class FColor8 color)
{
	SetDrawColor(color.To32());
}

void KD2DInterface::DrawLine(KHudPointF a, KHudPointF b, f32 thickness)
{
	RenderTarget->DrawLine(
		*((D2D1_POINT_2F*)(&a)),
		*((D2D1_POINT_2F*)(&b)),
		CurrentBrush,
		thickness);
}

void KD2DInterface::DrawRect(KHudRectF r, f32 thickness)
{
	RenderTarget->DrawRectangle(
		*((D2D1_RECT_F*)(&r)),
		CurrentBrush,
		thickness);
}

void KD2DInterface::FillRect(KHudRectF r)
{
	RenderTarget->FillRectangle(
		*((D2D1_RECT_F*)(&r)),
		CurrentBrush);
}

void KD2DInterface::FillEllipse(f32 x, f32 y, f32 radX, f32 radY)
{
	ID2D1RadialGradientBrush* radial = nullptr;
	CurrentBrush->QueryInterface(&radial);
	if (radial)
	{
		radial->SetCenter({ x,y });
		radial->SetRadiusX(radX);
		radial->SetRadiusY(radY);
		radial->Release();
	}

	D2D1_ELLIPSE e;
	e.radiusX = radX;
	e.radiusY = radY;
	e.point = { x , y };
	RenderTarget->FillEllipse(e, CurrentBrush);
}

void KD2DInterface::End()
{
	RenderTarget->EndDraw();
}

void KD2DInterface::UpdateTextLayout(TextLayout& l)
{
	f32 viewX = GetViewportX();
	f32 viewY = GetViewportY();
	f32 scaledX = GetScaledX();
	f32 scaledY = GetScaledY();

	// update dimensions if our viewport changed
	if (l.LastY != viewY || l.LastX != viewX)
	{
		CreateTextLayout(l.String, l.FontUsage, l.MaxW, l.MaxH, l.LayoutIndex);

		/*l.LastY = viewY;
		l.LastX = viewX;
		l.Layout->SetFontSize(l.FontSize * scaledY, { 0, l.Length });
		if (l.Container)
		{
			// TODO get adjusted size limits from container
		}
		else
		{
			l.Layout->SetMaxHeight(l.MaxH * scaledY);
			l.Layout->SetMaxWidth(l.MaxW * scaledX);

			DWRITE_TEXT_METRICS met;
			l.Layout->GetMetrics(&met);
			l.LineCount = met.lineCount;
			l.TotalHeight = met.height * scaledY;
			l.TotalWidth = met.widthIncludingTrailingWhitespace * scaledY;
		}*/
	}
}

void KD2DInterface::CreatePowerupGradient(const FColor32& color, ComPtr<ID2D1RadialGradientBrush>& brush, ComPtr<ID2D1GradientStopCollection>& stops)
{
	D2D1_GRADIENT_STOP gradientStops[7];
	gradientStops[0].color = D2D1::ColorF(0, 0, 0, 0);
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = D2D1::ColorF(0, 0, 0, 0);
	gradientStops[1].position = 0.65f;
	gradientStops[2].color = D2D1::ColorF(0, 0, 0, 1);
	gradientStops[2].position = 0.7f;
	gradientStops[3].color = D2D1::ColorF(color.r * .7, color.g * .7, color.b * .7, 1);
	gradientStops[3].position = 0.8f;
	gradientStops[4].color = D2D1::ColorF(color.r, color.g, color.b, 1);
	gradientStops[4].position = 0.85f;
	gradientStops[5].color = D2D1::ColorF(color.r * .7, color.g * .7, color.b * .7, 1);
	gradientStops[5].position = 0.9f;
	gradientStops[6].color = D2D1::ColorF(0, 0, 0, 1);
	gradientStops[6].position = 1.f;

	K_ASSERT_HR(RenderTarget->CreateGradientStopCollection(
		gradientStops,
		7,
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&PowerupGradientStops
	), "could not create radial gradient");

	K_ASSERT_HR(RenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(
			D2D1::Point2F(75, 75),
			D2D1::Point2F(0, 0),
			45 * GetScaledY(),
			45 * GetScaledY()),
		stops.Get(),
		&brush
	), "could not create gradient brush");
}

void KD2DInterface::CreatePowerupConsumedGradient()
{
	D2D1_GRADIENT_STOP gradientStops[7];
	gradientStops[0].color = D2D1::ColorF(0, 0, 0, 0);
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = D2D1::ColorF(0, 0, 0, 0);
	gradientStops[1].position = 0.65f;
	gradientStops[2].color = D2D1::ColorF(0, 0, 0, 1);
	gradientStops[2].position = 0.7f;
	gradientStops[3].color = D2D1::ColorF(.2, .2, .2, 1);
	gradientStops[3].position = 0.8f;
	gradientStops[4].color = D2D1::ColorF(.4, .4, .4, 1);
	gradientStops[4].position = 0.85f;
	gradientStops[5].color = D2D1::ColorF(.2, .2, .2, 1);
	gradientStops[5].position = 0.9f;
	gradientStops[6].color = D2D1::ColorF(0, 0, 0, 1);
	gradientStops[6].position = 1.f;

	K_ASSERT_HR(RenderTarget->CreateGradientStopCollection(
		gradientStops,
		7,
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&EmptyPowerupGradientStops
	), "could not create radial gradient");

	K_ASSERT_HR(RenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(
			D2D1::Point2F(75, 75),
			D2D1::Point2F(0, 0),
			45 * GetScaledY(),
			45 * GetScaledY()),
		EmptyPowerupGradientStops.Get(),
		&PowerupRadialGradients[EPowerupID::NumPowerups]
	), "could not create gradient brush");
}

void KD2DInterface::CreatePowerupGradients()
{
	CreatePowerupConsumedGradient();
	CreatePowerupGradient(FColor32(1, .4, 1, 1), PowerupRadialGradients[EPowerupID::Brain], PowerupGradientStops);
	CreatePowerupGradient(FColor32(.2, .3, .4, 1), PowerupRadialGradients[EPowerupID::Invis], PowerupGradientStops);

	D2D1_GRADIENT_STOP gradientStops[3];
	gradientStops[0].color = D2D1::ColorF(0, 0, 0, 0);
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = D2D1::ColorF(0, 0, 0, 0);
	gradientStops[1].position = 0.4f;
	gradientStops[2].color = D2D1::ColorF(0, 0, 0, 1);
	gradientStops[2].position = 1.f;

	ComPtr<ID2D1GradientStopCollection> stops;

	K_ASSERT_HR(RenderTarget->CreateGradientStopCollection(
		gradientStops,
		3,
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&stops
	), "could not create radial gradient");

	K_ASSERT_HR(RenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(
			D2D1::Point2F(75, 75),
			D2D1::Point2F(0, 0),
			45 * GetScaledY(),
			45 * GetScaledY()),
		stops.Get(),
		&PowerupRadialGradients[EPowerupID::NumPowerups + 1]
	), "could not create gradient brush");
}

void KD2DInterface::CreateLinearGradient(ComPtr<ID2D1LinearGradientBrush>& brush, const TVector<D2D1_GRADIENT_STOP>& stops, D2D1_POINT_2F axisStart, D2D1_POINT_2F axisEnd)
{
	ComPtr<ID2D1GradientStopCollection> collection;

	K_ASSERT_HR(RenderTarget->CreateGradientStopCollection(
		stops.data(),
		stops.size(),
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&collection
	), "could not create gradient stops");

	K_ASSERT_HR(RenderTarget->CreateLinearGradientBrush(
		D2D1::LinearGradientBrushProperties(axisStart, axisEnd),
		collection.Get(), &brush
	), "could not create gradient brush");
}

void KD2DInterface::SetLinearGradientBrush(u8 index)
{
	CurrentBrush = LinearGradients[index].Get();
}

void KD2DInterface::BindRadialBrush(ID2D1RadialGradientBrush* brush)
{
	CurrentBrush = brush;
}

void KD2DInterface::BindBitmapBrush(class KHudBitmap* bitmap, f32 xpos, f32 ypos, f32 xscale, f32 yscale, f32 alpha /*= 1*/, bool repeat /*= false*/)
{
	BitmapBrush->SetBitmap(dynamic_cast<KHudBitmap_D3D11*>(bitmap)->Bitmap.Get());
	BitmapBrush->SetOpacity(alpha);
	D2D1::Matrix3x2F mat = D2D1::Matrix3x2F::Translation(xpos, ypos);
	mat = D2D1::Matrix3x2F::Scale(xscale, yscale) * mat;
	BitmapBrush->SetTransform(mat);

	BitmapBrush->SetExtendModeX(repeat ? D2D1_EXTEND_MODE_WRAP : D2D1_EXTEND_MODE_CLAMP);
	BitmapBrush->SetExtendModeY(repeat ? D2D1_EXTEND_MODE_WRAP : D2D1_EXTEND_MODE_CLAMP);

	CurrentBrush = BitmapBrush.Get();
}

void KD2DInterface::CreateBitmapScaler(ComPtr<struct IWICBitmapScaler>& scaler)
{
	pIWICFactory->CreateBitmapScaler(&scaler);
}

void KD2DInterface::CreateBitmapFromWicBitmap(struct IWICBitmapScaler* scaler, class KHudBitmap_D3D11* bmp)
{
	RenderTarget->CreateBitmapFromWicBitmap(
		scaler,
		NULL,
		&bmp->Bitmap
	);
}

void KD2DInterface::PrintAllFonts(IDWriteFontCollection* collection)
{
	if (!collection) WriteFactory->GetSystemFontCollection(&collection);
	u32 famCount = 0;
	famCount = collection->GetFontFamilyCount();
	for (u32 i = 0; i < famCount; i++)
	{
		IDWriteFontFamily* fam = nullptr;
		collection->GetFontFamily(i, &fam);
		IDWriteLocalizedStrings* names = nullptr;
		fam->GetFamilyNames(&names);

		u32 index = 0;
		BOOL exists = false;
		wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

		int defaultLocaleSuccess = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

		if (defaultLocaleSuccess)
			names->FindLocaleName(localeName, &index, &exists);
		if (!exists)
			names->FindLocaleName(L"en-us", &index, &exists);

		if (!exists) index = 0;

		u32 length = 0;
		names->GetStringLength(index, &length);
		wchar_t* name = new (std::nothrow) wchar_t[length + 1];

		names->GetString(index, name, length + 1);
		KString n(name);
		LOG(n);
	}
}

void KD2DInterface::CreateFontCollection()
{
	
}

#endif

