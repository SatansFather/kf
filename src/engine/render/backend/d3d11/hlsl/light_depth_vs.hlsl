#include "hlslglobal.hlsl"

struct vsout
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

vsout main(float4 pos : POSITION, float2 coord : TEXCOORD)
{
	vsout o;
	o.position = mul(pos, ModelViewProjection);
	o.texcoord = coord;
	return o;
}