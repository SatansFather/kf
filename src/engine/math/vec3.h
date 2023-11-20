#pragma once

#include "kfglobal.h"
#include "glm.h"
#include "math.h"

template <typename T>
struct /*alignas(16)*/ KVec3
{
	using Vec3 = KVec3<T>;
	
	T x, y, z;

	constexpr KVec3() : x(T(0)), y(T(0)), z(T(0)) {}
	constexpr KVec3(T inX, T inY, T inZ) : x(inX), y(inY), z(inZ) {}
	constexpr KVec3(T all) : x(all), y(all), z(all) {}

	class KString ToString() const;

	T Length() const;
	T LengthSq() const;
	T Distance(const Vec3& point) const;
	T DistanceSq(const Vec3& point) const;
	Vec3 ProjectToNorm(const Vec3& direction) const;
	Vec3 Perpendicular(const Vec3& hint = Vec3(0, 1, 0), const Vec3 hint2 = Vec3(0, 0, 1)) const;
	bool Equals(const Vec3& other, T epsilon = Epsilon<T>()) const;
	T AngleBetween(const Vec3& other) const;
	Vec3 Reflect(const Vec3& norm) const;
	Vec3 Abs() const;
	
	T operator|(const Vec3& other) const;
	T Dot(const Vec3& a) const;
	Vec3 operator^(const Vec3& other) const;
	Vec3 Cross(const Vec3& a) const;
	Vec3 GetNormalized() const;
	T Normalize();
	Vec3 GetRotated(T angle, Vec3 axis) const;
	void Rotate(T angle, Vec3 axis);
	Vec3 GetRotatedDegrees(T angle, Vec3 axis) const;
	void RotateDegrees(T angle, Vec3 axis);
	static Vec3 Interpolate(const Vec3& start, const Vec3& target, const T& alpha);

	T MinComponent() const { return KMin(x, KMin(y, z)); }
	T MaxComponent() const { return KMax(x, KMin(y, z)); }

	Vec3 GetClamped(Vec3 min, Vec3 max) const;
	void Clamp(Vec3 min, Vec3 max);

	Vec3 GetClampedToMaxLength(GFlt len) const;

	bool ContainsNaN() const;

	static Vec3 RandomDir();

	static T DifferenceBetweenColors(const Vec3& a, const Vec3& b)
	{
		return sqrt
		(
			pow((b.x - a.x), 2) +
			pow((b.y - a.y), 2) +
			pow((b.z - a.z), 2)
		);
	}

	Vec3 Adjust(T amountX, T amountY, T amountZ) const;
	Vec3 AdjustX(T amount) const;
	Vec3 AdjustY(T amount)const;
	Vec3 AdjustZ(T amount)const;
	Vec3 SetX(T value)const;
	Vec3 SetY(T value)const;
	Vec3 SetZ(T value)const;
	Vec3 ScaleX(T scale)const;
	Vec3 ScaleY(T scale)const;
	Vec3 ScaleZ(T scale)const;
	Vec3 XY() const;

	Vec3 operator + (const Vec3& other) const;
	Vec3 operator + (const T& other) const;
	Vec3 operator += (const Vec3& other);
	Vec3 operator += (const T& other);
	Vec3 operator - (const Vec3& other) const;
	Vec3 operator - (const T& other) const;
	Vec3 operator -= (const Vec3& other);
	Vec3 operator -= (const T& other);
	Vec3 operator * (const Vec3& other) const;
	Vec3 operator * (const T& other) const;
	Vec3 operator *= (const Vec3& other);
	Vec3 operator *= (const T& other);
	Vec3 operator / (const Vec3& other) const;
	Vec3 operator / (const T& other) const;
	Vec3 operator /= (const Vec3& other);
	Vec3 operator /= (const T& other);
	bool operator == (const Vec3& other) const;
	bool operator != (const Vec3& other) const;
	bool operator < (const Vec3& other) const;
	bool operator < (const T& other) const;
	bool operator <= (const Vec3& other) const;
	bool operator <= (const T& other) const;
	bool operator > (const Vec3& other) const;
	bool operator > (const T& other) const;
	bool operator >= (const Vec3& other) const;
	bool operator >= (const T& other) const;
	Vec3 operator - () const;
	const T At(i32 index) const
	{
		return ptr()[index];
	}

	static Vec3 Lerp(const Vec3& a, const Vec3& b, T alpha);
	static KVec3<T> FromPitchYaw(T pitch, T yaw);
	void ToPitchYaw(T& pitch, T& yaw) const;
	void ToPitchYawNormalized(T& pitch, T& yaw) const; // accepts normalized vector only

	T& At(i32 index)
	{
		return ptr()[index];
	}

	T* ptr() { return &x; }
	const T* ptr() const { return &x; }
	T& operator [](i32 index) { return At(index); }
	const T operator [](i32 index) const { return At(index); }

	glm::vec3 ToGLM() const { return glm::vec3(f32(y), f32(z), f32(x)); }
	glm::vec4 ToGLM4() const { return glm::vec4(f32(y), f32(z), f32(x), 1.f); }
	static KVec3<T> FromGLM(const glm::vec3& v) { return KVec3<T>(v.z , v.x , v.y); }

	template <typename Type>
	KVec3<Type> ToType() const { return KVec3<Type>(x, y, z); }
};

// global operator, dont move this to cpp file
template <typename T>
KVec3<T> operator*(const T scale, const KVec3<T>& vec)
{
	return vec * scale;
}

typedef KVec3<f32> FVec3;
typedef KVec3<f64> DVec3;
typedef KVec3<GFlt> GVec3;
//typedef KVec3<x64> XVec3;
