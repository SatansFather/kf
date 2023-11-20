#include "line_segment.h"

template <typename T>
bool KLineSegment<T>::Contains(const Vec3& point, T threshold /*= 1e-3f*/) const
{	
	return ClosestPoint(point).DistanceSq(point) <= threshold;
}

template <typename T>
KVec3<T> KLineSegment<T>::ClosestPoint(const Vec3& point, T& d) const
{
	Vec3 dir = b - a;
	d = std::clamp(
		( (point - a).Dot(dir) / dir.LengthSq() ),
		T(0), T(1));
	return a + (dir * d);
}

template <typename T>
KVec3<T> KLineSegment<T>::Direction() const
{
	return (b - a).GetNormalized();
}

template <typename T>
KVec3<T> KLineSegment<T>::DirectionUnNormalized() const
{
	return (b - a);
}

template <typename T>
T KLineSegment<T>::Length() const
{
	return (b - a).Length();
}

template <typename T>
KVec3<T> KLineSegment<T>::GetPoint(T dist) const
{
	return a + ((b - a) * dist);
}

template <typename T>
bool KLineSegment<T>::Equals(const KLineSegment<T>& other, T epsilon /*= Epsilon<T>()*/) const
{
	return ( a.Equals(other.a, epsilon) && b.Equals(other.b, epsilon) )
		|| ( b.Equals(other.a, epsilon) && a.Equals(other.b, epsilon) );
}

template <typename T>
KVec3<T> KLineSegment<T>::GetPointAtAxis(u8 axis, T value) const
{
	/*
	*	when this line crosses an axis at a given value, find the point
	* 
	*		use these equations:
	*			x = ax + t * dx
	*			y = ay + t * dy
	*			z = az + t * dz
	* 
	*		where 'a' is the segment start
	*		'd' is direction
	*		't' is distance along line
	*		
	*		rearrange known axis equation to find t, so the x equation is:
	*			t = (x - ax) / dx
	*/

	Vec3 out;
	out[axis] = value;
	Vec3 dir = Direction();

	T t = (value - a[axis]) / dir[axis];

	for (u8 i = 0; i < 3; i++)
	{
		if (i == axis) continue;
		out[i] = a[i] + (t * dir[i]);
	}
	return out;
}

template <typename T>
KLineSegment<T> KLineSegment<T>::AdjustLineZ(T bias)
{
	return KLineSegment<T>(a.AdjustZ(bias), b.AdjustZ(bias));
}


template struct KLineSegment<f32>;
template struct KLineSegment<f64>;