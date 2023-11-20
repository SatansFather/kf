#pragma once

#if !_SERVER

#include "engine/render/interface/blend_state.h"
#include "d3d11_include.h"

class KBlendState_D3D11 : public KBlendState, public D3D11Object
{
	friend class KRenderInterface_D3D11;
	ComPtr<ID3D11BlendState> State;
	void Finalize(const KBlendData& data) override;
	void Finalize(KBlendData* data, u32 descCount) override;
};

#endif