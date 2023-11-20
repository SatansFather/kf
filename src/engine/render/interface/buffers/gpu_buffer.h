#pragma once

#if !_SERVER

#include "engine/global/types_numeric.h"
#include "engine/render/shader_stage.h"

class KGpuBuffer
{
protected:

	EShaderStage ShaderStage = EShaderStage::Pipeline;

	// true if the buffer was created as dynamic
	bool bIsDynamic = false;

	// buffer is only valid if this is >= 0
	// should only be set once the buffer object is created
	i32 Slot = -1;

	// the size (in bytes) of the type that will be represented by this buffer
	u32 TypeSize = 0;
	
	// how many elements are in a structured buffer
	u32 NumElements = 1;

public:

	// virtual destructor allows ComPtrs to be properly released
	virtual ~KGpuBuffer() = default;

	/* virtual interface */

	// creates a static buffer
	virtual void CreateConstantStatic(u32 typeSize, i32 slot, EShaderStage stage) = 0;
	virtual void CreateStructuredStatic(u32 typeSize, i32 slot, EShaderStage stage, u32 numElements) = 0;
	virtual void CreateInstancedStatic(u32 typeSize, u32 instanceCount) = 0;

	// creates a dynamic buffer
	virtual void CreateConstantDynamic(u32 typeSize, i32 slot, EShaderStage stage) = 0;
	virtual void CreateStructuredDynamic(u32 typeSize, i32 slot, EShaderStage stage, u32 numElements) = 0;
	virtual void CreateInstancedDynamic(u32 typeSize, u32 instanceCount) = 0;

	// updates the buffer according to it being static or dynamic
	virtual void Update(const void* data, u32 elemCount = 0) = 0;

	// important that we DON'T have a bind function here
	// all bind calls must be made from the interface

public:

	inline bool IsValid() const   { return Slot > -1 && ShaderStage != EShaderStage::Pipeline; }
	inline bool IsDynamic() const { return bIsDynamic; }
	inline bool GetSlot() const   { return Slot; }
	inline EShaderStage GetShaderStage() const { return ShaderStage; }
	u32 GetTypeSize() const { return TypeSize; }
};

#endif