#pragma once

#include "vec3.h"

template <typename T>
struct KLineSegment
{
	using Vec3 = KVec3<T>;

	Vec3 a, b;

	KLineSegment<T>() {}
	KLineSegment<T>(const Vec3& a, const Vec3& b) : a(a), b(b) {}

	bool Contains(const Vec3& point, T threshold = T(.001)) const;
	Vec3 ClosestPoint(const Vec3& point) const { T d; return ClosestPoint(point, d); }
	Vec3 ClosestPoint(const Vec3& point, T& d) const;
	Vec3 GetPoint(T dist) const;
	T Length() const;
	Vec3 Direction() const;
	Vec3 DirectionUnNormalized() const;
	bool Equals(const KLineSegment<T>& other, T epsilon = Epsilon<T>()) const;
	Vec3 GetPointAtAxis(u8 axis, T value) const;
	KLineSegment<T> AdjustLineZ(T bias);
};

typedef KLineSegment<f32> FLineSegment;
typedef KLineSegment<f64> DLineSegment;
typedef KLineSegment<GFlt> GLineSegment;
