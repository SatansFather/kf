#pragma once

#include "kfglobal.h"
#include "engine/math/vec3.h"

#pragma pack(push, 1)
struct KNetVec3
{
	i16 x = 0, y = 0, z = 0;
	u16 Data = 0;

	KNetVec3() = default;
	KNetVec3(const GVec3& v, bool tele = false);
	GVec3 ToVec3() const;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct KNetVec3_Normal
{
	i8 x, y, z;

	KNetVec3_Normal() = default;
	KNetVec3_Normal(const GVec3& v);
	GVec3 ToVec3() const;
};
#pragma pack(pop)