#pragma once

#if !_SERVER

enum class EShaderStage : unsigned char
{
	Pipeline = 0,
	Vertex = 1,
	Pixel = 2,
	Compute = 3,
	All = 4
};

#endif