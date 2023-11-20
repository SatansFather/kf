#include "hlslglobal.hlsl"

Texture2D Input : register(t0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	return Input.Sample(SmpUnfiltered, texcoord);
}