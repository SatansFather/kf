#ifndef SHADER_SHARED
#define SHADER_SHARED

#if !_SERVER

#ifdef __cplusplus
#define FLOAT4	glm::vec4
#define UINT4	glm::uvec4
#define gUINT	u32
#define MAT4	glm::mat4
#else
#define FLOAT4	float4
#define UINT4	uint4
#define gUINT	uint
#define MAT4	float4x4
#endif

#endif
#endif