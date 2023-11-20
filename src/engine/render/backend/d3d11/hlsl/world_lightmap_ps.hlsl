#include "hlslglobal.hlsl"

Texture2D tex : register(t0);
Texture2D lightdepth : register(t1);
Texture2D lightmap : register(t2);

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
	return lightmap.Sample(SmpFiltered, lightcoord) + float4(lightcolor, 1);
}