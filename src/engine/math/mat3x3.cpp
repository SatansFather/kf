#include "mat3x3.h"
#include "line.h"

template<typename T>
void Swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

template <typename T>
const T KMat3x3<T>::At(i32 row, i32 col) const
{
	return v[row][col];
}

template <typename T>
T& KMat3x3<T>::At(i32 row, i32 col)
{
	return v[row][col];
}

template <typename T>
void KMat3x3<T>::SetRow(i32 row, const T* data)
{
	At(row, 0) = data[0];
	At(row, 1) = data[1];
	At(row, 2) = data[2];
}

template <typename T>
bool KMat3x3<T>::SolveAxb(Vec3 b, Vec3& x) const
{
	T v00 = At(0, 0);
	T v10 = At(1, 0);
	T v20 = At(2, 0);

	T v01 = At(0, 1);
	T v11 = At(1, 1);
	T v21 = At(2, 1);

	T v02 = At(0, 2);
	T v12 = At(1, 2);
	T v22 = At(2, 2);

	T av00 = abs(v00);
	T av10 = abs(v10);
	T av20 = abs(v20);

	if (av10 >= av00 && av10 >= av20)
	{
		Swap(v00, v10);
		Swap(v01, v11);
		Swap(v02, v12);
		Swap(b[0], b[1]);
	}
	else if (av20 >= av00)
	{
		Swap(v00, v20);
		Swap(v01, v21);
		Swap(v02, v22);
		Swap(b[0], b[2]);
	}

	if (EqualAbs(v00, T(0))) return false;

	T denom = T(1) / v00;
	v01 *= denom;
	v02 *= denom;
	b[0] *= denom;

	v11 -= v10 * v01;
	v12 -= v10 * v02;
	b[1] -= v10 * b[0];

	v21 -= v20 * v01;
	v22 -= v20 * v02;
	b[2] -= v20 * b[0];

	if (abs(v21) > abs(v11))
	{
		Swap(v11, v21);
		Swap(v12, v22);
		Swap(b[1], b[2]);
	}

	if (EqualAbs(v11, T(0))) return false;

	denom = T(1) / v11;
	v12 *= denom;
	b[1] *= denom;

	v22 -= v21 * v12;
	b[2] -= v21 * b[1];

	if (EqualAbs(v22, T(0))) return false;

	x[2] = b[2] / v22;
	x[1] = b[1] - x[2] * v12;
	x[0] = b[0] - x[2] * v02 - x[1] * v01;

	return true;
}

template struct KMat3x3<f32>;
template struct KMat3x3<f64>;
//template struct KMat3x3<x64>;