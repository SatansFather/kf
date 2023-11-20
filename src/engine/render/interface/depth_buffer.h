#pragma once

#if !_SERVER

#include "engine/global/types_numeric.h"
#include "../shader_stage.h"
#include "uav.h"

enum class EDepthUsage : u8
{
	SceneDepth,
	ShadowDepth
};

class KDepthBuffer : public KUnorderedAccessView
{
protected:

	bool bIsReadOnly = false;
	bool bIsEnabled = true;
	u32 Width = 0;
	u32 Height = 0;
	EDepthUsage Usage;

public:
	
	virtual ~KDepthBuffer() = default;

	virtual void SetEnabled(bool enabled) = 0;
	virtual void SetReadOnly(bool read_only) = 0;
	virtual void BindTexture2D(u8 slot = 0, EShaderStage stage = EShaderStage::Pixel) = 0;
	virtual void Clear() = 0;

	inline bool IsReadOnly() { return bIsReadOnly; }
	inline bool IsEnabled() { return bIsEnabled; }
	inline EDepthUsage GetUsage() { return Usage; }
};

#endif