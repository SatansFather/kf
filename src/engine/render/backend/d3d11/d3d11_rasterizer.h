#pragma once

#include "engine/render/backend/d3d11/d3d11_include.h"
#include "engine/render/interface/rasterizer.h"
#include "engine/global/types_container.h"
#include "engine/global/types_numeric.h"


#if !_SERVER && _WIN32

class KRasterizerState_D3D11 : public KRasterizerState, public D3D11Object
{
private:
	
	ComPtr<ID3D11RasterizerState> CullFront_Fill;
	ComPtr<ID3D11RasterizerState> CullFront_Wire;

	ComPtr<ID3D11RasterizerState> CullNone_Fill;
	ComPtr<ID3D11RasterizerState> CullNone_Wire;

	ComPtr<ID3D11RasterizerState> CullBack_Fill;
	ComPtr<ID3D11RasterizerState> CullBack_Wire;


	// allows SetNewState to find the correct state based on selected modes
	// 4 bits for cull mode, 4 bits for fill mode
	TMap<u8, ID3D11RasterizerState*> RasterMap;

	void SetNewState();
	void AddRasterMapEntry(ID3D11RasterizerState* state, ECullMode cull, EFillMode fill);

public:

	KRasterizerState_D3D11();
	virtual void SetFillMode(EFillMode mode) override;
	virtual void SetCullMode(ECullMode mode) override;

};

#endif