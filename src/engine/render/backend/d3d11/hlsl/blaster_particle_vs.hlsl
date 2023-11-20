#include "hlslglobal.hlsl"

StructuredBuffer<KBlasterParticle> ParticleBuffer : register(t0);

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
	uint bufferIndex = particleIndex / BLASTER_PARTICLE_COUNT;
	particleIndex %= BLASTER_PARTICLE_COUNT;

	KBlasterParticle blast = ParticleBuffer[bufferIndex];

	float3 position = blast.GetPosition(RenderAlpha);
	float3 velocity = blast.GetVelocity();
	float time = blast.GetTimeCreated();
	float age = Time - time;

	float3 adjust = 0;

	if (particleIndex != 0)
	{
		adjust.x = sin((age + random(particleIndex) * 4) * (6 + random(particleIndex * time)));
		adjust.y = sin((age + random(particleIndex) * 5) * (6 + random(particleIndex + time)));
		adjust.z = sin((age + random(particleIndex) * 3) * (6 + random(particleIndex - time)));
		adjust = normalize(adjust) * 9 * saturate(age * 6);

		//adjust -= velocity * .01 * saturate(-dot(normalize(adjust), normalize(velocity)));
	}
	position += adjust;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= particleIndex == 0 ? 6 * saturate(age * 6) : .85;
	pos = mul(pos, (float3x3)View) + position;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Color = particleIndex == 0 ? float4(.5, .5, 1, 1) : float4(random(particleIndex), random(particleIndex + time), 1, .7);
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}