#include "hlslglobal.hlsl"

//StructuredBuffer<KBaseParticle> ParticleBuffer : register(t0);
StructuredBuffer<KBloodTrail_UnderWater> ParticleBuffer : register(t0);

struct vs_out
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float4 Data : DATA;
	float2 TexCoord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	uint vertexInQuad = id % 4;
	uint particleIndex = id / 4;
	uint bufferIndex = particleIndex / BLOOD_TRAIL_PARTICLE_COUNT;
	particleIndex %= BLOOD_TRAIL_PARTICLE_COUNT;

	KBloodTrail_UnderWater blood = ParticleBuffer[bufferIndex];

	float3 position = blood.GetPosition();
	float time = blood.GetTimeCreated();
	float age = Time - time;

	float3 adjust;
	adjust.x = (random(position.x + time + particleIndex) - .5) * 7; 
	adjust.y = (random(position.y + time + particleIndex) - .5) * 7; 
	adjust.z = (random(position.z + time + particleIndex) - .5) * 7; 

	adjust *= 1 + age * 14;

	//adjust.y -= 32 * age * age;

	position += adjust;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	
	float x = age * 1.f / .7;
	float y = sin(x * 3.151692);

	pos.xy *= y * 1.5;
	pos = mul(pos, (float3x3)View) + position;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Color = float4(.9, 0, 0, 1);
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}