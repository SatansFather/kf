#include "hlslglobal.hlsl"

struct vs_out
{
	float4 position : SV_POSITION;
	float2 coord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	vs_out o;
	o.position = float4(0, 0, 0, 0);
	return o;
}