#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "engine/math/glm.h"
#include "engine/math/vec3.h"

#define LINE_BUFFER_SIZE 512

struct KDebugLineInfo
{
	glm::vec4 PointA;
	//glm::vec4 PointB;
	glm::vec4 Color;
};

class KDebugLine
{
	static UPtr<class KGpuBuffer> DebugLineBuffer;
	static UPtr<class KVertexBuffer> DebugIndexBuffer;
	static KDebugLineInfo LineData[LINE_BUFFER_SIZE];
	static u32 LineCount;

public:

	static void AddLine(const KDebugLineInfo& info);
	static void AddLine(const GVec3& start, const GVec3& target, f32 r = 1, f32 g = 0, f32 b = 1, f32 thickness = 2);
	static void DrawLines();
};

#endif