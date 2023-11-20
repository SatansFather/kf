#include "hlslglobal.hlsl"

struct vs_out
{
	float4 position : SV_POSITION;
	float2 coord : TEXCOORD;
};

//vs_out main(float4 pos : POSITION, float2 texcoord : TEXCOORD)
vs_out main(uint id : SV_VERTEXID)
{
	vs_out o;
	o.position.x = (float)(id / 2) * 4.f - 1.f;
	o.position.y = (float)(id % 2) * 4.f - 1.f;
	o.position.z = 0.f;
	o.position.w = 1.f;

	o.coord.x = (float)(id / 2) * 2.f;
	o.coord.y = 1.f - (float)(id % 2) * 2.f;

	return o;
}