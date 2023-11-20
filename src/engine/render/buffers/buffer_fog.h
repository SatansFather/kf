#if !_SERVER
#ifndef FOG_BUFFER
#define FOG_BUFFER

#ifdef __cplusplus
#include "../shader_types.h"
#else
#define FLOAT4	float4
#define UINT4	uint4
#define gUINT	uint
#define MAT4	float4x4
#endif

struct KFogBuffer
{
	FLOAT4 Color;
	FLOAT4 Data;
};

#ifdef __cplusplus
#include "../../global/types_numeric.h"
struct KFogArray
{
	u32 FogCount = 1;
	u32 pad[3];
	KFogBuffer Fog[32];
};
#endif

#endif
#endif
