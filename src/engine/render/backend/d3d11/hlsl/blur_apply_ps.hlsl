#include "hlslglobal.hlsl"

Texture2D blur : register(t0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	return 0;
};