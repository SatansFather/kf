#include "hlslglobal.hlsl"

Texture2D tex : register(t0);

float4 main(float4 positon : SV_POSITION, float2 texCoord : TEXCOORD) : SV_TARGET
{
	float4 c = tex.Sample(SmpUnfiltered, texCoord);
	if (c.a < .1)
	{
		discard;
		return 0;
	}
	return c / 1.3;
}