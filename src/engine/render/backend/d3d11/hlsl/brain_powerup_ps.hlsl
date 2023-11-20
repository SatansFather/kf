#include "hlslglobal.hlsl"

Texture2D tex : register(t0);

float4 main(
	float4 Pos : SV_POSITION,
	float2 Texcoord : TEXCOORD,
	float3 Normal : NORMAL,
	float4 PixelPosition : WORLDPOS
) : SV_TARGET
{
	
	float4 fog = GetFogValue(PixelPosition);
	if (fog.a == 1) return fog;

	float4 color = tex.Sample(SmpUnfiltered, Texcoord);
	float light = dot(Normal, float3(0, 1, 0));
	light = (light + 3) / 4;
	color *= float4(.5, .3, .7 ,1) * light;
	return lerp(color, float4(fog.xyz, 1), fog.w);
}