#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "engine/math/glm.h"

class KLight
{

protected:

	glm::mat4 LightMatrix;
	u32 DepthRes = 2048;

	UPtr<class KDepthBuffer> DepthBuffer;
	UPtr<class KRenderTarget> RenderTarget;

public:

	KLight(u32 depthres = 2048);
	~KLight();

	void UpdateLightDepthBuffer();

	f32 GetDepthResolution() const { return DepthRes; }
	void SetDepthResolution(f32 res);

	KDepthBuffer* GetDepthBuffer() { return DepthBuffer.get(); }

protected:

	virtual void BuildLightMatrix() = 0;
};

#endif