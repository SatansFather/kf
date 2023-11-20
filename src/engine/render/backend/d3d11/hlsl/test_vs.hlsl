#include "hlslglobal.hlsl"


StructuredBuffer<KDynamicLight> DynamicLights : register(t64);

struct vs_out
{
	float4 position : SV_POSITION;
	float4 fragpos : FRAGPOS;
	float2 coord : TEXCOORD;
	float3 norm : NORMAL;
	float2 lightcoord : LIGHTCOORD;
	float4 lightfragpos : LIGHTSPACE;
	float4 coordclamp : COORDCLAMP;
	float3 lightcolor : LIGHTCOLOR;
	uint bufferIndex : BUFFERINDEX;
};

vs_out main(
	float4 pos : POSITION, 
	float2 texcoord : TEXCOORD, 
	float3 norm : NORMAL, 
	float2 lightcoord : LIGHTCOORD, 
	float4 minmaxcoords : COORDCLAMP, 
	float3 lightcolor : LIGHTCOLOR,
	uint bufferIndex : BUFFERINDEX)
{
	vs_out o;
	o.position = mul(pos, ModelViewProjection);
	o.lightcoord = lightcoord;
	o.norm = norm;
	o.fragpos = mul(pos, ModelTransform);
	o.lightfragpos = mul(LightSpaceMatrix, o.fragpos);
	o.coord = texcoord;
	o.coordclamp = minmaxcoords;
	o.lightcolor = lightcolor;
	o.bufferIndex = bufferIndex;

	/*for (uint i = 0; i < ActiveFlashCount; i++)
	{
		KLightFlash flash = ActiveFlashes[i];
		float4 flashColor = float4(CalculateLightFlash(flash), 1);
		float radius = flash.GetRadius();
		float3 flashPos = flash.GetPosition(RenderAlpha);
		if (distancesq(flashPos, o.fragpos.xyz) > pow(radius, 2))
			continue;

		o.lightcolor += GetLightAtPixel(o.fragpos, norm, flashColor, flashPos, radius, flash.GetFalloff(), false, false);
	}

	for (uint i = 0; i < ActiveDynamicLightCount; i++)
	{
		KDynamicLight dynLight = DynamicLights[i];
		float4 lightColor = dynLight.GetColor();
		float radius = dynLight.GetRadius(RenderAlpha);
		float3 lightPos = dynLight.GetPosition(RenderAlpha);
		if (distancesq(lightPos, o.fragpos.xyz) > pow(radius, 2))
			continue;

		o.lightcolor += GetLightAtPixel(o.fragpos, norm, lightColor, lightPos, radius, dynLight.GetFalloff(), dynLight.IsNegative(), dynLight.IsFullFill());
	}*/

	// this gives eye space
	// eyepos.z / eyepos.w is depth
	//o.eyepos = mul(mul(Projection, View), o.fragpos);

	return o;
}