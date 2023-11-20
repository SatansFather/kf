
#include "math.h"
#include <algorithm>

#define PI 3.141592

f32 MapRange(f32 input, f32 in1, f32 in2, f32 out1, f32 out2)
{
	return (input - in1) / (in2 - in1) * (out2 - out1) + out1;
}

bool SameSign(f32 a, f32 b)
{
	if (a == 0 || b == 0)
		return a == 0 && b == 0;

	return a / b > 0;
}

GFlt LerpFade(GFlt alpha)
{
	alpha = KClamp(alpha, 0.f, 1.f);
	//return (6 * pow(alpha, 5)) - (15 * pow(alpha, 4)) + (10 * pow(alpha, 3));
	GFlt x = sin(PI * alpha * .5);
	return x * x;
}

f32 RandomSmoothValue(f32 time, f32 scale)
{
	return (sin(2 * time) + sin(PI * time)) * (scale * .5);
}
