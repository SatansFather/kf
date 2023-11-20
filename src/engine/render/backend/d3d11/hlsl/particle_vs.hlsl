#if 0
#include "hlslglobal.hlsl"

//StructuredBuffer<KBaseParticle> ParticleBuffer : register(t0);
StructuredBuffer<KProjectileRender> ParticleBuffer : register(t0);

struct vs_out
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float4 Data : DATA;
	float2 TexCoord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	uint particleIndex = id / 4;
	uint vertexInQuad = id % 4;


#if 1
	
	float alpha = saturate(RenderAlpha) / ParticleBuffer[particleIndex].CurrentPos.w;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 4 * (saturate(RenderAlpha) > ParticleBuffer[particleIndex].CurrentPos.w ? 0 : 1);
	pos = mul(pos, (float3x3)View) + 
		lerp(ParticleBuffer[particleIndex].PrevPos.xyz,
			 ParticleBuffer[particleIndex].CurrentPos.xyz,
			 alpha);
#else
	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= ParticleBuffer[particleIndex].Position.w;
	pos = mul(pos, (float3x3)View) + ParticleBuffer[particleIndex].Position.xyz;

	/*float time = Time - ParticleBuffer[particleIndex].GetTimeCreated();
	float rand = random(particleIndex) * 3;//ParticleBuffer[particleIndex].GetTimeOffset();
	time += rand;
	rand = map(rand, 0, 3, .5, 1.5);
	time *= rand;
	time = time - int(time / 2) * 2;
	pos.y += map(time, 0, 2, 280, 0);
	pos.x += (sin(time * rand) * 16);
	pos.z += (cos(time * rand) * 16);*/
	
	//pos.x += CamPosition.x - 500;
	//pos.z += CamPosition.z - 500;
#endif
	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	//o.Data = ParticleBuffer[particleIndex].Data;
	//o.Color = ParticleBuffer[particleIndex].Color;
	o.Data = 0;
	o.Color = float4(1, 1, 0, 1);
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}
#endif