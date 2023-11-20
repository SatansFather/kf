#include "hlslglobal.hlsl"

RWTexture2D<float4> Output : register(u0);
Texture2D<float4> Color : register(t0);
Texture2D<float4> Position : register(t1);
Texture2D<float4> Normal : register(t2);

StructuredBuffer<KDynamicLight> DynamicLights : register(t64);

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float2 coord = float2(dispatchThreadID.xy) / Resolution * 2;

	float4 color = Color.SampleLevel(SmpUnfiltered, coord, 0);
	float4 pos = Position.SampleLevel(SmpUnfiltered, coord, 0);
	float4 normal = Normal.SampleLevel(SmpUnfiltered, coord, 0);
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

	Output[dispatchThreadID.xy] = light;
}