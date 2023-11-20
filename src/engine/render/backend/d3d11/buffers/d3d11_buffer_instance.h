#pragma once

#if !_SERVER && _WIN32

#include "engine/render/interface/buffers/buffer_instance.h"
#include "../d3d11_include.h"


template <typename DataType>
class KInstanceBuffer_D3D11 : public KInstanceBuffer<DataType>, public D3D11Object
{
private:

	ComPtr<ID3D11Buffer> Buffer;

	void CreateD3D11InstanceBuffer(bool dynamic, u32 count)
	{
		K_ASSERT(!Buffer.Get(), "cannot re-create a constant buffer");

		D3D11_BUFFER_DESC cbbd;
		ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
		cbbd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = sizeof(DataType) * count;
		cbbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		cbbd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		cbbd.MiscFlags = 0;
		if (CheckHR(GetDevice()->CreateBuffer(&cbbd, NULL, &ConstantBuffer)))
		{
			InstanceCount = count;
			bIsDynamic = dynamic;
		}
	}

public:

	void CreateStatic(u32 count) override
	{
		K_ASSERT(!Buffer.Get(), "cannot re-create a constant buffer");
		CreateD3D11InstanceBuffer(false, count);
	}

	void CreateDynamic(u32 count) override
	{
		K_ASSERT(!Buffer.Get(), "cannot re-create a constant buffer");
		CreateD3D11InstanceBuffer(true, count);
	}

	void Update(const void* data) override
	{
		if (IsDynamic())
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			GetContext()->Map(Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			memcpy(mapped.pData, data, sizeof(DataType) * InstanceCount);
			GetDeviceContext()->Unmap(Buffer.Get(), 0);
		}
		else
		{
			GetContext()->UpdateSubresource(Buffer.Get(), 0, NULL, data, 0, 0);
		}
	}
};

#endif