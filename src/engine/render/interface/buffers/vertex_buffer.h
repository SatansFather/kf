#pragma once

#if !_SERVER

#include "engine/global/types_numeric.h"

// create base class for pointer usage
class KVertexBuffer
{
protected:

	u32 VertexSize = 0;
	u32 VertexCount = 0;
	bool bUseIndexBuffer = false;
	bool bInstantiated = false;

public:

	// virtual destructor allows ComPtrs to be properly released
	virtual ~KVertexBuffer() = default;

	// return the address of the vertex buffer pointer
	// only needed for D3D
	virtual void* GetVertexBufferAddress() const { return nullptr; }

	// return a pointer to the vertex buffer
	// only needed for D3D
	virtual void* GetVertexBufferPointer() const { return nullptr; }

	// return the GLuint ID for the buffer
	// OpenGL only
	virtual u32 GetVertexBufferID() const { return 0; }

	// return a pointer to the index buffer
	// D3D only
	virtual void* GetIndexBufferPointer() const { return nullptr; };

	// return GLuint ID for the index buffer
	// OpenGL only
	virtual u32 GetIndexBufferID() const { return 0; }

	// index array can use a vector since the only type for indices is u32
	// using template for vertex type is dumb because it can pollute headers with vertex types
	virtual void CreateStatic(const void* vertices, u32 vert_size, u32 vert_count, const TVector<u32>& indices = {}) = 0;
	virtual void CreateDynamic(const void* vertices, u32 vert_size, u32 vert_count, const TVector<u32>& indices = {}) = 0;

	inline u32 GetVertexCount() const { return VertexCount; }
	inline u32 GetVertexSize() const { return VertexSize; }
	inline bool UsesIndexBuffer() const { return bUseIndexBuffer; }
	inline bool IsValid() const { return bInstantiated; }
};

#endif