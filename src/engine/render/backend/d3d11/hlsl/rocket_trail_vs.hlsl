#include "hlslglobal.hlsl"

StructuredBuffer<KRocketTrail> ParticleBuffer : register(t0);

struct VsOut
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD;
};

VsOut main(uint id : SV_VERTEXID)
{
	VsOut o;

	const float particleCount = ROCKET_TRAIL_PARTICLE_COUNT;

	uint vertexInQuad = id % 4;
	uint particleIndex = id / 4;
	uint bufferIndex = particleIndex / particleCount;

	KRocketTrail trail = ParticleBuffer[bufferIndex];
	particleIndex %= ROCKET_TRAIL_PARTICLE_COUNT;

	// data
	const float timeCreated = trail.GetTimeCreated();
	const float timeKilled = trail.GetDeathTime();
	const float age = (timeKilled > 0 ? timeKilled : Time) - timeCreated;
	const float rand = timeCreated;
	const float3 startPosition = trail.GetStartPosition();
	const float3 velocity = trail.GetVelocity();
	const float velLen = length(velocity);
	const float3 velNorm = velocity / velLen;
	const bool isMyRocket = trail.IsMyRocket();
	const bool useRenderOffset = trail.UseRenderOffset();

	// trail length
	const float rocketLength = velLen * (1.f/60.f) * 2;
	const float trailLength = particleCount;
	const float growTime = trailLength / velLen;
	const float visibleCount = particleCount * saturate((age - (rocketLength / velLen)) / growTime);

	const float3 rocketPosition = startPosition + velocity * age;
	const uint distanceTraveled = length(rocketPosition - startPosition);

	// only update emitPosition when distanceTraveled is incremented to prevent jitter
	const float3 emitPosition = startPosition + (velNorm * distanceTraveled) - (velNorm * rocketLength);
	
	// determine world position
	float3 position;
	uint index = (particleIndex + uint(distanceTraveled)) % particleCount;
	position = emitPosition - velNorm * (index * (trailLength / particleCount));

	if (useRenderOffset && (particleIndex + uint(distanceTraveled)) < particleCount * 2)
	{
		// adjust for render offset

		// local down direction
		const float3 worldCross = velNorm.y >= .99 ? float3(1, 0, 0) : float3(0, 1, 0);
		const float3 right = cross(velNorm, worldCross);
		const float3 down = normalize(cross(velNorm, right)) * 14;

		const float offsetAlpha = float(particleIndex) / float(particleCount);
		position += down * offsetAlpha;
 	}

	// get scale based on position in trail
	// if the rocket is dead, advance scale based on time since it exploded to make trail fade into hit position
	float scale = (particleCount - index) / particleCount;
	if (index > visibleCount) scale = 0;
	if (timeKilled > 0) 
	{
		scale -= Time - timeKilled;
		scale = saturate(scale);
	}

	// move around
	float timeSince = 1 - scale;
	position += float3(
		random(particleIndex) - .5, 
		random(particleIndex + 1) - .5, 
		random(particleIndex + 2) - .5) 
			* 5 * (1 + timeSince * 2.5);

	// color
	float opacity = .2;
	if (timeSince < .1) opacity *= timeSince * 10;
	else if (timeSince > .5) opacity *= 1 - (timeSince * 2 - 1);

	if (isMyRocket)
	{
		// players shouldnt have particles in their face from their own rockets
		const float3 camPos = CamPosition.xyz;
		opacity *= saturate(map(distancesq(camPos, position), 0, 4096, 0, 1));
	}

	// color
	float4 color = float4(1, 1, 1, opacity);
	if (random(particleIndex) < .3) color = float4(1, .2, .2, opacity);

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 4;
	pos = mul(pos, (float3x3)View) + position;

	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Color = color * .5;
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;

	//float4 eyepos = mul(mul(Projection, View), mul(float4(pos, 1), ModelTransform));
	//o.Depth = eyepos.z / eyepos.w;

	return o;
}