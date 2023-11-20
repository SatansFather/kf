#include "hlslglobal.hlsl"

Texture2D input : register(t0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	return input.Sample(SmpFiltered, texcoord*8);
	//return SampleBlurred(input, texcoord*8, 4);
};