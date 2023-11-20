#pragma once

#include "vec3.h"
#include "triangle.h"
#include "plane.h"
#include "engine/global/types_container.h"
#include "line_segment.h"

template <typename T>
struct KPolygon
{
	using Vec3 = KVec3<T>;
	using Vec2 = KVec2<T>;
	using Plane = KPlane<T>;
	using LineSegment = KLineSegment<T>;

	TVector<Vec3> Points;

	KPolygon() {}
	Vec3 Vertex(u32 vertexIndex) const;
	struct KVec2<T> MapTo2D(const Vec3& point, bool side = false) const;
	i32 NumEdges() const;
	LineSegment Edge(u32 index) const;
	Vec3 MapFrom2D(const Vec2& point) const;
	bool Contains(const Vec3& point, T thicknessSq = Epsilon<T>(), const KVec3<T>& norm = 0) const;
	Vec3 BasisU() const;
	Vec3 BasisV(const KVec3<T>& norm = 0) const;
	T Distance(const Vec3& point) const;
	Vec3 ClosestPoint(const Vec3& point) const;
	TVector<KTriangle<T>> Triangulate() const;
	u32 NumVertices() const;
	KPlane<T> PlaneCCW() const;
	Vec3 ExtremePoint(const Vec3& direction) const;
	Vec3 ExtremePoint(const Vec3& direction, T& proj_dist) const;
	bool Intersects(const KPolygon<T>& poly, T thickness = Epsilon<T>()) const;
	EPolyClassification ClassifyToPlane(const KPlane<T>& plane, T epsilon = Epsilon<T>()) const;
	bool IsConvex() const;
	Vec3 Center() const;
	T Area() const;

	template <typename Type>
	KPolygon<Type> ToType()
	{
		KPolygon<Type> p;
		p.Points.reserve(Points.size());
		for (Vec3 v : Points)
			p.Points.push_back(v.template ToType<GFlt>());
		return p;
	}
};

template <typename T>
KVec3<T> KPolygon<T>::Center() const
{
	Vec3 center;
	for (const Vec3& p : Points) center += p;
	center /= Points.size();
	return center;
}

typedef KPolygon<f32> FPolygon;
typedef KPolygon<f64> DPolygon;
typedef KPolygon<GFlt> GPolygon;