#include "hlslglobal.hlsl"

Texture2D tex : register(t0);

float4 main(float4 positon : SV_POSITION, float2 coord : TEXCOORD) : SV_TARGET
{
	// texture needs to be sampled so shadows can be cast through mask
	float4 color = tex.Sample(SmpUnfiltered, coord);
	float c = color.a - 1;
	if (c < 0)
	{
		clip(c);
		return float4(color.xyz, 0);
	}

	return float4(0, 0, 0, 0);
}