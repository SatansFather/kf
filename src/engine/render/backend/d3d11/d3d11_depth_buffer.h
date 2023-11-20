#pragma once

#if !_SERVER && _WIN32

#include "engine/render/interface/depth_buffer.h"
#include "d3d11_include.h"

class KDepthBuffer_D3D11 : public KDepthBuffer, public D3D11Object
{
	ComPtr<ID3D11DepthStencilState> DepthWriteState;
	ComPtr<ID3D11DepthStencilState> DepthReadOnlyState;
	ComPtr<ID3D11DepthStencilState> DepthDisabledState;

	// to be bound with render target
	ComPtr<ID3D11DepthStencilView> DepthStencilView;

	// can be bound as a Texture2D to a shader
	ComPtr<ID3D11ShaderResourceView> DepthShaderView;

	ComPtr<ID3D11UnorderedAccessView> DepthUav;

	ComPtr<ID3D11Texture2D> DepthStencilTexture;


	// when going to enabled from disabled, we need to select the correct state
	bool bReadOnlySelected = false;

	bool bInitialized = false;

public:
	
	KDepthBuffer_D3D11(EDepthUsage usage = EDepthUsage::SceneDepth, u32 w = 0, u32 h = 0);

	void SetEnabled(bool enabled) override;
	void SetReadOnly(bool read_only) override;
	void Clear() override;

	ID3D11DepthStencilView* GetViewPointer() { return DepthStencilView.Get(); }
	inline ID3D11ShaderResourceView* GetShaderResource() const { return DepthShaderView.Get(); }

	void BindTexture2D(u8 slot = 0, EShaderStage stage = EShaderStage::Pixel) override;

	void* GetUAV() override;
};

#endif