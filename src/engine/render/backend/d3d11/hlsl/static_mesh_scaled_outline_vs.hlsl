#include "hlslglobal.hlsl"

struct VOut
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 PixelPosition : WORLDPOS;
};

float4 qmul(float4 q1, float4 q2)
{
	return float4(
		q2.xyz * q1.w + q1.xyz * q2.w + cross(q1.xyz, q2.xyz),
		q1.w * q2.w - dot(q1.xyz, q2.xyz));
}

// Vector rotation with a quaternion
// http://mathworld.wolfram.com/Quaternion.html
float3 rotate_vector(float3 v, float4 r)
{
	float4 r_c = r * float4(-1, -1, -1, 1);
	return qmul(r, qmul(float4(v, 0), r_c)).xyz;
}

VOut main(
	float4 pos : POSITION,
	float2 tex : TEXCOORD,
	float3 normal : NORMAL,
	float4x4 prev : PREV,
	float4x4 current : CURRENT,
	float4 data : DATA,
	uint id : SV_InstanceID
)
{
	VOut vout;

	float alpha = saturate(RenderAlpha) / data.y;
	float4x4 mat = lerp(prev, current, alpha);

	float3 outPos;
	outPos.x = mat[0][3];
	outPos.y = mat[1][3];
	outPos.z = mat[2][3];

	mat = m_translate(mat, 0);
	mat = m_scale(mat, float3(1.1, 1.1, 1.1));
	mat = m_translate(mat, outPos);
	

	vout.Position = mul(pos, transpose(mul(mul(Projection, View), mat)));
	vout.Texcoord = tex;
	vout.PixelPosition = mul(mat, pos);

	vout.Normal = normalize(mul(normal, transpose(inverse(mat))));
	//float4 quat = matrix_to_quaternion(mat);
	//vout.Normal = normalize(rotate_vector(normal, quat));

	return vout;
}