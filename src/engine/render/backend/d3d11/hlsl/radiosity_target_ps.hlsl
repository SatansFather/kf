#include "hlslglobal.hlsl"

Texture2D tex : register(t0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD, float3 UV : _UV, float2 UVDot : _UVDOT) : SV_TARGET
{
	float3 uv = dot(UVDot, UVDot) * float3(-.5, -.5, -1) + UV;
	return tex.Sample(SmpUnfiltered, uv.xy / uv.z);
}