#include "hlslglobal.hlsl"

float4 main(float4 positon : SV_POSITION, float4 normal : NORMAL) : SV_TARGET
{
	float u = RoundNearest(normal.x, 1.f / 16.f);
	float v = RoundNearest(normal.y, 1.f / 16.f);
	return (random(float2(u, v)) / 4 + .3);
}