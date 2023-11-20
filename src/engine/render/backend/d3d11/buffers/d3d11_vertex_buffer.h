#pragma once

#if !_SERVER && _WIN32

#include "../d3d11_include.h"
#include "engine/render/interface/buffers/vertex_buffer.h"
#include "engine/global/types_container.h"
#include "engine/os/windows/windows.h"
#include "engine/utility/k_assert.h"

class KVertexBuffer_D3D11 : public KVertexBuffer, public D3D11Object
{
	friend class KRenderInterface_D3D11;

private:

	ComPtr<ID3D11Buffer> VertexBuffer;
	ComPtr<ID3D11Buffer> IndexBuffer;

	void CreateVertexBuffer(const void* vertices, u32 vertSize, u32 vertCount, bool dynamic)
	{
		VertexSize = vertSize;
		VertexCount = vertCount;

		if (VertexCount == 0 || VertexSize == 0) return;

		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = UINT(this->VertexSize * this->VertexCount);
		bd.StructureByteStride = this->VertexSize;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;

		K_ASSERT_HR(GetDevice()->CreateBuffer(&bd, &sd, &VertexBuffer),
			"could not create vertex buffer");

		bInstantiated = true;
	}

	void CreateIndexBuffer(const TVector<u32>& indices)
	{
		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.ByteWidth = UINT(indices.size() * sizeof(u32));
		ibd.StructureByteStride = sizeof(u32);
		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = indices.data();

		K_ASSERT_HR(CheckHR(GetDevice()->CreateBuffer(&ibd, &isd, &IndexBuffer)),
			"could not create index buffer"); 
		
		bUseIndexBuffer = true;		
	}

public:

	void CreateStatic(const void* vertices, u32 vert_size, u32 vert_count, const TVector<u32>& indices = {}) override
	{
		CreateVertexBuffer(vertices, vert_size, vert_count, false);
		if (indices.size() > 0) CreateIndexBuffer(indices);
	}

	void CreateDynamic(const void* vertices, u32 vert_size, u32 vert_count, const TVector<u32>& indices = {}) override
	{
		CreateVertexBuffer(vertices, vert_size, vert_count, true);
		if (indices.size() > 0) CreateIndexBuffer(indices);
	}

	void* GetVertexBufferPointer() const override
	{
		return VertexBuffer.Get();
	}

	void* GetVertexBufferAddress() const override
	{
		return (void*)VertexBuffer.GetAddressOf();
	}

	void* GetIndexBufferPointer() const override
	{ 
		return IndexBuffer.Get();
	}
};

#endif