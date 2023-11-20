#pragma once

#include "vec3.h"
#include "line.h"
#include "line_segment.h"

enum class EPointSide
{
	Front,
	Behind,
	On
};

enum class EPolyClassification : u8
{
	Front,
	Behind,
	Spanning,
	Coplanar
};

template <typename T>
struct KPlane
{
	using Vec3 = KVec3<T>;
	using Plane = KPlane<T>;
	using Line = KLine<T>;

	Vec3 Normal;
	T D;

	KPlane<T>() = default;
	KPlane<T>(const Vec3& normal, T d);
	KPlane<T>(const Vec3& v1, const Vec3& v2, const Vec3& v3);
	KPlane<T>(const Vec3& point, const Vec3& normal);
	KPlane<T>(const Line& line, const Vec3& normal);

	void Set(const Vec3& v1, const Vec3& v2, const Vec3& v3);
	void Set(const Vec3& point, const Vec3& normal);

	bool IntersectLinePlane(const KVec3<T>& planeNormal, T planeD, const KVec3<T>& linePos, const KVec3<T>& lineDir, T& t, T epsilon = Epsilon<T>()) const;
	bool Intersects(const Plane& plane, const Plane& plane2, Vec3* out_point = 0) const;
	bool Intersects(const Plane& plane, Line* out_line = 0) const;
	bool Intersects(const KLineSegment<T>& line, T& dist, T epsilon = Epsilon<T>()) const;
	bool IsParallel(const Plane& plane, T epsilon = Epsilon<T>()) const;
	bool Contains(const Vec3& point, T epsilon = Epsilon<T>()) const;
	bool IsCoplanar(const Plane& other, T epsilon = Epsilon<T>());
	T Distance(const Vec3& point) const;
	T SignedDistance(const Vec3& point) const;
	KPlane<T> Inverted() const;
	bool Equals(const KPlane<T>& other, T norm_epsilon = 0.000001, T dist_epsilon = 0.0001) const;
	bool EqualsInverse(const KPlane<T>& other, T norm_epsilon = 0.000001, T dist_epsilon = 0.0001) const;
	bool IsAxial(T epsilon = T(0.000001), u8* index = nullptr) const;
	EPointSide ClassifyPoint(const Vec3& point, T epsilon = T(.001)) const;
	Vec3 PointOnPlane() const;
	// classify polygon is defined in polygon class to avoid circular include

	bool operator == (const KPlane<T>& other) const
	{
		return D == other.D && Normal == other.Normal;
	}

	bool operator != (const KPlane<T>& other) const
	{
		return D != other.D || Normal != other.Normal;
	}

	// allows this to be mapped
	bool operator < (const KPlane<T> other) const 
	{
		if (D == other.D)
		{
			if (Normal.x == other.Normal.x)
			  if (Normal.y == other.Normal.y)
				if (Normal.z == other.Normal.z)
				  return false;
				else return Normal.z < other.Normal.z;
			  else return Normal.y < other.Normal.y;
			else return Normal.x < other.Normal.x;
		}
		
		return D < other.D;
	}

	template <typename Type>
	KPlane<Type> ToType() const { return KPlane<Type>(Normal.template ToType<Type>(), (Type)D); }

	// allows this to be a map key without me figuring out how to do that properly
	void ToByteArray(std::array<u8, sizeof(T) * 4>& arr) const;
};

typedef KPlane<f32> FPlane;
typedef KPlane<f64> DPlane;
typedef KPlane<GFlt> GPlane;
