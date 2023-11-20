#pragma once

#if !_SERVER && _WIN32

#include "kfglobal.h"
#include "engine/render/interface/buffers/gpu_buffer.h"
#include "../d3d11_include.h"

class KGpuBuffer_D3D11 : public KGpuBuffer, public D3D11Object
{
	friend class KRenderInterface_D3D11;

private:

	ComPtr<ID3D11Buffer> Buffer;
	ComPtr<ID3D11ShaderResourceView> SRV;
	bool bConstant = false;

	void CreateD3D11ConstantBuffer(u32 typeSize, bool dynamic, i32 slot, EShaderStage stage);
	void CreateD3D11StructuredBuffer(u32 typeSize, bool dynamic, i32 slot, EShaderStage stage, u32 numElements);
	void CreateD3D11InstanceBuffer(u32 typeSize, bool dynamic, u32 numElements);

public:

	void CreateConstantStatic(u32 typeSize, i32 slot, EShaderStage stage) override;
	void CreateConstantDynamic(u32 typeSize, i32 slot, EShaderStage stage) override;
	void CreateStructuredStatic(u32 typeSize, i32 slot, EShaderStage stage, u32 numElements) override;
	void CreateStructuredDynamic(u32 typeSize, i32 slot, EShaderStage stage, u32 numElements) override;
	void CreateInstancedStatic(u32 typeSize, u32 instanceCount) override;
	void CreateInstancedDynamic(u32 typeSize, u32 instanceCount) override;
	void Update(const void* data, u32 elemCount = 0) override;
};

#endif