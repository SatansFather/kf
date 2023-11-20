#include "hlslglobal.hlsl"

Texture2D tex : register(t0);
Texture2D tex1 : register(t1);

float4 main(
	float4 Pos : SV_POSITION,
	float2 Texcoord : TEXCOORD,
	float3 Normal : NORMAL,
	float4 PixelPosition : WORLDPOS,
	float4 Color : COLOR
) : SV_TARGET
{
	float4 fog = GetFogValue(PixelPosition);
	if (fog.a == 1) return fog;

	float2 coord = Texcoord;
	coord.x += (Time * .5) + sin(Time * .2) * .3;
	coord.y += cos(Time * .2) * .4;
	coord = waterize(coord, 1, 3, 5);

	float4 color = tex1.Sample(SmpUnfiltered, coord);
	color *= Color * 1.5;

	coord = Texcoord;
	coord.x -= Time * .6;

	coord = waterize(coord, 2, 3, 5);

	float4 smoke = tex.Sample(SmpUnfiltered, -coord * 1.5);

	color *= lerp(color, smoke, pow(smoke.r, .3));

	return lerp(color, float4(fog.xyz, 1), fog.w);
}