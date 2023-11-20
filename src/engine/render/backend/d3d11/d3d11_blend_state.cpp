#if !_SERVER

#include "d3d11_blend_state.h"

void KBlendState_D3D11::Finalize(const KBlendData& data)
{
	D3D11_BLEND_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = (D3D11_BLEND)data.SrcBlend;
	rtbd.DestBlend = (D3D11_BLEND)data.DestBlend;
	rtbd.BlendOp = (D3D11_BLEND_OP)data.BlendOp;
	rtbd.SrcBlendAlpha = (D3D11_BLEND)data.SrcBlendAlpha;
	rtbd.DestBlendAlpha = (D3D11_BLEND)data.DestBlendAlpha;
	rtbd.BlendOpAlpha = (D3D11_BLEND_OP)data.BlendOpAlpha;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	bd.AlphaToCoverageEnable = false;
	bd.RenderTarget[0] = rtbd;

	GetDevice()->CreateBlendState(&bd, &State);
}

void KBlendState_D3D11::Finalize(KBlendData* data, u32 descCount)
{
	D3D11_BLEND_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = true;

	for (u32 i = 0; i < descCount; i++)
	{
		D3D11_RENDER_TARGET_BLEND_DESC rtbd;
		ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));

		rtbd.BlendEnable = true;
		rtbd.SrcBlend = (D3D11_BLEND)data[i].SrcBlend;
		rtbd.DestBlend = (D3D11_BLEND)data[i].DestBlend;
		rtbd.BlendOp = (D3D11_BLEND_OP)data[i].BlendOp;
		rtbd.SrcBlendAlpha = (D3D11_BLEND)data[i].SrcBlendAlpha;
		rtbd.DestBlendAlpha = (D3D11_BLEND)data[i].DestBlendAlpha;
		rtbd.BlendOpAlpha = (D3D11_BLEND_OP)data[i].BlendOpAlpha;
		rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
		
		bd.RenderTarget[i] = rtbd;
	}

	GetDevice()->CreateBlendState(&bd, &State);
}	

#endif
