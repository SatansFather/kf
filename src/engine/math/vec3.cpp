#include "vec3.h"
#include "engine/utility/kstring.h"
#include "math.h"
#include "../utility/random.h"

static inline __m128 LoadVec3(const f32* ptr)
{
	const f32 w = 0;
	__m128 low = _mm_loadl_pi(_mm_setzero_ps(), (const __m64*)ptr); // [_ _ y x]
	__m128 high = _mm_load_ss(ptr + 2); // [_ _ _ z]
	high = _mm_unpacklo_ps(high, _mm_set_ss(w)); // [_ _ w z]
	return _mm_movelh_ps(low, high);
}

static inline void StoreVec3(float* ptr, __m128 v)
{
	_mm_storel_pi((__m64*)ptr, v);
	v = _mm_movehl_ps(v, v);
	_mm_store_ss(ptr + 2, v);
}

template <typename T>
KString KVec3<T>::ToString() const
{
	return "(" + KString(x) + ", " + KString(y) + ", " + KString(z) + ")";
}

template <typename T>
KVec3<T> KVec3<T>::Interpolate(const Vec3& start, const Vec3& target, const T& alpha)
{
	return start + ((target - start) * alpha);
}

template <typename T>
void KVec3<T>::Rotate(T angle, Vec3 axis)
{

	*this = *this * (cos(angle)) + (axis ^ *this) * sin(angle) + axis * (axis | *this) * (T(1) - cos(angle));
}

template <typename T>
KVec3<T> KVec3<T>::GetRotated(T angle, Vec3 axis) const
{
	Vec3 out = Vec3(x, y, z);
	out.Rotate(angle, axis);
	return out;
}

template <typename T>
void KVec3<T>::RotateDegrees(T angle, Vec3 axis)
{
	angle = angle * (PI<T>() / T(180));
	Rotate(angle, axis);
}

template <typename T>
KVec3<T> KVec3<T>::GetRotatedDegrees(T angle, Vec3 axis) const
{
	// convert angle to radians
	angle = angle * (PI<T>() / T(180));
	return GetRotated(angle, axis);
}

template <typename T>
T KVec3<T>::Normalize()
{
	T len = Length();
	if (len == T(0))
	{
		x = T(0);
		y = T(0);
		z = T(0);
	}
	else
	{
		x /= len;
		y /= len;
		z /= len;
	}
	return len;
}

template <typename T>
KVec3<T> KVec3<T>::GetNormalized() const
{
	Vec3 out = Vec3(x, y, z);
	out.Normalize();
	return out;
}

template <typename T>
KVec3<T> KVec3<T>::Cross(const Vec3& a) const
{
	return *this ^ a;
}

template <typename T>
KVec3<T> KVec3<T>::operator^(const Vec3& other) const
{
	return Vec3
	(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}

template <typename T>
T KVec3<T>::Dot(const Vec3& a) const
{
	return *this | a;
}

template <typename T>
T KVec3<T>::operator|(const Vec3& other) const
{
	return 
		x * other.x + 
		y * other.y + 
		z * other.z;
}

template <typename T>
T KVec3<T>::Length() const
{
	return sqrt(LengthSq());
}

template <typename T>
T KVec3<T>::LengthSq() const
{
	return x * x + y * y + z * z;
}


template <typename T>
bool KVec3<T>::operator>=(const T& other) const
{
	return LengthSq() >= (other * other);
}

template <typename T>
bool KVec3<T>::operator>=(const Vec3& other) const
{
	return LengthSq() >= other.LengthSq();
}

template <typename T>
bool KVec3<T>::operator>(const T& other) const
{
	return LengthSq() > (other * other);
}

template <typename T>
bool KVec3<T>::operator>(const Vec3& other) const
{
	return LengthSq() > other.LengthSq();
}

template <typename T>
bool KVec3<T>::operator<=(const T& other) const
{
	return LengthSq() <= (other * other);
}

template <typename T>
bool KVec3<T>::operator<=(const Vec3& other) const
{
	return LengthSq() <= other.LengthSq();
}

template <typename T>
bool KVec3<T>::operator<(const T& other) const
{
	return LengthSq() < (other * other);
}

template <typename T>
bool KVec3<T>::operator<(const Vec3& other) const
{
	return LengthSq() < other.LengthSq();
}

template <typename T>
bool KVec3<T>::operator!=(const Vec3& other) const
{
	return x != other.x || y != other.y || z != other.z;
}

template <typename T>
bool KVec3<T>::operator==(const Vec3& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

template <typename T>
KVec3<T> KVec3<T>::operator/=(const T& other)
{
	x /= other;
	y /= other;
	z /= other;
	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::operator/=(const Vec3& other)
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::operator/(const T& other) const
{
	return Vec3(x / other, y / other, z / other);
}

template <typename T>
KVec3<T> KVec3<T>::operator/(const Vec3& other) const
{
	return Vec3(x / other.x, y / other.y, z / other.z);
}

template <typename T>
KVec3<T> KVec3<T>::operator*=(const T& other)
{
	x *= other;
	y *= other;
	z *= other;

	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::operator*=(const Vec3& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;

	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::operator*(const T& other) const
{
	return Vec3(x * other, y * other, z * other);
}

template <typename T>
KVec3<T> KVec3<T>::operator*(const Vec3& other) const
{
	return Vec3(x * other.x, y * other.y, z * other.z);
}

template <typename T>
KVec3<T> KVec3<T>::operator-=(const T& other)
{
	x -= other;
	y -= other;
	z -= other;
	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::operator-=(const Vec3& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::operator-(const T& other) const
{
	return Vec3(x - other, y - other, z - other);
}

template <typename T>
KVec3<T> KVec3<T>::operator-(const Vec3& other) const
{
	return Vec3(x - other.x, y - other.y, z - other.z);
}

template <typename T>
KVec3<T> KVec3<T>::operator+=(const T& other)
{
	x += other;
	y += other;
	z += other;
	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::operator+=(const Vec3& other)
{	
	x += other.x;
	y += other.y;
	z += other.z;

	// this crap is slower
	/*if constexpr (sizeof(T) == 8)
	{
		// double
		__m256d v1 = _mm256_load_pd(&x);
		__m256d v2 = _mm256_load_pd(&other.x);
		__m256d r = _mm256_add_pd(v1, v2);
		_mm256_store_pd(&x, r);
	}
	else
	{
		// float
		__m128 v1 = _mm_load_ps(&x);
		__m128 v2 = _mm_load_ps(&other.x);
		__m128 r = _mm_add_ps(v1, v2);
		_mm_store_ps(&x, r);
	}*/

	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::operator+(const T& other) const
{
	return Vec3(x + other, y + other, z + other);
}

template <typename T>
KVec3<T> KVec3<T>::operator+(const Vec3& other) const
{
	return Vec3(x + other.x, y + other.y, z + other.z);
}


template <typename T>
KVec3<T> KVec3<T>::operator-() const
{
	return Vec3(-x, -y, -z);
}

template <typename T>
T KVec3<T>::AngleBetween(const Vec3& other) const
{
	T cosa = (*this | other) / sqrt(LengthSq() * other.LengthSq());
	if (cosa >= T(1))
		return T(0);
	else if (cosa <= T(-1))
		return PI<T>();
	else
		return acos(cosa);
}

template <typename T>
bool KVec3<T>::Equals(const Vec3& other, T epsilon) const
{
	return
		(abs(x - other.x) < epsilon) &&
		(abs(y - other.y) < epsilon) &&
		(abs(z - other.z) < epsilon);
}

template <typename T>
KVec3<T> KVec3<T>::Perpendicular(const Vec3& hint /*= Vec3(0, 1, 0)*/, const Vec3 hint2 /*= Vec3(0, 0, 1)*/) const
{
	Vec3 v = *this ^ hint;
	T len = v.Length();
	v.Normalize();
	if (len == T(0)) return hint2;
	else return v;
}

template <typename T>
KVec3<T> KVec3<T>::ProjectToNorm(const Vec3& direction) const
{
	return direction * (*this | direction);
}

template <typename T>
T KVec3<T>::DistanceSq(const Vec3& point) const
{
	T dx = x - point.x;
	T dy = y - point.y;
	T dz = z - point.z;
	return dx * dx + dy * dy + dz * dz;
}

template <typename T>
T KVec3<T>::Distance(const Vec3& point) const
{
	return sqrt(DistanceSq(point));
}

template <typename T>
KVec3<T> KVec3<T>::XY() const
{
	return Vec3(x, y, 0);
}

template <typename T>
KVec3<T> KVec3<T>::ScaleZ(T scale) const
{
	return Vec3(x, y, z * scale);
}

template <typename T>
KVec3<T> KVec3<T>::ScaleY(T scale) const
{
	return Vec3(x, y * scale, z);
}

template <typename T>
KVec3<T> KVec3<T>::ScaleX(T scale) const
{
	return Vec3(x * scale, y, z);
}

template <typename T>
KVec3<T> KVec3<T>::SetZ(T value) const
{
	return Vec3(x, y, value);
}

template <typename T>
KVec3<T> KVec3<T>::SetY(T value) const
{
	return Vec3(x, value, z);
}

template <typename T>
KVec3<T> KVec3<T>::SetX(T value) const
{
	return Vec3(value, y, z);
}

template <typename T>
KVec3<T> KVec3<T>::AdjustZ(T amount) const
{
	return Vec3(x, y, z + amount);
}

template <typename T>
KVec3<T> KVec3<T>::AdjustY(T amount) const
{
	return Vec3(x, y + amount, z);
}

template <typename T>
KVec3<T> KVec3<T>::AdjustX(T amount) const
{
	return Vec3(x + amount, y, z);
}

template <typename T>
KVec3<T> KVec3<T>::Adjust(T amountX, T amountY, T amountZ) const
{
	return Vec3(x + amountX, y + amountY, z + amountZ);
}

template <typename T>
KVec3<T> KVec3<T>::GetClamped(Vec3 min, Vec3 max) const
{
	Vec3 v = *this;
	v.Clamp(min, max);
	return v;
}

template <typename T>
void KVec3<T>::Clamp(Vec3 min, Vec3 max)
{
	x = std::clamp(x, min.x, max.x);
	y = std::clamp(y, min.y, max.y);
	z = std::clamp(z, min.z, max.z);
}

template <typename T>
KVec3<T> KVec3<T>::Abs() const
{
	return Vec3(abs(x), abs(y), abs(z));
}

template <typename T>
KVec3<T> KVec3<T>::FromPitchYaw(T pitch, T yaw)
{
	T sp = sin(pitch);
	T sy = sin(yaw);
	T cp = cos(pitch);
	T cy = cos(yaw);
	return KVec3<T>(cp * cy, cp * sy, sp );
}

template <typename T>
void KVec3<T>::ToPitchYaw(T& pitch, T& yaw) const
{
	GetNormalized().ToPitchYawNormalized(pitch, yaw);
}

template <typename T>
void KVec3<T>::ToPitchYawNormalized(T& pitch, T& yaw) const
{
	yaw = atan2(y, x);
	pitch = atan2(z, sqrt(x * x + y * y));
}

template <typename T>
KVec3<T> KVec3<T>::GetClampedToMaxLength(GFlt len) const
{
	if (len < 1e-4)
	{
		return Vec3(0, 0, 0);
	}

	const GFlt vsq = LengthSq();
	if (vsq > len * len)
	{
		const GFlt scale = len * GFlt(1) / sqrt(vsq);
		return Vec3(x * scale, y * scale, z * scale);
	}

	return *this;
}

template <typename T>
KVec3<T> KVec3<T>::Lerp(const KVec3<T>& a, const KVec3<T>& b, T alpha)
{
	return Vec3(
		KLerp(a.x, b.x, alpha),
		KLerp(a.y, b.y, alpha),
		KLerp(a.z, b.z, alpha));
}

template <typename T>
KVec3<T> KVec3<T>::Reflect(const Vec3& norm) const
{
	return (*this) - 2 * (Dot(norm)) * norm;
}

template <typename T>
KVec3<T> KVec3<T>::RandomDir()
{
	Vec3 out;
	out.x = RandFloat(-1, 1);
	out.y = RandFloat(-1, 1);
	out.z = RandFloat(-1, 1);
	return out.GetNormalized();
}


template <typename T>
bool KVec3<T>::ContainsNaN() const
{
	return !(x == x && y == y && z == z);
}

template struct KVec3<f32>;
template struct KVec3<f64>;
//template struct KVec3<x64>;