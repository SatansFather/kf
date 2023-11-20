#if !_SERVER && _WIN32

#include "d3d11_gpu_buffer.h"

void KGpuBuffer_D3D11::CreateD3D11ConstantBuffer(u32 typeSize, bool dynamic, i32 slot, EShaderStage stage)
{
	K_ASSERT(!Buffer.Get(), "cannot re-create a constant buffer");

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.ByteWidth = typeSize;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = 0;
	K_ASSERT_HR(GetDevice()->CreateBuffer(&desc, NULL, &Buffer), "could not create constant buffer");

	TypeSize = typeSize;
	Slot = slot;
	ShaderStage = stage;
	bIsDynamic = dynamic;
	bConstant = true;
	
}

void KGpuBuffer_D3D11::CreateD3D11StructuredBuffer(u32 typeSize, bool dynamic, i32 slot, EShaderStage stage, u32 numElements)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = typeSize * numElements;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = typeSize;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;;
	K_ASSERT_HR(GetDevice()->CreateBuffer(&desc, NULL, &Buffer), "could not create structured buffer");

	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	ZeroMemory(&srv, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srv.Format = DXGI_FORMAT_UNKNOWN;
	srv.Buffer.FirstElement = 0;
	srv.Buffer.NumElements = numElements;
	srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	K_ASSERT_HR(GetDevice()->CreateShaderResourceView(Buffer.Get(), &srv, &SRV), "could not create SRV");

	TypeSize = typeSize;
	Slot = slot;
	ShaderStage = stage;
	bIsDynamic = dynamic;
	NumElements = numElements;
}

void KGpuBuffer_D3D11::CreateD3D11InstanceBuffer(u32 typeSize, bool dynamic, u32 numElements)
{
	TypeSize = typeSize;
	NumElements = numElements;
	bIsDynamic = dynamic;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.ByteWidth = typeSize * NumElements;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = 0;
	GetDevice()->CreateBuffer(&desc, NULL, &Buffer);
}

void KGpuBuffer_D3D11::CreateConstantStatic(u32 typeSize, i32 slot, EShaderStage stage)
{
	CreateD3D11ConstantBuffer(typeSize, false, slot, stage);
}

void KGpuBuffer_D3D11::CreateConstantDynamic(u32 typeSize, i32 slot, EShaderStage stage)
{
	CreateD3D11ConstantBuffer(typeSize, true, slot, stage);
}

void KGpuBuffer_D3D11::CreateStructuredStatic(u32 typeSize, i32 slot, EShaderStage stage, u32 numElements)
{
	CreateD3D11StructuredBuffer(typeSize, false, slot, stage, numElements);
}

void KGpuBuffer_D3D11::CreateStructuredDynamic(u32 typeSize, i32 slot, EShaderStage stage, u32 numElements)
{
	CreateD3D11StructuredBuffer(typeSize, true, slot, stage, numElements);
}

void KGpuBuffer_D3D11::CreateInstancedStatic(u32 typeSize, u32 instanceCount)
{
	CreateD3D11InstanceBuffer(typeSize, false, instanceCount);
}

void KGpuBuffer_D3D11::CreateInstancedDynamic(u32 typeSize, u32 instanceCount)
{
	CreateD3D11InstanceBuffer(typeSize, true, instanceCount);
}

void KGpuBuffer_D3D11::Update(const void* data, u32 elemCount /*= 0*/)
{
	if (this->IsDynamic())
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		GetContext()->Map(Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		memcpy(mapped.pData, data, TypeSize * (bConstant ? 1 : (elemCount == 0 ? NumElements : elemCount)));
		GetContext()->Unmap(Buffer.Get(), 0);
	}
	else
	{
		GetContext()->UpdateSubresource(Buffer.Get(), 0, NULL, data, 0, 0);
	}
}

#endif