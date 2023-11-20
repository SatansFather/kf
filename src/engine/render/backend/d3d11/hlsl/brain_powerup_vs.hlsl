#include "hlslglobal.hlsl"

struct VOut
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 PixelPosition : WORLDPOS;
};

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

	float4 position = pos;
	float4 add = 0;
	float t = Time;

	if ((round(sin(position.x) * 100)) % 2 == 0)
	{
		t = -t;
		add += float4(.1, 0, 0, 0) / 5;
	}
	if ((round(sin(position.y) * 100)) % 2 == 0)
	{
		t *= 1.1;
		//add += float4(0, .1, 0, 0);
	}
	if ((round(sin(position.z) * 100)) % 2 == 0)
	{
		t += .2;
		add += float4(0, 0, .1, 0) / 5;
	}

	position.x *= (sin(t * 2) / 8) + 1;
	position.z *= (sin(t * 3) / 8) + 1;
	position.y *= (sin((t * 1.5 + .2) * 1.5) / 8) + 1;
	position += add;

	vout.Position = mul(position, transpose(mul(mul(Projection, View), mat)));
	vout.Normal = normalize(mul(normal, transpose(inverse(mat))));
	vout.PixelPosition = mul(pos, mat);
	vout.Texcoord = tex;

	return vout;
}