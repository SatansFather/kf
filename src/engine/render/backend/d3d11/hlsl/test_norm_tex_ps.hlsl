#include "hlslglobal.hlsl"

Texture2D tex : register(t0);

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
	float2 coord = GetAtlasCoords(texcoord, float2(minmaxcoords.x, minmaxcoords.y), float2(minmaxcoords.z, minmaxcoords.w));
	float4 color = tex.Sample(SmpUnfiltered, coord);
	float c = color.a - 1;
	if (c < 0) clip(c);
	return color * float4(abs(norm), 1);
}