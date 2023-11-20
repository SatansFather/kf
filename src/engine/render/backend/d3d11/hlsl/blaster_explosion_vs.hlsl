#include "hlslglobal.hlsl"

StructuredBuffer<KBlastExplosion> ParticleBuffer : register(t0);

#define LIFESPAN 4.6 // real lifespan is 1.0 / LIFESPAN
#define SCALE 1

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
	uint bufferIndex = particleIndex / BLAST_EXPLODE_PARTICLE_COUNT;

	KBlastExplosion explosion = ParticleBuffer[bufferIndex];
	particleIndex %= BLAST_EXPLODE_PARTICLE_COUNT;

	// data
	const float timeCreated = explosion.GetTimeCreated();
	const float age = Time - timeCreated;
	const float3 position = explosion.GetPosition();
	const float rand = timeCreated;

	float randA = (random(rand + particleIndex * timeCreated) - .5) * 2;
	float randB = (random(rand + particleIndex + timeCreated) - .5) * 2;
	float randC = (random(particleIndex * rand) - .5) * 2;
	float randPos = random(particleIndex * rand);

	float3 adjust = 0;
	adjust.x += randA;
	adjust.y += randB;
	adjust.z += randC;
	adjust = 11 * normalize(adjust) * (4 * age * LIFESPAN);
	
	float3 pos;					   
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 4;
	pos = mul(pos, (float3x3)View) + position + adjust;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Data = 0;
	o.Color = float4(random(particleIndex), random(particleIndex + timeCreated), 1, .7 * (1 - age * LIFESPAN));

	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}