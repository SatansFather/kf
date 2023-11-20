#pragma once

#if !_SERVER && _WIN32

#include "../d3d11_include.h"
#include "engine/render/2d_prim.h"
#include <d2d1_1.h>
#include <dwrite.h>
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d2d1")
#include "engine/render/font_usage.h"
#include "../../../color.h"
#include "../../../../../game/entity/powerup_index.h"

class KD2DInterface
{
	friend class KRenderInterface_D3D11;
public:

	ComPtr<ID2D1Factory1> D2DFactory;
	ComPtr<ID2D1Device> Device;
	ComPtr<ID2D1DeviceContext> DeviceContext;
	ComPtr<IDWriteFactory> WriteFactory;
	ComPtr<ID2D1RenderTarget> RenderTarget;
	ComPtr<ID2D1SolidColorBrush> ColorBrush;
	ComPtr<ID2D1BitmapBrush> BitmapBrush;

	ComPtr<ID2D1GradientStopCollection> EmptyPowerupGradientStops; // consumed time
	ComPtr<ID2D1GradientStopCollection> PowerupGradientStops;

	ComPtr<ID2D1RadialGradientBrush> PowerupImageCoverBrush;

	ID2D1Brush* CurrentBrush = nullptr;

	ComPtr<IDWriteFontCollection> MemoryFontCollection;

	TVector<ComPtr<ID2D1RadialGradientBrush>> PowerupRadialGradients;
	TVector<ComPtr<ID2D1LinearGradientBrush>> PowerupLinearGradients;
	TVector<ComPtr<ID2D1LinearGradientBrush>> LinearGradients;
private:

	THashMap<EFontUsage, ComPtr<IDWriteTextFormat>> Fonts;

	struct TextLayout 
	{ 
		f32 FontSize = 0; 
		f32 MaxW = 0; 
		f32 MaxH = 0;
		u32 Length = 0;
		f32 LastX = 0; 
		f32 LastY = 0; 
		f32 TotalHeight = 0;
		f32 TotalWidth = 0;
		u32 LineCount = 1;
		u32 LayoutIndex = 0;
		KString String;
		EFontUsage FontUsage;
		void* Container = nullptr;
		ComPtr<IDWriteTextLayout> Layout; 
	};
	THashMap<u32, TextLayout> Layouts;
	u32 LayoutIndex = 0;

	class ResourceFontContext* FontContext = nullptr;
	ComPtr<IDWriteFontCollection> GameFontCollection;
public:
	TVector<class KWadChunk_Font*> FontChunks;
private:

	void InitD2D();
	void CreateD2DSurfaceForD3D11(IDXGISwapChain* swapchain);
	
	// interface functions
	void Begin();
	void SetDrawColor(const FColor32& color);
	void SetDrawColor(FColor8 color);
	void DrawLine(KHudPointF a, KHudPointF b, f32 thickness = 1.f);
	void DrawRect(KHudRectF r, f32 thickness = 1.f);
	void FillRect(KHudRectF r);
	void FillEllipse(f32 x, f32 y, f32 radX, f32 radY);
	void* LoadFont(const class KString& name, f32 size, EFontUsage usage, bool bold = false, bool italic = false);
	void* GetFont(EFontUsage usage);
	f32 GetFontSize(u32 handle);
	f32 GetFontSize(EFontUsage font);
	EFontUsage GetFontFromHandle(u32 handle);
	u32 CreateTextLayout(const KString& text, EFontUsage font, f32 maxW = 0, f32 maxH = 0, u32 existingHandle = 0);
	void DrawTextLayout(u32 handle, KHudPointF origin);
	void SetTextLayoutFontSize(u32 handle, f32 size, i32 startPosition = 0, i32 length = -1);
	void DeleteTextLayout(u32 handle);
	void LoadBitmapFromFile(const KString& name, class KHudBitmap_D3D11* bitmap);
	void DrawBitmap(class KHudBitmap_D3D11* bmp, f32 x, f32 y, f32 w = 0, f32 h = 0, f32 alpha = 1);
	void DrawCircleProgressBar(f32 centerX, f32 centerY, f32 radius, f32 progress);
	void LoadFontsFromWad();
	void End();


	void UpdateTextLayout(TextLayout& l);

	void CreatePowerupGradient(const FColor32& color, ComPtr<ID2D1RadialGradientBrush>& brush, ComPtr<ID2D1GradientStopCollection>& stops);
	void CreatePowerupConsumedGradient();
	void CreatePowerupGradients();

	void CreateLinearGradient(ComPtr<ID2D1LinearGradientBrush>& brush, const TVector<D2D1_GRADIENT_STOP>& stops, D2D1_POINT_2F axisStart, D2D1_POINT_2F axisEnd);
	void SetLinearGradientBrush(u8 index);

	void BindRadialBrush(ID2D1RadialGradientBrush* brush);
	void BindBitmapBrush(class KHudBitmap* bitmap, f32 xpos, f32 ypos, f32 xscale, f32 yscale, f32 alpha = 1, bool repeat = false);
	void CreateBitmapScaler(ComPtr<struct IWICBitmapScaler>& scaler);
	void CreateBitmapFromWicBitmap(struct IWICBitmapScaler* scaler, class KHudBitmap_D3D11* bmp);

	void CreateFontCollection();

public:
	void PrintAllFonts(IDWriteFontCollection* collection = nullptr);

	~KD2DInterface();
};

#endif