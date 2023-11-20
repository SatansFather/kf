
#include "hlslglobal.hlsl"

Texture2D DepthBuffer : register(t0);

float main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_DEPTH
{
	return DepthBuffer.Sample(SmpFiltered, texcoord * 2);
}