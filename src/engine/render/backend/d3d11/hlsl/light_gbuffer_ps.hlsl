#include "hlslglobal.hlsl"

Texture2D<float4> Color : register(t0);
Texture2D<float4> Position : register(t1);
Texture2D<float4> Normal : register(t2);

StructuredBuffer<KDynamicLight> DynamicLights : register(t64);

float4 main(float4 position : SV_POSITION, float2 texCoord : TEXCOORD) : SV_TARGET
{
	float2 coord = texCoord;

	float4 color = Color.Sample(SmpUnfiltered, coord);
	float4 pos = Position.Sample(SmpUnfiltered, coord);
	float4 normal = Normal.Sample(SmpUnfiltered, coord);
	float3 norm = normal.xyz;

	float4 light = 0;

	for (uint i = 0; i < ActiveFlashCount; i++)
	{
		KLightFlash flash = ActiveFlashes[i];
		float4 flashColor = float4(CalculateLightFlash(flash), 1);
		float radius = flash.GetRadius();
		float3 flashPos = flash.GetPosition(RenderAlpha);
		if (distancesq(flashPos, pos.xyz) > pow(radius, 2))
			continue;

		light += GetLightAtPixel(pos, norm, flashColor, flashPos, radius, flash.GetFalloff(), false, false);
	}

	for (uint i = 0; i < ActiveDynamicLightCount; i++)
	{
		KDynamicLight dynLight = DynamicLights[i];
		float4 lightColor = dynLight.GetColor();
		float radius = dynLight.GetRadius(RenderAlpha);
		float3 lightPos = dynLight.GetPosition(RenderAlpha);
		if (distancesq(lightPos, pos.xyz) > pow(radius, 2))
			continue;

		light += GetLightAtPixel(pos, norm, lightColor, lightPos, radius, dynLight.GetFalloff(), dynLight.IsNegative(), dynLight.IsFullFill());
	}

	light.x += color.w;
	light.y += pos.w;
	light.z += normal.w;

	return light;
}