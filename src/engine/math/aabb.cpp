#include "aabb.h"
#include "engine/math/math.h"

template <typename T>
KBoundingBox<T>::KBoundingBox(const TVector<KVec3<T>>& points)
{
	for (const Vec3& v : points) Update(v);
}

template <typename T>
KBoundingBox<T>::KBoundingBox(const TVector<KBoundingBox<T>>& boxes)
{
	for (const KBoundingBox<T>& b : boxes) Update(b);
}

template <typename T>
void KBoundingBox<T>::Reset()
{
	Min = Vec3(0,0,0);
	Max = Vec3(0,0,0);
	bSet = false;
}

template <typename T>
bool KBoundingBox<T>::Overlaps(const KBoundingBox<T>& other, T epsilon) const
{
	// TODO this can be done with SIMD instructions
	// this function is the biggest percentage of all trace time by a very large margin
	// apparently AVX/AVX2 (256 bit width) works on intel and amd processors from 2012 and later

	if (!bSet) return false;

	/*if constexpr (sizeof(T) == 4)
	{
		__m128 aMin = _mm_load_ps(&Min.x);
		__m128 aMax = _mm_load_ps(&Max.x);
		__m128 bMin = _mm_load_ps(&other.Min.x);
		__m128 bMax = _mm_load_ps(&other.Max.x);

		__m128 a = _mm_cmplt_ps(aMin, bMax);
		__m128 b = _mm_cmpgt_ps(aMax, bMin);
		a = _mm_or_ps(a, b);

		__m128i i = _mm_castps_si128(a);
		__m128i ii = _mm_setzero_si128();
		__m128i neq = _mm_xor_si128(i, ii);
		return _mm_testz_si128(neq, neq) == 0;
	}*/

	return Min.x < (other.Max.x + epsilon)
		&& Min.y < (other.Max.y + epsilon)
		&& Min.z < (other.Max.z + epsilon)
		&& (Max.x + epsilon) > other.Min.x
		&& (Max.y + epsilon) > other.Min.y
		&& (Max.z + epsilon) > other.Min.z;
}	

template <typename T>
bool KBoundingBox<T>::Contains(const KVec3<T>& point, T epsilon) const
{
	if (!bSet) return false;

	return point.x < (Max.x + epsilon) && point.x > (Min.x - epsilon)
		&& point.y < (Max.y + epsilon) && point.y > (Min.y - epsilon)
		&& point.z < (Max.z + epsilon) && point.z > (Min.z - epsilon);
}

template <typename T>
void KBoundingBox<T>::Update(const KVec3<T>& point)
{
	if (!bSet)
	{
		Min = point;
		Max = point;
	}
	else
	{
		if (point.x < Min.x) Min.x = point.x;
		if (point.y < Min.y) Min.y = point.y;
		if (point.z < Min.z) Min.z = point.z;
		if (point.x > Max.x) Max.x = point.x;
		if (point.y > Max.y) Max.y = point.y;
		if (point.z > Max.z) Max.z = point.z;
	}

	bSet = true;
}

template <typename T>
void KBoundingBox<T>::Update(const KBoundingBox<T>& box)
{
	if (!bSet)
	{
		Min = box.Min;
		Max = box.Max;
	}
	else
	{
		if (Min.x > box.Min.x) Min.x = box.Min.x;
		if (Min.y > box.Min.y) Min.y = box.Min.y;
		if (Min.z > box.Min.z) Min.z = box.Min.z;
		if (Max.x < box.Max.x) Max.x = box.Max.x;
		if (Max.y < box.Max.y) Max.y = box.Max.y;
		if (Max.z < box.Max.z) Max.z = box.Max.z;
	}

	bSet = true;
}

template <typename T>
void KBoundingBox<T>::GetPlanes(TVector<Plane>& planes) const
{
	planes.clear();
	planes.resize(6);

	// x plane
	planes[0] = Plane(Vec3(Max.x, Max.y, Max.z), Vec3(1, 0, 0));

	// -x plane
	planes[1] = Plane(Vec3(Min.x, Max.y, Max.z), Vec3(-1, 0, 0));

	// y plane
	planes[2] = Plane(Vec3(Max.x, Max.y, Max.z), Vec3(0, 1, 0));

	// -y plane
	planes[3] = Plane(Vec3(Max.x, Min.y, Max.z), Vec3(0, -1, 0));

	// z plane
	planes[4] = Plane(Vec3(Max.x, Max.y, Max.z), Vec3(0, 0, 1));

	// -z plane
	planes[5] = Plane(Vec3(Max.x, Max.y, Min.z), Vec3(0, 0, -1));
}

template <typename T>
void KBoundingBox<T>::GetPlanes(Plane* planes) const
{
	// x plane
	planes[0] = Plane(Vec3(Max.x, Max.y, Max.z), Vec3(1, 0, 0));

	// -x plane
	planes[1] = Plane(Vec3(Min.x, Max.y, Max.z), Vec3(-1, 0, 0));

	// y plane
	planes[2] = Plane(Vec3(Max.x, Max.y, Max.z), Vec3(0, 1, 0));

	// -y plane
	planes[3] = Plane(Vec3(Max.x, Min.y, Max.z), Vec3(0, -1, 0));

	// z plane
	planes[4] = Plane(Vec3(Max.x, Max.y, Max.z), Vec3(0, 0, 1));

	// -z plane
	planes[5] = Plane(Vec3(Max.x, Max.y, Min.z), Vec3(0, 0, -1));
}


template <typename T>
void KBoundingBox<T>::GetEdges(TVector<class KLineSegment<T>>& edges) const
{
	using LineSegment = KLineSegment<T>;

	T ax = Min.x;
	T ay = Min.y;
	T az = Min.z;
	T bx = Max.x;
	T by = Max.y;
	T bz = Max.z;

	edges =
	{
		// bottom square
		LineSegment(Vec3(ax, ay, az), Vec3(bx, ay, az)),
		LineSegment(Vec3(ax, ay, az), Vec3(ax, by, az)),
		LineSegment(Vec3(ax, by, az), Vec3(bx, by, az)),
		LineSegment(Vec3(bx, ay, az), Vec3(bx, by, az)),

		// top square												 
		LineSegment(Vec3(ax, ay, bz), Vec3(bx, ay, bz)),
		LineSegment(Vec3(ax, ay, bz), Vec3(ax, by, bz)),
		LineSegment(Vec3(bx, ay, bz), Vec3(bx, by, bz)),
		LineSegment(Vec3(ax, by, bz), Vec3(bx, by, bz)),

		// connections
		LineSegment(Vec3(ax, ay, az), Vec3(ax, ay, bz)),
		LineSegment(Vec3(bx, ay, az), Vec3(bx, ay, bz)),
		LineSegment(Vec3(bx, by, az), Vec3(bx, by, bz)),
		LineSegment(Vec3(ax, by, az), Vec3(ax, by, bz)),
	};
}

template <typename T>
T KBoundingBox<T>::GetArea() const
{
	return abs(Max.x - Min.x) * abs(Max.y - Min.y) * abs(Max.z - Min.z);
}

template <typename T>
bool KBoundingBox<T>::Intersects(const KLineSegment<T>& line) const
{	
	Vec3 e = Max - Min;
	Vec3 d = line.b - line.a;
	Vec3 m = line.a + line.b - Min - Max;

	T ad[3];
	for (u8 i = 0; i < 3; i++)
	{
		ad[i] = abs(d[i]);
		if (abs(m[i]) > e[i] + ad[i]) return false;
	}
	
	for (u8 i = 0; i < 3; i++) ad[i] += Epsilon<T>();

	if (abs(m.y * d.z - m.z * d.y) > e.y * ad[2] + e.z * ad[1]) return false;
	if (abs(m.z * d.x - m.x * d.z) > e.x * ad[2] + e.z * ad[0]) return false;
	if (abs(m.x * d.y - m.y * d.x) > e.x * ad[1] + e.y * ad[0]) return false;

	return true;
}

template <typename T>
KVec3<T> KBoundingBox<T>::GetCenter() const
{
	return Min + ((Max - Min) / 2);
}

template <typename T>
T KBoundingBox<T>::DistanceToBoxSq(const KBoundingBox<T>& box) const
{
	Vec3 span;
	
	for (u32 i = 0; i < 3; i++)
	{
		T max = KMax(box.Min[i] - Max[i], Min[i] - box.Max[i]);
		span[i] = KMax(0, max);
	}

	return span.LengthSq();
}

template <typename T>
T KBoundingBox<T>::DistanceToBox(const KBoundingBox<T>& box) const
{
	return sqrt(DistanceToBoxSq(box));
}

template <typename T>
void KBoundingBox<T>::ClosestPointOnBoxToExternalPoint(const Vec3& point, Vec3& out) const
{
	for (u32 i = 0; i < 3; i++) 
	{
		T v = point[i];
		v = KMax(v, Min[i]);
		v = KMin(v, Max[i]);
		out[i] = v;
	}
}

template struct KBoundingBox<f32>;
template struct KBoundingBox<f64>;