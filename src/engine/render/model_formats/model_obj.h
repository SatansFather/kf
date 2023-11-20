#pragma once

#if !_SERVER

#include "kfglobal.h"

class KModelObj
{
	UPtr<class KVertexBuffer> VertexBuffer;
	u32 VertexCount = 0;

public:

	~KModelObj();
	
	KVertexBuffer* GetVertexBuffer() { return VertexBuffer.get(); }
	u32 GetVertexCount() const { return VertexCount; }

	void LoadFromFile(const KString& name);
	void Draw();
};

#endif