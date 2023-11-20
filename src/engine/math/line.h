#pragma once

#include "vec3.h"

template <typename T>
struct KLine
{
	using Vec3 = KVec3<T>;
	using Line = KLine<T>;

	Vec3 Position;
	Vec3 Direction;
	KLine<T>() = default;
	KLine<T>(const Vec3& pos, const Vec3& dir) : Position(pos), Direction(dir) {}

	Vec3 GetPoint(T distance) const;
};

typedef KLine<f32> FLine;
typedef KLine<f64> DLine;
typedef KLine<GFlt> GLine;