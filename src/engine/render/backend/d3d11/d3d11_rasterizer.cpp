#if !_SERVER && _WIN32

#include "d3d11_rasterizer.h"
#include "d3d11_interface.h"

// DELETE
#include "..\..\..\system\terminal\terminal.h"

void KRasterizerState_D3D11::SetNewState()
{
	u8 c = (u8)CullMode;
	u8 f = (u8)FillMode;
	GetContext()->RSSetState(RasterMap[c | (f << 4)]);
}

void KRasterizerState_D3D11::AddRasterMapEntry(ID3D11RasterizerState* state, ECullMode cull, EFillMode fill)
{
	u8 c = (u8)cull;
	u8 f = (u8)fill;
	RasterMap[c | (f << 4)] = state;
}

KRasterizerState_D3D11::KRasterizerState_D3D11()
{
	D3D11_RASTERIZER_DESC rast;
	
	ZeroMemory(&rast, sizeof(D3D11_RASTERIZER_DESC));
	//rast.DepthClipEnable = TRUE;
	//rast.DepthBias = 100000;
	rast.DepthBiasClamp = 0;
	rast.SlopeScaledDepthBias = 1;

	rast.FillMode = D3D11_FILL_SOLID;
	rast.CullMode = D3D11_CULL_NONE;
	GetDevice()->CreateRasterizerState(&rast, &CullNone_Fill);
	AddRasterMapEntry(CullNone_Fill.Get(), ECullMode::CullNone, EFillMode::Solid);

	rast.FillMode = D3D11_FILL_SOLID;
	rast.CullMode = D3D11_CULL_FRONT;
	GetDevice()->CreateRasterizerState(&rast, &CullFront_Fill);
	AddRasterMapEntry(CullFront_Fill.Get(), ECullMode::CullFront, EFillMode::Solid);

	rast.FillMode = D3D11_FILL_SOLID;
	rast.CullMode = D3D11_CULL_BACK;
	GetDevice()->CreateRasterizerState(&rast, &CullBack_Fill);
	AddRasterMapEntry(CullBack_Fill.Get(), ECullMode::CullBack, EFillMode::Solid);

	rast.FillMode = D3D11_FILL_WIREFRAME;
	rast.CullMode = D3D11_CULL_NONE;
	GetDevice()->CreateRasterizerState(&rast, &CullNone_Wire);
	AddRasterMapEntry(CullNone_Wire.Get(), ECullMode::CullNone, EFillMode::Wireframe);

	rast.FillMode = D3D11_FILL_WIREFRAME;
	rast.CullMode = D3D11_CULL_FRONT;
	GetDevice()->CreateRasterizerState(&rast, &CullFront_Wire);
	AddRasterMapEntry(CullFront_Wire.Get(), ECullMode::CullFront, EFillMode::Wireframe);

	rast.FillMode = D3D11_FILL_WIREFRAME;
	rast.CullMode = D3D11_CULL_BACK;
	GetDevice()->CreateRasterizerState(&rast, &CullBack_Wire);
	AddRasterMapEntry(CullBack_Wire.Get(), ECullMode::CullBack, EFillMode::Wireframe);

	SetNewState();
}


void KRasterizerState_D3D11::SetFillMode(EFillMode mode)
{
	if (FillMode != mode)
	{
		FillMode = mode;
		SetNewState();
	}
}

void KRasterizerState_D3D11::SetCullMode(ECullMode mode)
{
	if (CullMode != mode)
	{
		CullMode = mode;
		SetNewState();
	}
}

#endif