#include "line.h"

template <typename T>
KVec3<T> KLine<T>::GetPoint(T distance) const
{
	return Position + (Direction * distance);
}

template struct KLine<f32>;
template struct KLine<f64>;
//template struct KLine<x64>;