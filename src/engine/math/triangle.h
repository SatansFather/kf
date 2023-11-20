#pragma once

#include "vec3.h"
#include "vec2.h"

template <typename T>
struct KTriangle
{
	using Vec3 = KVec3<T>;
	using Triangle = KTriangle<T>;

	Vec3 a;
	Vec3 b;
	Vec3 c;
	KTriangle<T>() = default;
	KTriangle<T>(const Vec3& a, const Vec3& b, const Vec3& c) : a(a), b(b), c(c) {}
	Vec3 ClosestPoint(const Vec3& point) const;
	bool Contains(const KVec3<T>& point, const KVec3<T>& normal = 0) const;
	Vec3 BarycentricUVW(const Vec3& point) const;
	KVec2<T> BarycentricUV(const Vec3& point) const;
};

typedef KTriangle<f32> FTriangle;
typedef KTriangle<f64> DTriangle;
typedef KTriangle<GFlt> GTriangle;