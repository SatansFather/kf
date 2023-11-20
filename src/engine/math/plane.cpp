#include "plane.h"
#include "mat3x3.h"
#include "polygon.h"
#include "engine/utility/k_assert.h"


template <typename T>
KPlane<T>::KPlane(const Vec3& normal, T d) : Normal(normal), D(d) {}

template <typename T>
KPlane<T>::KPlane(const Vec3& v1, const Vec3& v2, const Vec3& v3)
{
	Set(v1, v2, v3);
}

template <typename T>
KPlane<T>::KPlane(const Vec3& point, const Vec3& normal)
{
	Set(point, normal);
}

template <typename T>
KPlane<T>::KPlane(const Line& line, const Vec3& normal)
{
	Vec3 perpnorm = normal - normal.ProjectToNorm(line.Direction);
	Set(line.Position, perpnorm.GetNormalized());
}

template <typename T>
T KPlane<T>::SignedDistance(const Vec3& point) const
{
	return (Normal | point) - D;
}

template <typename T>
T KPlane<T>::Distance(const Vec3& point) const
{
	return abs(SignedDistance(point));
}

template <typename T>
bool KPlane<T>::Contains(const Vec3& point, T epsilon) const
{
	return Distance(point) <= epsilon;
}

template <typename T>
bool KPlane<T>::IsParallel(const Plane& plane, T epsilon) const
{
	return Normal.Equals(plane.Normal, epsilon);
}

template <typename T>
bool KPlane<T>::Intersects(const Plane& plane, Line* out_line /*= 0*/) const
{
	Vec3 perp = Normal.Perpendicular(plane.Normal);

	KMat3x3<T> m;
	m.SetRow(0, (T*)(&Normal));
	m.SetRow(1, (T*)(&plane.Normal));
	m.SetRow(2, (T*)(&perp));
	Vec3 intersection;
	bool success = m.SolveAxb(Vec3(D, plane.D, T(0)), intersection);
	if (!success) // Inverse failed, so the planes must be parallel.
	{
		T norm_dir = Normal | plane.Normal;
		if ((norm_dir > T(0) && EqualAbs(D, plane.D)) || (norm_dir < T(0) && EqualAbs(D, -plane.D)))
		{
			if (out_line) *out_line = Line(Normal * D, plane.Normal.Perpendicular());
			return true;
		}
		else 
		{
			return false;
		}
	}
	if (out_line) *out_line = Line(intersection, perp.GetNormalized());

	return true;
}

template <typename T>
bool KPlane<T>::Intersects(const Plane& plane, const Plane& plane2, Vec3* out_point /*= 0*/) const
{
	Line dummy;

	if (IsParallel(plane) || IsParallel(plane2))
	{
		if (EqualAbs(D, plane.D) || EqualAbs(D, plane2.D))
		{
			bool intersect = plane.Intersects(plane2, &dummy);
			if (intersect && out_point)
				*out_point = dummy.GetPoint(0);
			return intersect;
		}
		else
			return false;
	}
	if (plane.IsParallel(plane2))
	{
		if (EqualAbs(plane.D, plane2.D))
		{
			bool intersect = this->Intersects(plane, &dummy);
			if (intersect && out_point)
				*out_point = dummy.GetPoint(0);
			return intersect;
		}
		else
			return false;
	}

	KMat3x3<T> m;
	m.SetRow(0, (T*)(&Normal));
	m.SetRow(1, (T*)(&plane.Normal));
	m.SetRow(2, (T*)(&plane2.Normal));
	Vec3 intersectionPos;
	bool success = m.SolveAxb(Vec3(D, plane.D, plane2.D), intersectionPos);
	if (!success)
		return false;
	if (out_point)
		*out_point = intersectionPos;
	return true;
}

template <typename T>
void KPlane<T>::Set(const Vec3& point, const Vec3& normal)
{
	Normal = normal;
	D = point | Normal;
}

template <typename T>
void KPlane<T>::Set(const Vec3& v1, const Vec3& v2, const Vec3& v3)
{
	Normal = ((v2 - v1) ^ (v3 - v1)) * T(-1);
	T len = Normal.Length();
	K_ASSERT(len > 0, "plane normal divide by zero");
	Normal /= len;
	D = Normal | v1;
}

template <typename T>
EPointSide KPlane<T>::ClassifyPoint(const Vec3& point, T epsilon) const
{
	T dist = SignedDistance(point);

	if (dist > epsilon) return EPointSide::Front;
	if (dist < -epsilon) return EPointSide::Behind;
	return EPointSide::On;
}

template <typename T>
bool KPlane<T>::Intersects(const KLineSegment<T>& line, T& dist, T epsilon) const
{
	T t;
	bool success = IntersectLinePlane(Normal, D, line.a, line.Direction(), t, epsilon);
	const T len = line.Length();
	dist = t / len;
	return success && t >= 0.f && t <= len;
}

template <typename T>
bool KPlane<T>::IntersectLinePlane(const KVec3<T>& planeNormal, T planeD, const KVec3<T>& linePos, const KVec3<T>& lineDir, T& t, T epsilon) const
{
	T denom = planeNormal | lineDir;
	if (abs(denom) > epsilon)
	{
		// Compute the distance from the line starting point to the point of intersection.
		t = (planeD - (planeNormal | linePos)) / denom;
		return true;
	}

	if (denom != T(0))
	{
		t = (planeD - (planeNormal | linePos)) / denom;
		if (abs(t) < epsilon) return true;
	}
	t = T(0);
	return abs((planeNormal | linePos) - planeD) < epsilon;
}

template <typename T>
bool KPlane<T>::IsCoplanar(const Plane& other, T epsilon /*= Epsilon<T>()*/)
{
	return (Normal.Equals(other.Normal) && abs(D - other.D) < epsilon) ||
		(Normal.Equals(-other.Normal) && abs(-D - other.D) < epsilon);
}

template <typename T>
bool KPlane<T>::EqualsInverse(const KPlane<T>& other, T norm_epsilon /*= 0.000001*/, T dist_epsilon /*= 0.0001*/) const
{
	return Normal.Equals(-other.Normal, norm_epsilon) && abs(D + other.D) < dist_epsilon;
}

template <typename T>
bool KPlane<T>::Equals(const KPlane<T>& other, T norm_epsilon /*= 0.000001*/, T dist_epsilon /*= 0.0001*/) const
{
	return Normal.Equals(other.Normal, norm_epsilon) && abs(D - other.D) < dist_epsilon;
}

template <typename T>
KPlane<T> KPlane<T>::Inverted() const
{
	KPlane<T> out = *this;
	out.D = -out.D;
	out.Normal = -out.Normal;
	return out;
}

template <typename T>
bool KPlane<T>::IsAxial(T epsilon, u8* index) const
{
	for (u8 i = 0; i < 3; i++)
	{
		if (abs(Normal[i] - 1) < epsilon)
		{
			if (index) *index = i;
			return true;
		}
	}
	return false;
}

template <typename T>
KVec3<T> KPlane<T>::PointOnPlane() const
{
	return Normal * D;
}

template <typename T>
void KPlane<T>::ToByteArray(std::array<u8, sizeof(T) * 4>& arr) const
{
	memcpy(arr.data(), &Normal, sizeof(T) * 3);
	memcpy(arr.data() + (sizeof(T) * 3), &D, sizeof(T));
}

template struct KPlane<f32>;
template struct KPlane<f64>;
//template struct KPlane<x64>;
