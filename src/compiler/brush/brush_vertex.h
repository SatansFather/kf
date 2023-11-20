#pragma once

#include "engine/math/vec3.h"
#include "engine/math/vec2.h"

class KBrushVertex
{
public:

	KBrushVertex() = default;
	KBrushVertex(DVec3 v) : Point(v) {}

	DVec3 Point;
	DVec3 Normal;
	f32 TexU, TexV;
	f32 LightU, LightV; // coords in packed lightmap
	f32 LocalLightU, LocalLightV; // coords relative to face
	DVec2 ClampU, ClampV;
	f32 Red = 0, Green = 0, Blue = 0;
	
	DVec3 GIScale = 1;

	// all contacted normals with a matching smoothing group will be added here
	// cleared once all vertices have their normals
	TVector<DVec3> PendingSmoothNormals;
	void AddSmoothedNormal(const DVec3& norm);
};