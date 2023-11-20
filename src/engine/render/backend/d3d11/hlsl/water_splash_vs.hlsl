#include "hlslglobal.hlsl"

StructuredBuffer<KWaterSplash> ParticleBuffer : register(t0);

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
	uint bufferIndex = particleIndex / WATER_SPLASH_COUNT;

	KWaterSplash splash = ParticleBuffer[bufferIndex];
	particleIndex %= WATER_SPLASH_COUNT;

	// data
	const float timeCreated = splash.GetTimeCreated();
	const float age = Time - timeCreated;
	const float3 position = splash.GetHitPosition();
	const float3 normal = splash.GetHitNormal();
	const float rand = splash.GetRandomOffset();
	const float strength = splash.GetStrength();
	const float4 color = splash.GetColor();
	const float scale = splash.GetScale();

	// calculate velocity
	const float3 localUp = abs(normal.y) >= .99 ? float3(1, 0, 0) : float3(0, 1, 0);
	const float3 rightDir = normalize(cross(normal, localUp));
	const float3 upDir = normalize(cross(rightDir, normal));

	const float sideSpeed = 32 * scale;
	const float outSpeed = 16 + random(particleIndex * rand) * 16 + (saturate(normal.y) * 12) * strength * (saturate(random(particleIndex * rand)) + .4);
	const float angleInCircle = float(particleIndex) / float(WATER_SPLASH_COUNT) * (2.f * 3.141592f);
	const float2 circlePoint = float2(sideSpeed * cos(angleInCircle), sideSpeed * sin(angleInCircle));

	const float upRand = (random(rand + particleIndex * timeCreated) - .5) * 2;
	const float sideRand = (random(rand + particleIndex + timeCreated) - .5) * 2;
	const float3 initialVel =
		(rightDir * sideRand) * circlePoint.x
		+ (upDir * upRand) * circlePoint.y
		+ normal * outSpeed;

	float3 splashPos =
		position +
		rightDir * 4 * sideRand +
		upDir * 4 * upRand +
		initialVel * age;

	splashPos.y -= 575 * age * age;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 1.5 * (1 - age * 2);
	pos = mul(pos, (float3x3)View) + splashPos;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Data = 0;
	o.Color = color;
	o.Color.a *= 1 - age * 2;
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}