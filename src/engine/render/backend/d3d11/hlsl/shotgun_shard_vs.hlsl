#include "hlslglobal.hlsl"

//StructuredBuffer<KBaseParticle> ParticleBuffer : register(t0);
StructuredBuffer<KShotgunShard> ShardBuffer : register(t0);

struct vs_out
{
	float4 Position : SV_POSITION;
	float4 Normal : NORMAL;
};

float4x4 RotateMatrix(float4x4 mat, float angle, float3 axis)
{
	const float a = angle;
	const float c = cos(a);
	const float s = sin(a);

	axis = normalize(axis);
	float3 temp = (1.f - c) * axis;

	float4x4 Rotate;
	Rotate[0][0] = c + temp[0] * axis[0];
	Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
	Rotate[0][2] = temp[0] * axis[2] - s * axis[1];

	Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
	Rotate[1][1] = c + temp[1] * axis[1];
	Rotate[1][2] = temp[1] * axis[2] + s * axis[0];

	Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
	Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
	Rotate[2][2] = c + temp[2] * axis[2];

	float4x4 Result;
	Result[0] = mat[0] * Rotate[0][0] + mat[1] * Rotate[0][1] + mat[2] * Rotate[0][2];
	Result[1] = mat[0] * Rotate[1][0] + mat[1] * Rotate[1][1] + mat[2] * Rotate[1][2];
	Result[2] = mat[0] * Rotate[2][0] + mat[1] * Rotate[2][1] + mat[2] * Rotate[2][2];
	Result[3] = mat[3];
	return Result;
}

vs_out main(uint id : SV_VERTEXID)
{
	vs_out o;

	uint shardIndex = id / 6;
	uint vertexInShard = id % 6;

	KShotgunShard shard = ShardBuffer[shardIndex];

	float3 prevPos = shard.GetPrevPos();
	float3 currentPos = shard.GetCurrentPos();
	//if (ShardBuffer[shardIndex].GetLastMoveAlpha() == 1) prevPos = currentPos;

	float randSeed = (random(shard.GetRandomSeed()) - .5) * 2;

	float3 pos;
	if (vertexInShard == 0)
	{
		pos.x = 0;
		pos.y = 1;
		pos.z = 0;
	}
	else if (vertexInShard == 1)
	{
		pos.x = 1;
		pos.y = 0;
		pos.z = 0;
	}
	else if (vertexInShard == 2)
	{
		pos.x = -1;
		pos.y = 0;
		pos.z = 0;
	}
	else if (vertexInShard == 3)
	{
		pos.x = 0;
		pos.y = 0;
		pos.z = 1;
	}
	else if (vertexInShard == 4)
	{
		pos.x = 0;
		pos.y = 0;
		pos.z = -1;
	}
	else if (vertexInShard == 5)
	{
		pos.x = 0;
		pos.y = -1;
		pos.z = 0;
	}
	o.Normal = float4(pos, 1);
	pos.x += random(vertexInShard * randSeed) + randSeed / 4;
	pos.y += random(vertexInShard * randSeed) + random(randSeed) / 4;
	pos.z += random(vertexInShard * randSeed) + random(randSeed * 2) / 4;

	pos *= 3 * (saturate(RenderAlpha) > shard.GetLastMoveAlpha() ? 0 : 1);

	float alpha = saturate(RenderAlpha) / shard.GetLastMoveAlpha();
	float3 newPos = lerp(shard.GetPrevPos().xyz,
		shard.GetCurrentPos().xyz,
			alpha);

	float4x4 mvp = mul(Projection, View);
	float4x4 model = RotateMatrix(ModelTransform, Time * (32 + randSeed * 32), float3(randSeed, random(randSeed), random(randSeed * 2)));
	model = m_translate(model, newPos);
	mvp = transpose(mul(mvp, model));

	o.Position = mul(float4(pos, 1), mvp);
	return o;
}