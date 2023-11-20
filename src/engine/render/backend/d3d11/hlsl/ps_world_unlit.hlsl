float4 main(
	float4 position : SV_POSITION,
	float4 fragpos : FRAGPOS,
	float2 texcoord : TEXCOORD,
	float3 norm : NORMAL,
	float2 lightcoord : LIGHTCOORD,
	float4 lightfragpos : LIGHTSPACE,
	float4 minmaxcoords : COORDCLAMP
) : SV_TARGET
{
	return float4(normalize(abs(norm)), 1);
}