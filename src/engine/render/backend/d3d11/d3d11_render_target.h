#pragma once

#if !_SERVER && _WIN32

#include "engine/render/interface/render_target.h"
#include "engine/render/backend/d3d11/d3d11_include.h"

class KRenderTarget_D3D11 : public KRenderTarget, public D3D11Object
{
	friend class KRenderInterface_D3D11;

	ComPtr<ID3D11RenderTargetView> RenderTarget;
	ComPtr<ID3D11Texture2D> RenderTexture;
	ComPtr<ID3D11ShaderResourceView> RenderShaderResource;
	ComPtr<ID3D11UnorderedAccessView> UnorderedAccessView;

	bool bCreated = false;

public:

	void Create(u32 w, u32 h, FRenderTargetCreationFlags flags) override;

	// clears this render target to a color
	void Clear(f32 r = .1, f32 g = .1, f32 b = .1, f32 a = 1) override;

	UPtr<class KMappedRender> GetMapped() override;

	// resource getters
	inline ID3D11RenderTargetView* GetRenderTargetView() { return RenderTarget.Get(); }
	inline ID3D11Texture2D* GetRenderTexture() { return RenderTexture.Get(); }
	inline ID3D11ShaderResourceView* GetRenderShaderResourceView() { return RenderShaderResource.Get(); }
	inline ID3D11UnorderedAccessView* GetUnorderedAccessView() { return UnorderedAccessView.Get(); }

	inline ID3D11RenderTargetView** GetRenderTargetViewAddress() { return RenderTarget.GetAddressOf(); }

	void BindTexture2D(u8 slot, EShaderStage stage = EShaderStage::Pixel) override;
	void* GetUAV() override;
};

class KMappedRender_D3D11 : public KMappedRender, public D3D11Object
{
public:
	
	using KMappedRender::KMappedRender;

	/* virtual interface */
	FColor8 GetAt(u32 x, u32 y) const override;

};

#endif