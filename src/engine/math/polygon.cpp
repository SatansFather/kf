#include "polygon.h"
#include "plane.h"

template <typename T>
using Line = KLine<T>;

template <typename T>
bool IntersectLineLine2D(const KVec2<T>& a1, const KVec2<T>& a2, const KVec2<T>& b1, const KVec2<T>& b2, KVec2<T>& out)
{
	T u = (b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x);
	T v = (a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x);

	T det = (b2.y - b1.y) * (a2.x - a1.x) - (b2.x - b1.x) * (a2.y - a1.y);
	if (abs(det) < Epsilon<T>()) return false;

	det = T(1) / det;
	out.x = u * det;
	out.y = v * det;

	return true;
}

template <typename T>
bool IntersectLineSegmentLineSegment2D(const KVec2<T>& a1, const KVec2<T>& a2, const KVec2<T>& b1, const KVec2<T>& b2, KVec2<T>& out)
{
	bool ret = IntersectLineLine2D(a1, a2, b1, b2, out);
	return ret && out.x >= T(0) && out.x <= T(1) && out.y >= T(0) && out.y <= T(1);
}

template <typename T>
bool IsAnEar(const TVector<KVec2<T>>& poly, int i, int j)
{
	KVec2<T> dummy;
	i32 x = (i32)poly.size() - 1;
	for (i32 y = 0; y < i; ++y)
	{
		if (IntersectLineSegmentLineSegment2D(poly[i], poly[j], poly[x], poly[y], dummy))
			return false;
		x = y;
	}
	x = j + 1;
	for (i32 y = x + 1; y < (i32)poly.size(); ++y)
	{
		if (IntersectLineSegmentLineSegment2D(poly[i], poly[j], poly[x], poly[y], dummy)) return false;
		x = y;
	}
	return true;
}

template <typename T>
KVec3<T> KPolygon<T>::ExtremePoint(const Vec3& direction, T& proj_dist) const
{
	KVec3<T> most;
	proj_dist = MIN_I32;
	for (i32 i = 0; i < NumVertices(); ++i)
	{
		KVec3<T> pt = Vertex(i);
		T d = (direction | pt);
		if (d > proj_dist)
		{
			proj_dist = d;
			most = pt;
		}
	}
	return most;
}

template <typename T>
KVec3<T> KPolygon<T>::ExtremePoint(const Vec3& direction) const
{
	T proj_dist;
	return ExtremePoint(direction, proj_dist);
}

template <typename T>
struct KPlane<T> KPolygon<T>::PlaneCCW() const
{
	if (Points.size() > 3)
	{
		KPlane<T> plane;
		for (size_t i = 0; i < Points.size() - 2; ++i)
		{
			for (size_t j = i + 1; j < Points.size() - 1; ++j)
			{
				KVec3<T> pij = KVec3<T>(Points[j]) - KVec3<T>(Points[i]);
				for (size_t k = j + 1; k < Points.size(); ++k)
				{
					plane.Normal = (pij ^ (KVec3<T>(Points[k])) - KVec3<T>(Points[i]));
					T lenSq = plane.Normal.LengthSq();
					if (lenSq > Epsilon<T>())
					{
						plane.Normal /= sqrt(lenSq);
						plane.D = plane.Normal | (KVec3<T>(Points[i]));
						return plane;
					}
				}
			}
		}

		KVec3<T> dir = (KVec3<T>(Points[1]) - KVec3<T>(Points[0])).GetNormalized();
		return KPlane<T>(KLine<T>(Points[0], dir), dir.Perpendicular());
	}

	if (Points.size() == 3)
	{
		return Plane(Points[0], Points[1], Points[2]);
	}
	if (Points.size() == 2)
	{
		Vec3 dir = (Vec3(Points[1]) - Vec3(Points[0])).GetNormalized();
		return Plane(KLine<T>(Points[0], dir), dir.Perpendicular());
	}
	if (Points.size() == 1)
	{
		return Plane(Points[0], Vec3(0, 1, 0));
	}
	return KPlane<T>();
}

template <typename T>
u32 KPolygon<T>::NumVertices() const
{
	return Points.size();
}

template <typename T>
TVector<KTriangle<T>> KPolygon<T>::Triangulate() const
{
	TVector<KTriangle<T>> t;

	if (NumVertices() < 3) return t;

	if (NumVertices() == 3)
	{
		t.push_back(KTriangle<T>(Vertex(0), Vertex(1), Vertex(2)));
		return t;
	}

	TVector<Vec2> p2d;
	TVector<i32> indices;

	for (i32 v = 0; v < NumVertices(); ++v)
	{
		p2d.push_back(MapTo2D(T(v)));
		indices.push_back(v);
	}

	i32 i = 0;
	i32 j = 1;
	i32 k = 2;
	size_t numTries = 0;
	while (p2d.size() > 3 && numTries < p2d.size())
	{
		if (Vec2::OrientedCCW(p2d[i], p2d[j], p2d[k]) && IsAnEar(p2d, i, k))
		{
			t.push_back(KTriangle<T>(Points[indices[i]], Points[indices[j]], Points[indices[k]]));
			p2d.erase(p2d.begin() + j);
			indices.erase(indices.begin() + j);

			if (i > 0)
			{
				i = (i + (i32)p2d.size() - 1) % p2d.size();
				j = (j + (i32)p2d.size() - 1) % p2d.size();
				k = (k + (i32)p2d.size() - 1) % p2d.size();
			}
			numTries = 0;
		}
		else
		{
			i = j;
			j = k;
			k = (k + 1) % p2d.size();
			++numTries;
		}
	}

	if (p2d.size() > 3) return t;

	t.push_back(KTriangle<T>(Points[indices[0]], Points[indices[1]], Points[indices[2]]));

	return t;
}

template <typename T>
KVec3<T> KPolygon<T>::ClosestPoint(const Vec3& point) const
{
	Vec3 closestPoint;
	if (Points.size() < 3) return closestPoint;
	//TVector<KTriangle<T>> tris = Triangulate();

	TVector<KTriangle<T>> tris;
	for (u32 i = 2; i < Points.size(); i++)
	{
		tris.push_back(KTriangle<T>(
			Points[0],
			Points[i - 1],
			Points[i]));
	}

	T closestDist = MAX_U32;
	for (u32 i = 0; i < tris.size(); i++)
	{
		Vec3 pt = tris[i].ClosestPoint(point);
		T d = pt.DistanceSq(point);
		if (d < closestDist)
		{
			closestPoint = pt;
			closestDist = d;
		}
	}
	return closestPoint;
}

template <typename T>
T KPolygon<T>::Distance(const Vec3& point) const
{
	Vec3 pt = ClosestPoint(point);
	return pt.Distance(point);
}

template <typename T>
KVec3<T> KPolygon<T>::BasisV(const KVec3<T>& norm) const
{
	if (Points.size() < 2) return Vec3(0, 1, 0);
	return ((norm == 0 ? PlaneCCW().Normal : norm) ^ BasisU()).GetNormalized();
}

template <typename T>
KVec3<T> KPolygon<T>::BasisU() const
{
	if (Points.size() < 2) return Vec3(1, 0, 0);
	Vec3 u = (Vec3)Points[1] - (Vec3)Points[0];
	u.Normalize(); // Always succeeds, even if u was zero (generates (1,0,0)).
	return u;
}

template <typename T>
bool KPolygon<T>::Contains(const Vec3& point, T thicknessSq /*= Epsilon<T>*/, const KVec3<T>& norm) const
{
	if (Points.size() < 3)
		return false;

	// TODO try constructing planes from each edge and returning false if point is in front of any
	// would only work with convex (but thats all there ever is)
	// benchmark


	Vec3 basisU = BasisU();
	Vec3 basisV = BasisV(norm);
	Vec3 normal = norm == 0 ? basisU.Cross(basisV) : norm;

	T dot = (normal | (Vec3(Points[0])) - point);
	if (dot * dot > thicknessSq)
		return false; 

	int numIntersections = 0;

	const T epsilon = Epsilon<T>();

	Vec3 vt = Vec3(Points.back()) - point;
	Vec2 p0 = Vec2((vt | basisU), (vt | basisV));
	if (abs(p0.y) < epsilon)
		p0.y = -epsilon;

	for (i32 i = 0; i < (i32)Points.size(); ++i)
	{
		vt = Vec3(Points[i]) - point;
		Vec2 p1 = Vec2((vt | basisU), (vt | basisV));
		if (abs(p1.y) < epsilon)
			p1.y = -epsilon; 

		if (p0.y * p1.y < T(0))
		{
			if ((std::min)(p0.x, p1.x) > T(0))
				++numIntersections;
			else if ((std::max)(p0.x, p1.x) > T(0)) 
			{
				Vec2 d = p1 - p0;
				if (d.y != T(0))
				{
					T t = -p0.y / d.y; 
					T x = p0.x + t * d.x;
					if (t >= T(0) && t <= T(1) && x > T(0))
						++numIntersections;
				}
			}
		}
		p0 = p1;
	}

	return numIntersections % 2 == 1;
}

template <typename T>
KVec3<T> KPolygon<T>::MapFrom2D(const Vec2& point) const
{
	return (Vec3)Points[0] + (BasisU() * point.x) + (BasisV() * point.y);
}

template <typename T>
KVec2<T> KPolygon<T>::MapTo2D(const Vec3& point, bool side) const
{
	Vec3 basisU = BasisU();
	Vec3 basisV = BasisV();
	Vec3 pt = point - Points[0];
	return KVec2<T>((pt | (side ? basisV : basisU)), (pt | (side ? basisU : basisV)));
}

template <typename T>
KVec3<T> KPolygon<T>::Vertex(u32 index) const
{
	return Points[index];
}

template <typename T>
KLineSegment<T> KPolygon<T>::Edge(u32 index) const
{
	if (Points.empty()) return KLineSegment<T>(Vec3(T(0)), Vec3(T(0)));
	if (Points.size() == 1) return KLineSegment<T>(Points[T(0)], Points[T(0)]);
	return KLineSegment<T>(Points[index], Points[(index + 1) % Points.size()]);
}

template <typename T>
i32 KPolygon<T>::NumEdges() const
{
	return Points.size();
}

template <typename T>
EPolyClassification KPolygon<T>::ClassifyToPlane(const KPlane<T>& plane, T epsilon) const
{
	// check where vertices sit in relation to the plane
	u32 frontcount = 0, behindcount = 0;
	u32 vertcount = NumVertices();
	for (u32 i = 0; i < vertcount; i++)
	{
		Vec3 p = Vertex(i);
		switch (plane.ClassifyPoint(p, epsilon))
		{
			case EPointSide::Front:
			{
				frontcount++;
				break;
			}
			case EPointSide::Behind:
			{
				behindcount++;
				break;
			}
		}
	}

	if (behindcount > 0 && frontcount > 0) return EPolyClassification::Spanning;
	if (frontcount > 0) return EPolyClassification::Front;
	if (behindcount > 0) return EPolyClassification::Behind;
	return EPolyClassification::Coplanar; // front and back == 0
}

template <typename T>
bool KPolygon<T>::Intersects(const KPolygon<T>& poly, T thickness) const 
{	
	Plane p1 = PlaneCCW();
	Plane p2 = poly.PlaneCCW();

	if (p2.Normal.Equals(p1.Normal)) return false;

	for (u32 i = 0; i < poly.NumEdges(); i++)
	{
		LineSegment line = poly.Edge(i);
		T t;
		bool intersects = p1.IntersectLinePlane(p1.Normal, p1.D, line.a, line.b - line.a, t, thickness);
		if (!intersects || t < T(0) || t > T(1)) continue;
		if (Contains(line.GetPoint(t), thickness)) return true;
	}

	for (u32 i = 0; i < NumEdges(); i++)
	{
		LineSegment line = Edge(i);
		T t;
		bool intersects = p2.IntersectLinePlane(p2.Normal, p2.D, line.a, line.b - line.a, t, thickness);
		if (!intersects || t < T(0) || t > T(1)) continue;
		if (poly.Contains(line.GetPoint(t), thickness)) return true;
	}
	return false;
}

template <typename T>
bool KPolygon<T>::IsConvex() const
{
	if (Points.empty()) return false;
	if (Points.size() <= 3) return true;

	u32 i = Points.size() - 2;
	u32 j = Points.size() - 1;
	u32 k = 0;

	while (k < Points.size())
	{
		KVec2<T> a = MapTo2D(Points[i]);
		KVec2<T> b = MapTo2D(Points[j]);
		KVec2<T> c = MapTo2D(Points[k]);
		if (!KVec2<T>::OrientedCCW(a, b, c)) return false;
		i = j;
		j = k;
		k++;
	}
	return true;
}

template <typename T>
T KPolygon<T>::Area() const
{
	Vec3 area;
	if (Points.size() <= 2) return 0;

	u32 i = NumEdges() - 1;
	for (u32 j = 0; j < NumEdges(); j++)
	{
		area += Vertex(i).Cross(Vertex(j));
		i = j;
	}
	return 0.5 * abs(PlaneCCW().Normal | area);
}

template struct KPolygon<f32>;
template struct KPolygon<f64>;
//template struct KPolygon<x64>;
