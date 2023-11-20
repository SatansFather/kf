#include "hlslglobal.hlsl"

StructuredBuffer<KAtomProjectile> ParticleBuffer : register(t0);

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
	uint bufferIndex = particleIndex / (ATOM_SHELL_COUNT * ATOM_SHELL_PARTICLE_COUNT);
	particleIndex %= ATOM_SHELL_COUNT * ATOM_SHELL_PARTICLE_COUNT;
	uint shellIndex = particleIndex / ATOM_SHELL_PARTICLE_COUNT;
	uint particleInShell = particleIndex % ATOM_SHELL_PARTICLE_COUNT;

	KAtomProjectile atom = ParticleBuffer[bufferIndex];

	const float3 position = atom.GetPosition(RenderAlpha);

	float x = 3 * sin(Time * 3) + 32 * (random((float)particleIndex) - .5);
	float y = 0 * tan(Time * 3) + 32 * (random((float)particleIndex * 1.5) - .5);
	float z = 3 * cos(Time * 3) + 32 * (random((float)particleIndex * 1.3) - .5);
	float3 xyz = normalize(float3(x, y, z)) * 16;
	float3 p = xyz;
	p.x += 1 * sin(11 * Time + 2000 * random((float)particleIndex));
	p.y += 1 * sin(11 * Time + 2000 * random((float)particleIndex * 1.5));
	p.z += 1 * sin(11 * Time + 2000 * random((float)particleIndex * 1.3));
	p += position;

	/*
	const float3 directions[] = 
	{
		float2( 0,      1),
		float2(-0.866, -0.5),
		float2(-0.866,  0.5),
		//float3(-0.707, 0, 0.707)
	};

	const float3 position = atom.GetPosition(RenderAlpha);
	const float pi2 = 2.f * 3.141592;

	const float rand = random(abs(float(particleInShell) - (ATOM_SHELL_PARTICLE_COUNT - 2) ));

	float angle = (Time * 5) + (float(shellIndex) / float(ATOM_SHELL_COUNT)) * pi2;
	angle += (float(particleInShell) / float(ATOM_SHELL_PARTICLE_COUNT)) * pi2;

	const float2 dir2 = normalize(lerp
			(directions[shellIndex], 
			directions[(shellIndex + 1) % ATOM_SHELL_COUNT], 
			float(particleInShell) / float(ATOM_SHELL_PARTICLE_COUNT)));

	const float3 dir = float3(dir2.xy, 0);

	const float x = sin(angle) * (sin(Time * 7 + rand) * 4 + 16);
	const float y = cos(angle) * (cos(Time * 7 + rand) * 4 + 16);
	const float3 zDir = cross(float3(0, 0, 1), dir);

	float3 aAdjust = (x * dir);
	float3 bAdjust = (y * float3(0, 0, 1));
	float3 cAdjust = zDir * sin(Time * 7 * rand) * 5;

	float3 p = position + aAdjust + bAdjust + cAdjust;
	*/
	
	float3 pos;
	pos.x = (vertexInQuad % 2) ? 1 : -1;
	pos.y = (vertexInQuad & 2) ? -1 : 1;
	pos.z = 0;
	pos.xy *= .5;
	pos = mul(pos, (float3x3)View) + p;

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Color = float4(1, 1, 1, 1);
	o.TexCoord.x = ((vertexInQuad % 2) ? 1 : 0);
	o.TexCoord.y = ((vertexInQuad & 2) ? 1 : 0);
	return o;
}