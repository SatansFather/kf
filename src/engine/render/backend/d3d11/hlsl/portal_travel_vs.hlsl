#include "hlslglobal.hlsl"

StructuredBuffer<KPortalTravel> ParticleBuffer : register(t0);

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
	uint bufferIndex = particleIndex / PORTAL_TRAVEL_COUNT;

	KPortalTravel particle = ParticleBuffer[bufferIndex];
	particleIndex %= PORTAL_TRAVEL_COUNT;

	// data
	const float timeCreated = particle.GetTimeCreated();
	const float age = Time - timeCreated;
	const float3 position = particle.GetPosition();
	const float3 extent = particle.GetHalfExtent();
	const bool isEntry = particle.IsEntry();

	const float maxExtent = max(extent.x, max(extent.y, extent.z));
	float lifeAlpha = pow(age / 3, .5);
	if (isEntry) lifeAlpha *= 2;
	lifeAlpha = saturate(lifeAlpha);

	float3 particlePos = position;

	if (isEntry)
	{	
		//float3 dir = normalize(
		//	float3(random(timeCreated + (float)particleIndex * 1.3) - .5, 
		//	random(timeCreated + (float)particleIndex * 1.6) - .5, 
		//	random(timeCreated + (float)particleIndex * 2.6) - .5));
		//	dir *= normalize(extent) * 2;
		//const float scale = random(particleIndex) * 1.2;
		//particlePos = position + lerp(dir, dir * maxExtent * sqrt(scale), lifeAlpha);

		const float angle = float(particleIndex) / 3;
		const float2 ringPos = float2(sin(angle), cos(angle));// * 3.1415926;
		const float ringHeight = map(particleIndex, 0, PORTAL_TRAVEL_COUNT, -extent.y, extent.y);
		particlePos = position + float3(ringPos.x, 0, ringPos.y) * extent.x;
		particlePos.y += ringHeight;

		float3 adjust = normalize(
			float3(random(timeCreated + (float)particleIndex * 1.3) - .5,
				random(timeCreated + (float)particleIndex * 1.6) - .5,
				random(timeCreated + (float)particleIndex * 2.6) - .5));

		particlePos += adjust * age * 6;
		particlePos = lerp(position, particlePos, sqrt(saturate(age * 12)));//pow(saturate(age), .3));
	}
	else
	{
		const float angle = float(particleIndex) / 12;
		const float2 ringPos = float2(sin(angle), cos(angle));// * 3.1415926;
		const float ringHeight = map(particleIndex, 0, PORTAL_TRAVEL_COUNT, -extent.y, extent.y);
		particlePos = position + float3(ringPos.x, 0, ringPos.y) * extent.x;
		particlePos.y += ringHeight;

		float3 adjust = normalize(
			float3(random(timeCreated + (float)particleIndex * 1.3) - .5,
				random(timeCreated + (float)particleIndex * 1.6) - .5,
				random(timeCreated + (float)particleIndex * 2.6) - .5));

		particlePos += adjust * age * 6; 	
		particlePos = lerp(position, particlePos, sqrt(saturate(age * 7)));//pow(saturate(age), .3));
	}

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 1 - lifeAlpha;
	pos = mul(pos, (float3x3)View) + particlePos;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Data = 0;
	o.Color = 
		pow(float4(random(timeCreated + (float)particleIndex * 1.8),
		random(timeCreated + (float)particleIndex * 3.2),
		random(timeCreated + (float)particleIndex * 5.1),
		1 - lifeAlpha * lifeAlpha), 2);

	if (isEntry)
	{
		const float particleAlpha = float(particleIndex) / float(PORTAL_TRAVEL_COUNT);
		o.Color.a = lerp(o.Color.a, pow(1 - lifeAlpha, 2) * o.Color.a, particleAlpha);
	}
	else
	{
		const float particleAlpha = float(PORTAL_TRAVEL_COUNT - particleIndex) / float(PORTAL_TRAVEL_COUNT);
		o.Color.a = lerp(saturate(age * age * 15) * o.Color.a, o.Color.a, pow(particleAlpha, 6));
	}

	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}