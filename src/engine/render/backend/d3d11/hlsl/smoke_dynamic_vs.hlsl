#include "hlslglobal.hlsl"

StructuredBuffer<KDynamicSmokeParticle> ParticleBuffer : register(t0);

struct vs_out
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	//float4 Data : DATA;
	float TimeOffset : TIMEOFFSET;
	float2 TexCoord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	uint particleIndex = id / 4;
	uint vertexInQuad = id % 4;

	float alpha = saturate(RenderAlpha);// / ParticleBuffer[particleIndex].CurrentPos.w;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= ParticleBuffer[particleIndex].GetScale(alpha);// * (saturate(RenderAlpha) > ParticleBuffer[particleIndex].CurrentPos.w ? 0 : 1);
	pos = mul(pos, (float3x3)View) +
		lerp(ParticleBuffer[particleIndex].PrevPos.xyz,
			ParticleBuffer[particleIndex].CurrentPos.xyz,
			alpha);

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	//o.Data = ParticleBuffer[particleIndex].Data;
	o.TimeOffset = ParticleBuffer[particleIndex].GetTimeOffset();
	o.Color = ParticleBuffer[particleIndex].GetColor();
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}