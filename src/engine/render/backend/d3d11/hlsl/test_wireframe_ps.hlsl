#include "hlslglobal.hlsl"

float4 main(
	float4 position : SV_POSITION,
	float4 fragpos : FRAGPOS,
	float2 texcoord : TEXCOORD,
	float3 norm : NORMAL,
	float2 lightcoord : LIGHTCOORD,
	float4 lightfragpos : LIGHTSPACE,
	float4 minmaxcoords : COORDCLAMP,
	float3 lightcolor : LIGHTCOLOR
) : SV_TARGET
{
	float dist = distance(fragpos, CamPosition);
	dist *= dist;
	dist = map(dist, 0, 384 * 384, 1, .25);
	return clamp(dist, .25, 1);
	return 1;
}