#include "triangle.h"
#include "vec2.h"
#include "polygon.h"

template <typename T>
using Vec2 = KVec2<T>;

template <typename T>
inline T TriArea2D(T x1, T y1, T x2, T y2, T x3, T y3)
{
	return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2);
}

template <typename T>
KVec3<T> KTriangle<T>::ClosestPoint(const Vec3& p) const
{
	Vec3 ab = b - a;
	Vec3 ac = c - a;
	Vec3 ap = p - a;
	T d1 = (ab | ap);
	T d2 = (ac | ap);
	if (d1 <= T(0) && d2 <= T(0))
		return a;

	Vec3 bp = p - b;
	T d3 = (ab | bp);
	T d4 = (ac | bp);
	if (d3 >= T(0) && d4 <= d3)
		return b;

	T vc = d1 * d4 - d3 * d2;
	if (vc <= T(0) && d1 >= T(0) && d3 <= T(0))
	{
		T v = d1 / (d1 - d3);
		return a + (ab * v); 
	}

	Vec3 cp = p - c;
	T d5 = (ab | cp);
	T d6 = (ac | cp);
	if (d6 >= T(0) && d5 <= d6)
		return c; 

	T vb = d5 * d2 - d1 * d6;
	if (vb <= T(0) && d2 >= T(0) && d6 <= T(0))
	{
		T w = d2 / (d2 - d6);
		return a + (ac * w); 
	}

	T va = d3 * d6 - d5 * d4;
	if (va <= T(0) && d4 - d3 >= T(0) && d5 - d6 >= T(0))
	{
		T w = (d4 - d3) / (d4 - d3 + d5 - d6);
		return b + ((c - b) * w);
	}

	T denom = T(1) / (va + vb + vc);
	T v = vb * denom;
	T w = vc * denom;
	return a + (ab * v) + (ac * w);
}

template <typename T>
class KVec3<T> KTriangle<T>::BarycentricUVW(const Vec3& point) const
{
	Vec3 m = (b - a) ^ (c - a);

	T nu, nv, ood;

	T x = abs(m.x);
	T y = abs(m.y);
	T z = abs(m.z);

	if (x >= y && x >= z)
	{
		// Project to the yz plane.
		nu = TriArea2D(point.y, point.z, b.y, b.z, c.y, c.z);
		nv = TriArea2D(point.y, point.z, c.y, c.z, a.y, a.z);
		ood = T(1) / m.x;
	}
	else if (y >= z)
	{
		nu = TriArea2D(point.x, point.z, b.x, b.z, c.x, c.z);
		nv = TriArea2D(point.x, point.z, c.x, c.z, a.x, a.z);
		ood = T(1) / -m.y;
	}
	else
	{
		nu = TriArea2D(point.x, point.y, b.x, b.y, c.x, c.y);
		nv = TriArea2D(point.x, point.y, c.x, c.y, a.x, a.y);
		ood = T(1) / m.z;
	}
	T u = nu * ood;
	T v = nv * ood;
	T w = T(1) - u - v;
	return Vec3(u, v, w);
}

template <typename T>
KVec2<T> KTriangle<T>::BarycentricUV(const Vec3& point) const
{
	Vec3 uvw = BarycentricUVW(point);
	return KVec2<T>(uvw.y, uvw.z);
}

template <typename T>
bool KTriangle<T>::Contains(const KVec3<T>& point, const KVec3<T>& normal) const
{
	KPolygon<T> poly;
	poly.Points = { a, b, c };
	return poly.Contains(point, 10, normal);
}

template struct KTriangle<f32>;
template struct KTriangle<f64>;
//template struct KTriangle<x64>;