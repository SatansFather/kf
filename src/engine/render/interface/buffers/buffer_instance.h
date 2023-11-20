#pragma once

#if !_SERVER

#include "engine/global/types_numeric.h"

class KInstanceBufferBase
{
protected:

	// true if the buffer was created as dynamic
	bool bIsDynamic = false;

	// the number of instances this object holds
	u32 InstanceCount = 0;

public:

	// virtual destructor allows ComPtrs to be properly released
	virtual ~KInstanceBufferBase() {}

	/* virtual interface */

	// creates a static buffer
	virtual void CreateStatic(u32 count) = 0;

	// creates a dynamic buffer
	virtual void CreateDynamic(u32 count) = 0;

	// updates the buffer according to it being static or dynamic
	virtual void Update(const void* data) = 0;

public:

	inline bool IsValid() const { return InstanceCount > 0; }
	inline bool IsDynamic() const { return bIsDynamic }
	inline bool GetCount() const { return InstanceCount; }

}

// inherit from this one
template <typename DataType>
class KInstanceBuffer : public KInstanceBufferBase
{
protected:

	DataType Data;

};


#endif