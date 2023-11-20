#include "hlslglobal.hlsl"

//StructuredBuffer<KBaseParticle> ParticleBuffer : register(t0);
StructuredBuffer<KSmokeSheet> ParticleBuffer : register(t0);

struct vs_out
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	uint particleIndex = id / 4;
	uint vertexInQuad = id % 4;
	KSmokeSheet smoke = ParticleBuffer[particleIndex];
	const float3 position = smoke.GetPosition();
	vs_out o;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 24;
	pos = mul(pos, (float3x3)View) + position;

	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}