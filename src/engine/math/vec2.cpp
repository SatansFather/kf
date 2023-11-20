#include "vec2.h"
#include "engine/utility/kstring.h"
#include "math.h"

template <typename T>
KString KVec2<T>::ToString() const
{
	return "(" + KString(x) + ", " + KString(y) + ")";
}

template <typename T>
KVec2<T> KVec2<T>::Interpolate(const Vec2& start, const Vec2& target, const T& alpha)
{
	return start + ((target - start) * alpha);
}

template <typename T>
void KVec2<T>::Normalize()
{
	T len = Length();
	if (len == T(0))
	{
		x = T(0);
		y = T(0);
	}
	else
	{
		x /= len;
		y /= len;
	}
}

template <typename T>
KVec2<T> KVec2<T>::GetNormalized() const
{
	Vec2 out = Vec2(x, y);
	out.Normalize();
	return out;
}

template <typename T>
T KVec2<T>::Dot(const Vec2& a)
{
	return *this | a;
}

template <typename T>
T KVec2<T>::operator|(const Vec2& other) const
{
	return x * other.x + y * other.y;
}

template <typename T>
T KVec2<T>::Length() const
{
	return sqrt(x * x + y * y);
}

template <typename T>
T KVec2<T>::LengthSq() const
{
	return x * x + y * y;
}


template <typename T>
bool KVec2<T>::operator>=(const T& other) const
{
	return LengthSq() >= (other * other);
}

template <typename T>
bool KVec2<T>::operator>=(const Vec2& other) const
{
	return LengthSq() >= other.LengthSq();
}

template <typename T>
bool KVec2<T>::operator>(const T& other) const
{
	return LengthSq() > (other * other);
}

template <typename T>
bool KVec2<T>::operator>(const Vec2& other) const
{
	return LengthSq() > other.LengthSq();
}

template <typename T>
bool KVec2<T>::operator<=(const T& other) const
{
	return LengthSq() <= (other * other);
}

template <typename T>
bool KVec2<T>::operator<=(const Vec2& other) const
{
	return LengthSq() <= other.LengthSq();
}

template <typename T>
bool KVec2<T>::operator<(const T& other) const
{
	return LengthSq() < (other * other);
}

template <typename T>
bool KVec2<T>::operator<(const Vec2& other) const
{
	return LengthSq() < other.LengthSq();
}

template <typename T>
bool KVec2<T>::operator!=(const Vec2& other) const
{
	return x != other.x || y != other.y;
}

template <typename T>
bool KVec2<T>::operator==(const Vec2& other) const
{
	return x == other.x && y == other.y;
}

template <typename T>
KVec2<T> KVec2<T>::operator/=(const T& other)
{
	x /= other;
	y /= other;
	return *this;
}

template <typename T>
KVec2<T> KVec2<T>::operator/=(const Vec2& other)
{
	x /= other.x;
	y /= other.y;
	return *this;
}

template <typename T>
KVec2<T> KVec2<T>::operator/(const T& other) const
{
	return Vec2(x / other, y / other);
}

template <typename T>
KVec2<T> KVec2<T>::operator/(const Vec2& other) const
{
	return Vec2(x / other.x, y / other.y);
}

template <typename T>
KVec2<T> KVec2<T>::operator*=(const T& other)
{
	x *= other;
	y *= other;
	return *this;
}

template <typename T>
KVec2<T> KVec2<T>::operator*=(const Vec2& other)
{
	x *= other.x;
	y *= other.y;
	return *this;
}

template <typename T>
KVec2<T> KVec2<T>::operator*(const T& other) const
{
	return Vec2(x * other, y * other);
}

template <typename T>
KVec2<T> KVec2<T>::operator*(const Vec2& other) const
{
	return Vec2(x * other.x, y * other.y);
}

template <typename T>
KVec2<T> KVec2<T>::operator-=(const T& other)
{
	x -= other;
	y -= other;
	return *this;
}

template <typename T>
KVec2<T> KVec2<T>::operator-=(const Vec2& other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

template <typename T>
KVec2<T> KVec2<T>::operator-(const T& other) const
{
	return Vec2(x - other, y - other);
}

template <typename T>
KVec2<T> KVec2<T>::operator-(const Vec2& other) const
{
	return Vec2(x - other.x, y - other.y);
}

template <typename T>
KVec2<T> KVec2<T>::operator+=(const T& other)
{
	x += other;
	y += other;
	return *this;
}

template <typename T>
KVec2<T> KVec2<T>::operator+=(const Vec2& other)
{
	x += other.x;
	y += other.y;
	return *this;
}

template <typename T>
KVec2<T> KVec2<T>::operator+(const T& other) const
{
	return Vec2(x + other, y + other);
}

template <typename T>
KVec2<T> KVec2<T>::operator+(const Vec2& other) const
{
	return Vec2(x + other.x, y + other.y);
}

template <typename T>
bool KVec2<T>::Equals(const Vec2& other, T epsilon) const
{
	return
		(abs(x - other.x) < epsilon) &&
		(abs(y - other.y) < epsilon);
}

template <typename T>
T KVec2<T>::DistanceSq(const Vec2& point) const
{
	T dx = x - point.x;
	T dy = y - point.y;
	return dx * dx + dy * dy;
}

template <typename T>
T KVec2<T>::Distance(const Vec2& point) const
{
	return sqrt(DistanceSq(point));
}

template <typename T>
KVec2<T> KVec2<T>::ScaleY(T scale)
{
	return Vec2(x, y * scale);
}

template <typename T>
KVec2<T> KVec2<T>::ScaleX(T scale)
{
	return Vec2(x * scale, y);
}

template <typename T>
KVec2<T> KVec2<T>::SetY(T value)
{
	return Vec2(x, value);
}

template <typename T>
KVec2<T> KVec2<T>::SetX(T value)
{
	return Vec2(value, y);
}

template <typename T>
KVec2<T> KVec2<T>::AdjustY(T amount)
{
	return Vec2(x, y + amount);
}

template <typename T>
KVec2<T> KVec2<T>::AdjustX(T amount)
{
	return Vec2(x + amount, y);
}

template <typename T>
KVec2<T> KVec2<T>::Adjust(T amountX, T amountY)
{
	return Vec2(x + amountX, y + amountY);
}

template <typename T>
bool KVec2<T>::OrientedCCW(const Vec2& a, const Vec2& b, const Vec2& c)
{
	return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x) >= 0.f;
}


template struct KVec2<f32>;
template struct KVec2<f64>;	