#pragma once

#if !_SERVER

#include "engine/global/types_numeric.h"
#include "../input_layout.h"

class KShader
{
public:
	virtual ~KShader() = default;

	// GL only
	virtual void SetShaderID(u32 id) {}

};

class KShaderVertex : public KShader
{
	virtual void SetInputLayout(const FInputLayout& layout) = 0;
};

class KShaderPixel : public KShader
{

};

class KShaderCompute : public KShader
{
	
};

#endif