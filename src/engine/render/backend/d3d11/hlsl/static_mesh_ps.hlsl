#include "hlslglobal.hlsl"

Texture2D tex : register(t0);

float4 main(
	float4 Pos : SV_POSITION,
	float2 Texcoord : TEXCOORD,
	float3 Normal : NORMAL,
	float4 PixelPosition : WORLDPOS
) : SV_TARGET
{
	//return float4(PixelPosition.xyz, 1);

	float4 fog = GetFogValue(PixelPosition);
	if (fog.a == 1) return fog;

	float4 color = tex.Sample(SmpUnfiltered, Texcoord);
	if (color.a == 0) color = float4(abs(Normal), 1);
	float light = clamp(dot(Normal, float3(0, 1, 0)), .75, 1);
	float4 underWater = UnderWaterColor;
	underWater = underWater.a == 0 ? 1 : underWater;
	underWater = lerp(1, underWater, (clamp(UnderWaterDepth, 0, 96) / 128));
	return lerp(color * light, float4(fog.xyz, 1), fog.w) * underWater;

	float spec = 0;
	//if (color.r > .36) spec = 1;
	if (int(color.r * 200) % 2 == 0) spec = .6;

	float3 viewDir = CamDirection.xyz;
	float3 lightDir = normalize(float3(0, -1, 0));

	float3 halfDir = normalize(lightDir + viewDir);
	float specAngle = max(dot(halfDir, -Normal), 0.0);
	float specular = pow(specAngle, 8);

	return color * light + specular * spec;
}