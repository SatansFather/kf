#pragma once

#include "kfglobal.h"
#include "plane.h"
#include "line_segment.h"

template <typename T>
struct KBoundingBox
{
	using Vec3 = KVec3<T>;
	using Plane = KPlane<T>;

	Vec3 Min, Max;
	bool bSet = false;

	KBoundingBox() = default;
	KBoundingBox(Vec3 min, Vec3 max) : Min(min), Max(max), bSet(true) {}

	KBoundingBox(const TVector<Vec3>& points);
	KBoundingBox(const TVector<KBoundingBox<T>>& boxes);

	void Reset();
	bool IsSet() const { return bSet; }

	void Update(const Vec3& point);
	void Update(const KBoundingBox<T>& box);

	bool Overlaps(const KBoundingBox<T>& other, T epsilon = Epsilon<T>()) const;
	bool Contains(const Vec3& point, T epsilon = Epsilon<T>()) const;

	// gets 6 planes representing the faces of the box
	void GetPlanes(TVector<Plane>& planes) const;
	void GetPlanes(Plane* planes) const;
	void GetEdges(TVector<class KLineSegment<T>>& edges) const;
	T GetArea() const;

	Vec3 GetCenter() const;

	bool Intersects(const KLineSegment<T>& line) const;

	T DistanceToBoxSq(const KBoundingBox<T>& box) const;
	T DistanceToBox(const KBoundingBox<T>& box) const;

	void ClosestPointOnBoxToExternalPoint(const Vec3& point, Vec3& out) const;

	template <typename Type>
	KBoundingBox<Type> ToType()
	{
		KBoundingBox<Type> b;
		b.Min = Min.template ToType<Type>();
		b.Max = Max.template ToType<Type>();
		b.bSet = bSet;
		return b;
	}
	
	KBoundingBox<T> operator+(const Vec3& v) const
	{
		KBoundingBox<T> b;
		b.Min = Min + v;
		b.Max = Max + v;
		b.bSet = bSet;
		return b;
	}

	const Vec3 operator[](i32 index) const 
	{ 
		K_ASSERT(index >= 0 && index <= 2, "index out of bounds");
		return (&Min)[index];		
	}
};

typedef KBoundingBox<f32> FBoundingBox;
typedef KBoundingBox<f64> DBoundingBox;
typedef KBoundingBox<GFlt> GBoundingBox;

