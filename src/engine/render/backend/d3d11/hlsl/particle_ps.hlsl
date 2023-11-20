#include "hlslglobal.hlsl"

float4 main(float4 position : SV_POSITION, float4 color : COLOR, float4 data : DATA, float2 texCoord : TEXCOORD) : SV_TARGET
{
	float4 c = color;
	float2 tex = texCoord - .5;
	float lenSq = tex.x * tex.x + tex.y * tex.y;
	c.a *= 1 - (lenSq * 4);
	if (c.a < .1)
	{
		discard;
		return 0;
	}
	return c;
}