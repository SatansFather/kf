#pragma once

#if !_SERVER

#include "engine/global/types_numeric.h"

struct TestVertex
{
	f32 x, y, z, u, v, nx, ny, nz, lu, lv, minx, maxx, miny, maxy;
};

struct KWorldVertex
{
	f32 PosX, PosY, PosZ;
	f32 TexU, TexV;
	f32 NormX, NormY, NormZ;
	f32 LightU, LightV;
	f32 MinTexU, MaxTexU;
	f32 MinTexV, MaxTexV;
	f32 Red, Green, Blue;
	u32 BufferIndex;
};

struct KPortalVertex
{
	f32 PosX, PosY, PosZ;
	f32 TexU, TexV;
	f32 NormX, NormY, NormZ;
	f32 TimeScale;
	f32 RippeRate;
	f32 HeightRatio;
};

struct KMeshVertex
{
	f32 PosX, PosY, PosZ;
	f32 TexU, TexV;
	f32 NormX, NormY, NormZ;
};

struct KLeakVertex
{
	f32 x, y, z;
};

#endif