#pragma once

#if !_SERVER

#include "d3d11_include.h"
#include "d2d/d2d_interface.h"
#include "engine/render/interface/bitmap.h"
#include <wincodec.h>

class KHudBitmap_D3D11 : public KHudBitmap
{
	friend class KD2DInterface;

public:

	ComPtr<ID2D1Bitmap> Bitmap;
	ComPtr<IWICFormatConverter> Source;

	f32 GetWidth() override;
	f32 GetHeight() override;

	void UpdateScale(f32 w, f32 h);

};

#endif