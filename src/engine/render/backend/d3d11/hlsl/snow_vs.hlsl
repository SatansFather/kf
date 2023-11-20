#include "hlslglobal.hlsl"

//StructuredBuffer<KBaseParticle> ParticleBuffer : register(t0);

//Texture2D<float> StopPoints : register(t1);
StructuredBuffer<float> StopPoints : register(t0);
SamplerState StopSampler : SMP_REGISTER(1);


struct vs_out
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float4 Data : DATA;
	float2 TexCoord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	// TODO particle index needs to be offset for a particle's position in master buffer

	uint countX = PrecipMin.w;
	uint countY = PrecipMax.w;

	uint particleIndex = id / 4;
	uint vertexInQuad = id % 4;

	uint xInd = particleIndex / countX;
	uint yInd = particleIndex % countX;
	float stopPoint = StopPoints[particleIndex];
	
	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -8 : 8;
	//pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 1;
	pos = mul(pos, (float3x3)View);

	float3 add = 0;
	add.x = (PrecipMax.x - PrecipMin.x) * (float(xInd) / float(countY));
	add.z = (PrecipMax.z - PrecipMin.z) * (float(yInd) / float(countX));
	add += float3(PrecipMin.x, 0, PrecipMin.z);
	pos += add;

	float time = Time;
	float rand = random(float(particleIndex) / 4.f) * 3;
	time += rand;
	rand = map(rand, 0, 3, .5, 1.5);
	time *= rand;
	time = time - int(time / 2) * 2;
	pos.y += map(time, 0, 2, PrecipMax.y, PrecipMin.y);
	pos.y = clamp(pos.y, PrecipMin.y, PrecipMax.y);
	pos.x += (sin(time * rand) * 16);
	pos.z += (cos(time * rand) * 16);

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Data = 0;
	o.Color = float4(.7, .7, .8, .5);
	//o.Color = float4(1, 0, 0, .5);
	if (pos.y < stopPoint) o.Color.a = 0;
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;
	return o;
}