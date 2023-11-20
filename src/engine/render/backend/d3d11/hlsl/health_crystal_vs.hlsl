#include "hlslglobal.hlsl"

struct VOut
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 PixelPosition : WORLDPOS;
	float4 Color : COLOR;
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

	vout.Position = mul(pos, transpose(mul(mul(Projection, View), mat)));
	vout.Texcoord = tex;
	vout.PixelPosition = mul(mat, pos);

	vout.Normal = normalize(mul(normal, transpose(inverse(mat))));
	//float4 quat = matrix_to_quaternion(mat);
	//vout.Normal = normalize(rotate_vector(normal, quat));

	float4 color;
	uint col = asuint(data.z);
	color.a = (col >> 24) / 255.f;
	color.r = ((col >> 16) & 0xFF) / 255.f;
	color.g = ((col >> 8) & 0xFF) / 255.f;
	color.b = (col & 0xFF) / 255.f;
	vout.Color = color;

	return vout;
}