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

	float4 color = 1;
	float4 underWater = UnderWaterColor;
	underWater = underWater.a == 0 ? 1 : underWater;
	underWater = lerp(1, underWater, (clamp(UnderWaterDepth, 0, 96) / 128));
	return lerp(color, float4(fog.xyz, 1), fog.w) * underWater;
}