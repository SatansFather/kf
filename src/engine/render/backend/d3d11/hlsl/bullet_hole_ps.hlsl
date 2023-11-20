#include "hlslglobal.hlsl"

Texture2D tex : register(t0);

float4 main(float4 position : SV_POSITION, float2 texCoord : TEXCOORD) : SV_TARGET
{
	float4 color = tex.Sample(SmpUnfiltered, texCoord);
	if (color.a < .8)
	{
		discard;
		return 0;
	}
	return color * 2;

	/*const float sceneDepth = Depth.Sample(SmpUnfiltered, position.xy).r;
	const float particleDepth = position.z;
	if (abs(particleDepth - sceneDepth) > .0001)
	{
		discard;
		return 0;
	}*/

	float4 c = float4(0, 0, 0, 1);//color;
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