#include "hlslglobal.hlsl"

#define GBUFFER 0

Texture2D tex : register(t0);
Texture2D lightdepth : register(t1);
Texture2D lightmap : register(t2);

StructuredBuffer<KDynamicLight> DynamicLights : register(t64);

#if GBUFFER
struct PsOut
{
	float4 color : SV_Target0;
	float4 position : SV_Target1;
	float4 normal : SV_Target2;
};

PsOut 
#else
float4 
#endif
main(
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
#if GBUFFER
	PsOut o;
	o.color = 0;
	o.position = 0;
	o.normal = 0;
#endif

	float4 fog = GetFogValue(fragpos);
	if (fog.a == 1) 
	{
#if GBUFFER
		o.color = fog;
		return o;
#else
		return fog;
#endif
	}


	float2 coord = GetAtlasCoords(texcoord, float2(minmaxcoords.x, minmaxcoords.y), float2(minmaxcoords.z, minmaxcoords.w));
	//float4 color = tex.SampleLevel(SmpUnfiltered, coord, 4);
	float4 color = tex.Sample(SmpUnfiltered, coord);

	if (color.a == 0) 
	{
		discard;
#if GBUFFER
		return o;
#else
		return 0;
#endif
	}

	float4 light = (lightmap.Sample(SmpFiltered, lightcoord) + float4(lightcolor, 1));

#if !GBUFFER
	for (uint i = 0; i < ActiveFlashCount; i++)
	{
		KLightFlash flash = ActiveFlashes[i];
		float4 flashColor = float4(CalculateLightFlash(flash), 1);
		float radius = flash.GetRadius();
		float3 flashPos = flash.GetPosition(RenderAlpha);
		if (distancesq(flashPos, fragpos.xyz) > pow(radius, 2))
			continue;

		light += GetLightAtPixel(fragpos, norm, flashColor, flashPos, radius, flash.GetFalloff(), false, false);
	}

	for (uint i = 0; i < ActiveDynamicLightCount; i++)
	{
		KDynamicLight dynLight = DynamicLights[i];
		float4 lightColor = dynLight.GetColor();
		float radius = dynLight.GetRadius(RenderAlpha);
		float3 lightPos = dynLight.GetPosition(RenderAlpha);
		if (distancesq(lightPos, fragpos.xyz) > pow(radius, 2))
			continue;

		light += GetLightAtPixel(fragpos, norm, lightColor, lightPos, radius, dynLight.GetFalloff(), dynLight.IsNegative(), dynLight.IsFullFill());
	}
	color *= light;
#endif

	//return (max(light.r, max(light.g, light.b))) >= 1 ? (max(light.r, max(light.g, light.b))) : 0;
	color = lerp(color, float4(fog.xyz, 1), fog.w);

#if GBUFFER
	o.color = color;
	o.position = fragpos;
	o.normal = float4(norm, 1);
	o.color.w = light.x;
	o.position.w = light.y;
	o.normal.w = light.z;
	
	return o;
#else
	return color;
#endif
}
