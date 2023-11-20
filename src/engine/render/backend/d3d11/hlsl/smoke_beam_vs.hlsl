#include "hlslglobal.hlsl"

StructuredBuffer<KSmokeBeam> ParticleBuffer : register(t0);
StructuredBuffer<uint> IndexMap : register(t1);

Texture2D WaterDepth : register(t12);

struct vs_out
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD;
};

vs_out main(uint id : SV_VERTEXID)
{
	uint particleIndex = id / 4;
	uint vertexInQuad = id % 4;
	uint bufferIndex = IndexMap[particleIndex];

	float alpha = saturate(RenderAlpha);

	KSmokeBeam beam = ParticleBuffer[bufferIndex];

	uint indexInBeam = particleIndex - beam.GetStartIndex();
	
	float distAlongBeam = beam.GetSpacing() * indexInBeam;

	float3 endPos = lerp(beam.GetLastEndPos(), beam.GetEndPos(), alpha);

	if (distAlongBeam * distAlongBeam > distancesq(beam.GetStartPos(), endPos))
	{
		// end is in front of where we should lerp to
		vs_out o;
		o.Color = 0;
		return o;
	}
	
	float3 dir = endPos - beam.GetStartPos();
	float len = length(dir);
	dir = dir / len;

	float3 smokePos = beam.GetStartPos() + dir * distAlongBeam;

	float rand1 = beam.GetAge() * 2 * random(indexInBeam + beam.GetTimeOffset());
	float rand2 = beam.GetAge() * 2 * random(indexInBeam * beam.GetTimeOffset());
	float rand3 = beam.GetAge() * 2 * random(indexInBeam * beam.GetTimeOffset() * 23);

	float sign1 = saturate(indexInBeam % 2) * 2 - 1;
	float sign2 = saturate((indexInBeam * uint(beam.GetTimeOffset() * 500.34)) % 2) * 2 - 1;
	float sign3 = saturate((indexInBeam * uint(beam.GetTimeOffset() * 5123.32)) % 2) * 2 - 1;

	float spreadFactor = 3.5 * saturate(distAlongBeam / 128);

	smokePos.x += rand1 * spreadFactor * sign1;
	smokePos.y += rand2 * spreadFactor * sign2;
	smokePos.z += rand3 * spreadFactor * sign3;

	smokePos += random(indexInBeam + beam.GetTimeOffset()) * dir * sign1 * beam.GetSpacing() * 2;

	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= 3 * beam.GetRadius();// * (1 - beam.GetAge());
	pos = mul(pos, (float3x3)View) + smokePos;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	//o.Data = ParticleBuffer[particleIndex].Data;
	//o.TimeOffset = beam.GetTimeOffset() + random(indexInBeam);
	o.TexCoord.x = (vertexInQuad % 2) ? 1 : 0;
	o.TexCoord.y = (vertexInQuad & 2) ? 1 : 0;


	o.Color = beam.GetColor();
	//o.Color.a = 1;
	if (sign1 == 1) 
	{
		o.Color.r = .5;
		o.Color.g /= 3;
		o.Color.b /= 3;
	}

	float transparency = distAlongBeam / 64;
	transparency = saturate(transparency);
	const float pi = 3.141592;
	float a = sin(pi * beam.GetAge() - (pi / 2)) / 2 + .5;

	/*float endStart = len - 512;
	endStart = max(endStart, 0);

	if (distAlongBeam > endStart)
	{
		o.Color.r += (distAlongBeam - endStart) / 512;
		o.Color.g += (distAlongBeam - endStart) / 512;
		a *= 1 + (distAlongBeam - endStart) / 512;;
	}*/
	

	a = min(transparency, (1 - saturate(a)));
	o.Color.a *= a;// * (1 - beam.GetAge());

	//float4 eyepos = mul(mul(Projection, View), mul(float4(pos, 1), ModelTransform));
	//o.Depth = eyepos.z / eyepos.w;

	float2 screenCoord = o.Position.xy;
	float vertDepth = o.Position.z;
	float waterDepth = abs(WaterDepth[screenCoord].r);
	//if (o.Depth > waterDepth)
	//	o.Color = 1;

	return o;
}