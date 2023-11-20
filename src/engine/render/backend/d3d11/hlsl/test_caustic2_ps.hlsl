#include "hlslglobal.hlsl"

Texture2D tex : register(t0);
Texture2D lightdepth : register(t1);

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
	float t = int(Time * 2);
	return float4(random(t), random(t + 1), random(t + 2), 1);

	float2 uv = texcoord * 512;

	float3x3 m = float3x3(-2,-1,2, 3,-2,1, 1,2,2);

	float3 a = mul(float3(uv / 4e2, Time / 4), m);
	float3 b = mul(mul(a,m),.4);
	float3 c = mul(mul(b,m),.3);

	float4 k = 
	pow
	(
		min
		(
			min
			(
				length(.5 - frac(a)),
				length(.5 - frac(b))
			), 
			length(.5 - frac(c))
		),
		7
	) 
	* 45;

	float4 light = k * tex.Sample(SmpUnfiltered, waterize(texcoord / 2, 1.5, 1.5, 2));
	return light;
}