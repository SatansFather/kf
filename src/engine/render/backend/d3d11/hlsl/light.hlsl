#include "hlslglobal.hlsl"
#include "../../../bufferslots.h"

/*
#define CB_MVP		0	// modeltransform * view matrix * projection matrix
#define CB_MODEL	1	// just the "M" in "MVP"
#define CB_LIGHTMAT 2	// view matrix for shadowmap/lightdepth generation
#define CB_FRAME	4	// passage of time

// currently bound light
// TODO use arrays?
#define CB_SPOTLIGHT	10
#define CB_POINTLIGHT	11
#define CB_PARALIGHT	12

#define SMP_UNFILTERED	0
#define SMP_FILTERED	1
#define SMP_CONFIG		2	// if theres an option to enable/disable texture filtering
#define SMP_SHADOWMAP	3	// samples a light's depth buffer for depth comparison

#define CB_REGISTER(x) register(b##x)
#define SMP_REGISTER(x) register(s##x)*/

struct FSpotlight
{
	float4 Position;
	float4 Direction;
	float4 Color;
	float4 Ambient;
	float4x4 ViewProjection;

	float3 GetPosition() { return Position.xyz; }
	float3 GetDirection() { return Direction.xyz; }
	float3 GetColor() { return Color.xyz; }
	float3 GetAmbient() { return Ambient.xyz; }

	float GetOuterAngle() { return Position.w; }
	float GetInnerAngle() { return Direction.w; }
	float GetRange() { return Color.w; }
	float GetFalloff() { return Ambient.w; }
};

cbuffer CB_LightSpot : CB_REGISTER(10)
{
	FSpotlight Spotlights[32];
	uint ActiveSpotlights; // mask
	uint SpotlightCount;
}

float CalculateShadow(float4 lightfragpos, Texture2D map, SamplerState smplr)
{
	float minval = 0;

	float3 frag = lightfragpos.xyz / lightfragpos.w;
	float pixeldepth = frag.z;

	if (pixeldepth > 1) return minval;

	float2 projcoords = frag.xy;
	projcoords = projcoords * .5 + .5;
	projcoords.y = 1 - projcoords.y;

	float closestdepth = map.Sample(smplr, projcoords).r;

	if (closestdepth == 0)
		return minval;
	
	return closestdepth == 0 ? minval : (pixeldepth - .0005) > closestdepth ? minval : 1;
}

float4 CalculateSpotlight(uint index)
{
	return float4(0, 0, 0, 0);
}

float3 CalculateLightFlash(KLightFlash flash)
{
	float3 color = flash.GetColor().xyz;
	float age = Time - flash.GetTimeCreated();
	
	float duration = flash.GetGrowDuration();
	float alpha = age / duration;
	if (age > duration)
	{
		age -= duration;
		duration = flash.GetDimDuration();
		alpha = 1 - age / duration;
	}

	const float pi = 3.141592;

	alpha = saturate(alpha);
	alpha = (sin((alpha * pi) - (pi / 2.f)) / 2.f) + .5;
	color *= alpha;

	return color;
}