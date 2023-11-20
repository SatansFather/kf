#include "hlslglobal.hlsl"

//StructuredBuffer<KBaseParticle> ParticleBuffer : register(t0);
StructuredBuffer<KBoundingBoxRender> CubeBuffer : register(t0);

struct vs_out
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

vs_out main(uint id : SV_VERTEXID)
{
	// TODO particle index needs to be offset for a particle's position in master buffer

	uint cubeIndex = id / 8;
	uint vertexInCube = id % 8;

	float3 prevPos = CubeBuffer[cubeIndex].PrevPos.xyz;
	float3 currentPos = CubeBuffer[cubeIndex].CurrentPos.xyz;

	float3 pos;
	if (vertexInCube == 0)
	{
		pos.x = -1;
		pos.y = 1;
		pos.z = 1;
	}
	else if (vertexInCube == 1)
	{
		pos.x = 1;
		pos.y = 1;
		pos.z = 1;
	}
	else if (vertexInCube == 2)
	{
		pos.x = -1;
		pos.y = 1;
		pos.z = -1;
	}
	else if (vertexInCube == 3)
	{
		pos.x = 1;
		pos.y = 1;
		pos.z = -1;
	}
	if (vertexInCube == 4)
	{
		pos.x = -1;
		pos.y = -1;
		pos.z = 1;
	}
	else if (vertexInCube == 5)
	{
		pos.x = 1;
		pos.y = -1;
		pos.z = 1;
}
	else if (vertexInCube == 6)
	{
		pos.x = -1;
		pos.y = -1;
		pos.z = -1;
	}
	else if (vertexInCube == 7)
	{
		pos.x = 1;
		pos.y = -1;
		pos.z = -1;
	}

	pos *= CubeBuffer[cubeIndex].HalfExtent.xyz;
	pos += lerp(prevPos, currentPos, RenderAlpha);

	vs_out o;
	o.Position = mul(float4(pos, 1), ModelViewProjection);
	o.Color = CubeBuffer[cubeIndex].Color;
	return o;
}