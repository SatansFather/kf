#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "engine/utility/kstring.h"
#include <memory>
#include "color.h"

enum class ESurfacePadMethod : u8
{
	NoPadding,
	Clamp,
	Repeat
};

class KSurface2D
{
	friend class KRenderInterface;
	friend class KMapFile;

private:

	UPtr<FColor8[]> Data;
	u32 Width, Height;
	bool bHasTransparency = false;

	u32 DisplayWidth = 0;
	u32 DisplayHeight = 0;

	ESurfacePadMethod PadMethod = ESurfacePadMethod::NoPadding;

	FColor8* GetPixelPointer(u32 x, u32 y) const;

	KString Name;

public:

	KSurface2D() = default;
	KSurface2D(u32 w, u32 h, ESurfacePadMethod pad = ESurfacePadMethod::NoPadding);
	KSurface2D(u32 w, u32 h, UPtr<FColor8[]> buffer);

	FColor8* GetData() { return Data.get(); }

	void ClearAndResizeData(u32 w, u32 h, ESurfacePadMethod pad = ESurfacePadMethod::NoPadding);

	u32 GetPaddedWidth() const { return Width; }
	u32 GetPaddedHeight() const  { return Height; }
	u32 GetContentWidth() const;
	u32 GetContentHeight() const;
	FColor8 GetPixel(u32 x, u32 y) const;
	FColor8 GetPaddedPixel(u32 x, u32 y) const;
	void SetPixel(u32 x, u32 y, u8 r, u8 g, u8 b, u8 a);
	void SetPixel(u32 x, u32 y, FColor8 color);

	void SetDisplayWidth(u32 w) { DisplayWidth = w; }
	void SetDisplayHeight(u32 h) { DisplayHeight = h; }
	u32 GetDisplayWidth() { return DisplayWidth; }
	u32 GetDisplayHeight() { return DisplayHeight; }

	bool HasTransparency() const { return bHasTransparency; }
	bool IsPadded() const { return PadMethod != ESurfacePadMethod::NoPadding; }

	u8 GetPadMethod() { return (u8)PadMethod; }

	KString GetName() { return Name; }

	// loads a png starting from TEX_DIR directory
	void LoadPNG(const KString& name, ESurfacePadMethod pad = ESurfacePadMethod::NoPadding);

#if !_DEV
	void LoadFromWad(const KString& name);
#endif

	// saves the current data as a png file
	void SavePNG() const;

	//void CreateFromColorArray(u8* colors, u32 w, u32 h, ESurfacePadMethod pad = ESurfacePadMethod::NoPadding);
	//void CreateFromColorArray(UPtr<u8>& colors, u32 w, u32 h, ESurfacePadMethod pad = ESurfacePadMethod::NoPadding);

	UPtr<KSurface2D> CopySurface() const;
};

#endif