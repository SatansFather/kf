#include "hlslglobal.hlsl"

StructuredBuffer<KTorchFlame> ParticleBuffer : register(t0);

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
	uint bufferIndex = particleIndex / TORCH_FLAME_PARTICLE_COUNT;

	KTorchFlame flame = ParticleBuffer[bufferIndex];
	particleIndex %= TORCH_FLAME_PARTICLE_COUNT;

	const float cycle = .5;
	const float3 position = flame.GetPosition();
	const float4 mainColor = flame.GetMainColor();
	const float4 offColor = flame.GetOffColor();
	const float time = Time + float(particleIndex) / float(TORCH_FLAME_PARTICLE_COUNT);
	//const float alpha = RoundNearest(frac(time), .05);
	const float alpha = frac(time);

	const float xRand = random(position.x + particleIndex) - .5;
	const float zRand = random(position.z + particleIndex) - .5;

	float3 start = float3(xRand, 0, zRand);
	if (lengthsq(start) > 1) start = normalize(start);

	const float off = lengthsq(start);
	start *= lerp(1, .6, off);
	start.y += lerp(0, -5.5, off);


	start = start * 4;
	float3 end = position + float3(0, 16, 0) + start * .3;
	start += position;
	
	float3 p = lerp(start, end, pow(alpha, 1.5));

	p.x += sin(time * xRand * 5 * random(particleIndex) - .5);
	p.y += cos(time * zRand * 5 * random(particleIndex) - .5);

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 6 * (1 - alpha)
	 * ((alpha < .5) ? (alpha * 2) : 1);
	 //* ((particleIndex % 2) ? 2 : 1);// * ((particleIndex % 2) ? .4 : 1);
	pos = mul(pos, (float3x3)View) + p;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Color = (particleIndex % 2) ? mainColor : offColor;
	o.Color.xyz *= 2;
	o.Color.a *= (alpha < .5) ? 1 : map(alpha, .5, 1, 1, 0);
	o.TexCoord.x = ((vertexInQuad % 2) ? 1 : 0);
	o.TexCoord.y = ((vertexInQuad & 2) ? 1 : 0);
	return o;
}