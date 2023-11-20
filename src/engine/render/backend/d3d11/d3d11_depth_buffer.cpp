#if !_SERVER && _WIN32

#include "d3d11_depth_buffer.h"
#include "engine/utility/k_assert.h"
#include "engine/render/draw_config.h"
#include "engine/render/interface/render_interface.h"

KDepthBuffer_D3D11::KDepthBuffer_D3D11(EDepthUsage usage, u32 w, u32 h)
{
	K_ASSERT(!bInitialized, "cannot re-initialize depth buffer");
	bInitialized = true;
	
	bool shader = usage == EDepthUsage::ShadowDepth;
	Usage = usage;

	u32 resX = w;
	u32 resY = h;

	// write state
	{
		D3D11_DEPTH_STENCIL_DESC dsdesc = {};
		dsdesc.DepthEnable = TRUE;
		dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsdesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		K_ASSERT_HR(GetDevice()->CreateDepthStencilState(&dsdesc, DepthWriteState.GetAddressOf()),
			"could not create depth stencil write state");
	}

	// read only state
	{
		D3D11_DEPTH_STENCIL_DESC dsdesc = {};
		dsdesc.DepthEnable = TRUE;
		dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsdesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		K_ASSERT_HR(GetDevice()->CreateDepthStencilState(&dsdesc, DepthReadOnlyState.GetAddressOf()),
			"could not create depth stencil read-only state");
	}

	// disabled state
	{
		D3D11_DEPTH_STENCIL_DESC dsdesc = {};
		dsdesc.DepthEnable = FALSE;
		dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsdesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		K_ASSERT_HR(GetDevice()->CreateDepthStencilState(&dsdesc, DepthDisabledState.GetAddressOf()),
			"could not create depth stencil write state");
	}

	// create depth stencil texture
	D3D11_TEXTURE2D_DESC descdepth = {};
	descdepth.Width = resX;//UserConfig->RenderSettings.ResX * UserConfig->RenderSettings.ResScale;
	descdepth.Height = resY;//UserConfig->RenderSettings.ResY * UserConfig->RenderSettings.ResScale;
	descdepth.MipLevels = 1;
	descdepth.ArraySize = 1;
	descdepth.Format = DXGI_FORMAT_R32_TYPELESS;//shader ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_D32_FLOAT;
	descdepth.SampleDesc.Count = 1;
	descdepth.SampleDesc.Quality = 0;
	descdepth.Usage = D3D11_USAGE_DEFAULT;
	descdepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	K_ASSERT_HR(GetDevice()->CreateTexture2D(&descdepth, nullptr, &DepthStencilTexture),
		"could not create depth stencil texture");

	// create depth stencil view from texture to use as a depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc = {};
	dsvdesc.Flags = 0;
	dsvdesc.Format = DXGI_FORMAT_D32_FLOAT;//shader ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
	dsvdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvdesc.Texture2D.MipSlice = 0;
	K_ASSERT_HR(GetDevice()->CreateDepthStencilView(DepthStencilTexture.Get(), &dsvdesc, &DepthStencilView),
		"could not create depth stencil view");

	//if (shader)
	{
		// create shader resource view so we can view this in a shader
		D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
		srv.Format = DXGI_FORMAT_R32_FLOAT;//DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv.Texture2D.MostDetailedMip = 0;
		srv.Texture2D.MipLevels = 1;
		K_ASSERT_HR(GetDevice()->CreateShaderResourceView(DepthStencilTexture.Get(), &srv, &DepthShaderView),
			"could not create shader resource view from depth texture");
	}
}

void KDepthBuffer_D3D11::SetEnabled(bool enabled)
{
	if (enabled)
	{
		GetContext()->OMSetDepthStencilState(
			bReadOnlySelected ? DepthReadOnlyState.Get() : DepthWriteState.Get(), 1);
	}
	else
	{
		GetContext()->OMSetDepthStencilState(DepthDisabledState.Get(), 1);
	}
}

void KDepthBuffer_D3D11::SetReadOnly(bool read_only)
{
	bReadOnlySelected = read_only;
	GetContext()->OMSetDepthStencilState(
		read_only ? DepthReadOnlyState.Get() : DepthWriteState.Get(), 1);
}

void KDepthBuffer_D3D11::Clear()
{
	GetContext()->ClearDepthStencilView(GetViewPointer(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void KDepthBuffer_D3D11::BindTexture2D(u8 slot, EShaderStage stage)
{
	ID3D11DeviceContext* context = GetContext();

	switch (stage)
	{
		case EShaderStage::Pipeline:
			context->VSSetShaderResources(slot, 1, DepthShaderView.GetAddressOf());
			context->PSSetShaderResources(slot, 1, DepthShaderView.GetAddressOf());
			break;
		case EShaderStage::All:
			context->VSSetShaderResources(slot, 1, DepthShaderView.GetAddressOf());
			context->PSSetShaderResources(slot, 1, DepthShaderView.GetAddressOf());
			context->CSSetShaderResources(slot, 1, DepthShaderView.GetAddressOf());
			break;
		case EShaderStage::Vertex:
			context->VSSetShaderResources(slot, 1, DepthShaderView.GetAddressOf());
			break;
		case EShaderStage::Pixel:
			context->PSSetShaderResources(slot, 1, DepthShaderView.GetAddressOf());
			break;
		case EShaderStage::Compute:
			context->CSSetShaderResources(slot, 1, DepthShaderView.GetAddressOf());
			break;
	}
}

void* KDepthBuffer_D3D11::GetUAV()
{
	return DepthUav.Get();
}

#endif