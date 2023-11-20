#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "d3d11_include.h"
#include "engine/render/interface/texture2d.h"

class KTexture2D_D3D11 : public KTexture2D, public D3D11Object
{

	ComPtr<ID3D11ShaderResourceView> TextureResource;

	void Finalize(const D3D11_TEXTURE2D_DESC& td, const D3D11_SUBRESOURCE_DATA& sd);

public:

	using KTexture2D::KTexture2D;

	void CreateFromSurface(UPtr<KSurface2D>& surface) override;
	void CreateFrom2dFloatArray(f32* values, u32 w, u32 h) override;
	void CreateFromFreetypeGlyph(struct FT_GlyphSlotRec_* glyph) override;
	inline ID3D11ShaderResourceView* GetResource() const { return TextureResource.Get(); }
};

#endif