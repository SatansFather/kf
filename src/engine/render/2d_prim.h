#pragma once

#if !_SERVER

#include "kfglobal.h"

// these types can be cast directly to d2d or sdl types

template <typename T>
struct KHudRect
{
	T left;
	T top;
	T right;
	T bottom;
};
typedef KHudRect<f32> KHudRectF;
typedef KHudRect<u32> KHudRectU;
typedef KHudRect<i32> KHudRectI;

template <typename T>
struct KHudPoint
{
	T x; T y;
};
typedef KHudPoint<f32> KHudPointF;
typedef KHudPoint<u32> KHudPointU;
typedef KHudPoint<i32> KHudPointI;

#endif