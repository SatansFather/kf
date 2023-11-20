#include "hlslglobal.hlsl"

RWTexture2D<float4> Output : register(u0);
Texture2D<float4> Color : register(t0);
Texture2D<float4> Light : register(t1);

float4 main(float4 position : SV_POSITION, float2 coord : TEXCOORD) : SV_TARGET
{
	float4 color = Color.Sample(SmpUnfiltered, coord);
	float4 light = Light.Sample(SmpFiltered, coord);
	
	return color * light;
}