#include "hlslglobal.hlsl"

StructuredBuffer<KHitSpark> ParticleBuffer : register(t0);

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
	uint bufferIndex = particleIndex / HIT_SPARK_COUNT;

	KHitSpark spark = ParticleBuffer[bufferIndex];
	particleIndex %= HIT_SPARK_COUNT;

	// data
	const float timeCreated = spark.GetTimeCreated();
	const float age = Time - timeCreated;
	const float3 position = spark.GetHitPosition();
	const float3 normal = spark.GetHitNormal();

	const float rand = spark.GetRandomOffset();

	// calculate velocity
	const float3 localUp = abs(normal.y) >= .99 ? float3(1, 0, 0) : float3(0, 1, 0);
	const float3 rightDir = normalize(cross(normal, localUp));
	const float3 upDir = normalize(cross(rightDir, normal));

	const float sideSpeed = 32;
	const float outSpeed = 16 + random(particleIndex * rand) * 16 + (saturate(normal.y) * 12);
	const float angleInCircle = float(particleIndex) / float(HIT_SPARK_COUNT) * (2.f * 3.141592f); 
	const float2 circlePoint = float2( sideSpeed * cos(angleInCircle), sideSpeed * sin(angleInCircle) );

	const float upRand = (random(rand + particleIndex * timeCreated) - .5) * 2;
	const float sideRand = (random(rand + particleIndex + timeCreated) - .5) * 2;
	const float3 initialVel = 
		  (rightDir * sideRand) * circlePoint.x 
		+ (upDir * upRand) * circlePoint.y 
		+ normal * outSpeed;

	float3 sparkPos = 
		position + 
		rightDir * 4 * sideRand + 
		upDir * 4 * upRand + 
		initialVel * age;

	sparkPos.y -= 128 * age * age;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= .75 * (1 - age * 2);
	pos = mul(pos, (float3x3)View) + sparkPos;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Data = 0;
	//o.Color = lerp(float4(.95, .74, .2, 1), float4(.3, .3, .3, 1), age * 2);
	float4 mainColor = float4(.7, .7, .7, 1);
	//if (particleIndex % 3 == 0) mainColor = float4(1, .8, .3, 1);
	o.Color = lerp(mainColor, float4(.2, .2, .2, 1), age * 2);
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}