#include "hlslglobal.hlsl"

StructuredBuffer<KExplosion> ParticleBuffer : register(t0);

#define LIFESPAN 1.8 // real lifespan is 1.0 / LIFESPAN
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
	uint bufferIndex = particleIndex / EXPLODE_PARTICLE_COUNT;

	KExplosion explosion = ParticleBuffer[bufferIndex];
	particleIndex %= EXPLODE_PARTICLE_COUNT;

	// data
	const float timeCreated = explosion.GetTimeCreated();
	const float age = Time - timeCreated;
	const float3 position = explosion.GetPosition();
	const float3 normal = explosion.GetNormal();
	const float radius = explosion.GetRadius();
	const float rand = timeCreated;

	float randA = (random(rand + particleIndex * timeCreated) - .5) * 2;
	float randB = (random(rand + particleIndex + timeCreated) - .5) * 2;
	float randC = (random(particleIndex * rand) - .5) * 2;
	float randPos = random(particleIndex * rand);

	float3 splashPos;
	splashPos.x = randA;
	splashPos.y = randC;
	splashPos.z = randB;
	splashPos = normalize(splashPos);
	float expand = 128;
	float exp = .6;
	if (particleIndex % 3 == 0) // outside flare
	{
		splashPos *= saturate(random(particleIndex * rand) + .5);
		expand = 96;
		exp = .6;
	}
	else if (particleIndex % 3 == 1) // center flare
	{
		splashPos *= saturate(random(particleIndex * rand));
		expand = 192;
		exp = 1.8;
	}
	//splashPos *= 96 * pow((2.5 * age), .75);
	expand *= SCALE;
	splashPos *= expand * pow((LIFESPAN * age), exp);// + (normal * 64 * SCALE * age * LIFESPAN);

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 24 * SCALE;// * saturate(1 - age);
	pos = mul(pos, (float3x3)View) + splashPos + position;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Data = 0;
	o.Color = (particleIndex % 3 == 1) ? float4(30, 15, 1, .11 * (1 - age * (LIFESPAN + randPos))) : float4(5, 1.7, .5, .065 * (1 - age * LIFESPAN));
	o.Color.xyz *= 
		(particleIndex % 3 == 2) 
		? 1 - age * (LIFESPAN * (1.1 + randPos * 1.1))
		: saturate(1 - age * (LIFESPAN * (1.52 + randPos * .8)));

	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}