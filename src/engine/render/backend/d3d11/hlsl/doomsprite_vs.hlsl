#include "hlslglobal.hlsl"

//StructuredBuffer<KBaseParticle> ParticleBuffer : register(t0);
StructuredBuffer<KJohnnyJiantdick> ParticleBuffer : register(t0);

struct vs_out
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	uint particleIndex = id / 4;
	uint vertexInQuad = id % 4;
	
	KJohnnyJiantdick john = ParticleBuffer[particleIndex];

	float alpha = saturate(RenderAlpha) / ParticleBuffer[particleIndex].CurrentPos.w;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.y += .22;
	pos.z = 0;
	float deathScale = saturate(RenderAlpha) > john.CurrentPos.w ? 0 : 1;
	pos.x *= 12 * deathScale;
	pos.y *= 28 * deathScale;
	float3 dir = CamDirection.xyz;
	pos = mul(pos, (float3x3)SpriteView) + john.GetPosition(RenderAlpha);
	
	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}