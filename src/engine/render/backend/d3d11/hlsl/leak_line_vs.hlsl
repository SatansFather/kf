#include "hlslglobal.hlsl"

struct vs_out
{
	float4 position : SV_POSITION;
};

vs_out main(float4 pos : POSITION)
{
	vs_out o;
	o.position = mul(pos, (ModelViewProjection));
	return o;
}