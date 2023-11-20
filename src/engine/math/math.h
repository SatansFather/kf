#pragma once

#include "engine/global/types_numeric.h"
#include <type_traits>
#include <cmath>
#include <algorithm>
#include <immintrin.h>

#define _PI 3.14159265358979323846

static const GFlt Sqrt2 = sqrt(2);
static const GFlt Sqrt3 = sqrt(3);

template <typename T>
constexpr T PI() { return atan(T(1)) * T(4); }

// constexpr log2
constexpr static u32 cilog2(u32 val) { return val ? 1 + cilog2(val >> 1) : -1; }

f32 MapRange(f32 input, f32 in1, f32 in2, f32 out1, f32 out2);
bool SameSign(f32 a, f32 b);
GFlt LerpFade(GFlt alpha);
f32 RandomSmoothValue(f32 time, f32 scale = 1);

template <typename T>
T Lerp(T a, T b, f64 t)
{
	return a + ((b - a) * t);
}

template <typename T>
constexpr T Epsilon()
{
	if constexpr (std::is_same<T, f32>::value)
	{
		return 1e-3f;	
	}
	else if constexpr (std::is_same<T, f64>::value)
	{
		return 1e-6f;
	}

	return 0;
}

template <typename T>
bool EqualAbs(T a, T b)
{
	return abs(a - b) < Epsilon<T>();
}

static GFlt KClamp(GFlt in, GFlt min, GFlt max)
{
	return std::clamp(in, min, max);
};

static GFlt KLerp(GFlt a, GFlt b, GFlt alpha)
{
	return a + (b - a) * alpha;
}

static GFlt KMin(GFlt a, GFlt b)
{
	return (std::min)(a, b);
}

static GFlt KSaturate(GFlt x)
{
	return KClamp(x, 0, 1);
}

template <typename T>
T KMin(T a, T b)
{
	return (std::min)(a, b);
}

static GFlt KMax(GFlt a, GFlt b)
{
	return (std::max)(a, b);
}

template <typename T>
T KMax(T a, T b)
{
	return (std::max)(a, b);
}

template <typename T>
T RoundNearest(T num, T mult)
{
	num = num + mult / T(2);
	//if constexpr (std::is_integral, T>::value)
	//	num -= num % mult;
	//else
		num -= std::fmod(num, mult);
	return num;
}