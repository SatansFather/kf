#include "hlslglobal.hlsl"

//StructuredBuffer<KBaseParticle> ParticleBuffer : register(t0);
StructuredBuffer<KBulletHole> ParticleBuffer : register(t0);

struct vs_out
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	uint particleIndex = id / 4;
	uint vertexInQuad = id % 4;
	
	KBulletHole bullet = ParticleBuffer[particleIndex];
	const float3 norm = bullet.GetNormal();
	const float3 position = bullet.GetPosition();
	const float3 up = normalize(cross(norm, abs(norm.y) > .99 ? float3(1, 0, 0) : float3(0, 1, 0)));
	const float3 right = normalize(cross(norm, up));
	const float age = Time - bullet.GetTimeCreated();
	const float scale = saturate(abs(10 - age * 2));
	const float rightScale = (vertexInQuad % 2) ? scale : -scale;
	const float upScale = (vertexInQuad & 2) ? -scale : scale;

	float3 pos = position + (up * upScale) + (right * rightScale);

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}