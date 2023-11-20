#pragma once

#include "engine/global/types_numeric.h"
#include "glm.h"

template <typename T>
struct KVec2
{
	using Vec2 = KVec2<T>;

	T x, y;

	constexpr KVec2() : x(T(0)), y(T(0)) {}
	constexpr KVec2(T inX, T inY) : x(inX), y(inY) {}
	constexpr KVec2(T all) : x(all), y(all) {}

	class KString ToString() const;

	T Length() const;
	T LengthSq() const;
	T Distance(const Vec2& point) const;
	T DistanceSq(const Vec2& point) const;
	bool Equals(const Vec2& other, T epsilon = Epsilon<T>()) const;

	T operator|(const Vec2& other) const;
	T Dot(const Vec2& a);
	Vec2 GetNormalized() const;
	void Normalize();
	static Vec2 Interpolate(const Vec2& start, const Vec2& target, const T& alpha);

	static bool OrientedCCW(const Vec2& a, const Vec2& b, const Vec2& c);

	Vec2 Adjust(T amountX, T amountY);
	Vec2 AdjustX(T amount);
	Vec2 AdjustY(T amount);
	Vec2 SetX(T value);
	Vec2 SetY(T value);
	Vec2 ScaleX(T scale);
	Vec2 ScaleY(T scale);

	Vec2 operator + (const Vec2& other) const;
	Vec2 operator + (const T& other) const;
	Vec2 operator += (const Vec2& other);
	Vec2 operator += (const T& other);
	Vec2 operator - (const Vec2& other) const;
	Vec2 operator - (const T& other) const;
	Vec2 operator -= (const Vec2& other);
	Vec2 operator -= (const T& other);
	Vec2 operator * (const Vec2& other) const;
	Vec2 operator * (const T& other) const;
	Vec2 operator *= (const Vec2& other);
	Vec2 operator *= (const T& other);
	Vec2 operator / (const Vec2& other) const;
	Vec2 operator / (const T& other) const;
	Vec2 operator /= (const Vec2& other);
	Vec2 operator /= (const T& other);
	bool operator == (const Vec2& other) const;
	bool operator != (const Vec2& other) const;
	bool operator < (const Vec2& other) const;
	bool operator < (const T& other) const;
	bool operator <= (const Vec2& other) const;
	bool operator <= (const T& other) const;
	bool operator > (const Vec2& other) const;
	bool operator > (const T& other) const;
	bool operator >= (const Vec2& other) const;
	bool operator >= (const T& other) const;

	glm::vec<2, T> ToGLM() const { return glm::vec<2, T>(x, y); }
	static KVec2<T> FromGLM(const glm::vec2& v) { return KVec2<T>(v.x, v.y); }
};

typedef KVec2<f32> FVec2;
typedef KVec2<f64> DVec2;
typedef KVec2<GFlt> GVec2;