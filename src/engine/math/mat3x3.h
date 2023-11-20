#pragma once

#include "vec3.h"

template <typename T>
struct KMat3x3
{
	using Mat3 = KMat3x3<T>;
	using Vec3 = KVec3<T>;

	T ScaleX, ShearXy, ShearXz;
	T ShearYx, ScaleY, ShearYz;
	T ShearZx, ShearZy, ScaleZ;
	T v[3][3];

	KMat3x3<T>() = default;

	bool SolveAxb(Vec3 b, Vec3& x) const;

	void SetRow(i32 row, const T* data);

	T& At(i32 row, i32 col);
	const T At(i32 row, i32 col) const;
};


typedef KMat3x3<f32> FMat3x3;
typedef KMat3x3<f64> DMat3x3;
typedef KMat3x3<GFlt> GMat3x3;

typedef KMat3x3<f32> FMat3;
typedef KMat3x3<f64> DMat3;
typedef KMat3x3<GFlt> GMat3;