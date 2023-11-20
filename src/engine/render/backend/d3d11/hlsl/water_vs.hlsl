#include "hlslglobal.hlsl"

struct vs_out
{
	float4 position : SV_POSITION;
	float4 fragpos : FRAGPOS;
	float2 coord : TEXCOORD;
	float3 norm : NORMAL;
	float2 lightcoord : LIGHTCOORD;
	//float4 lightfragpos : LIGHTSPACE;
	//float4 coordclamp : COORDCLAMP;
	float3 lightcolor : LIGHTCOLOR;
	float depthValue : DEPTH;
	float bufferIndex : BUFFERINDEX;
};

vs_out main(float4 pos : POSITION, float2 texcoord : TEXCOORD, float3 norm : NORMAL, float2 lightcoord : LIGHTCOORD, float4 minmaxcoords : COORDCLAMP, float3 lightcolor : LIGHTCOLOR, uint bufferIndex : BUFFERINDEX)
{
	vs_out o;
	o.position = mul(pos, ModelViewProjection);
	o.lightcoord = lightcoord;
	o.norm = norm;
	o.fragpos = mul(pos, ModelTransform);
	//o.lightfragpos = mul(LightSpaceMatrix, o.fragpos);
	o.coord = texcoord;
	//o.coordclamp = minmaxcoords;
	o.lightcolor = lightcolor;
	o.bufferIndex = bufferIndex;

	//float4 eyepos = mul(mul(Projection, View), mul(float4(pos.xyz, 1), ModelTransform));
	//float4 eyepos = mul(pos, mul(Projection, View));
	//o.depthValue = (1.0 - 0.0) * 0.5 * (o.position.z / o.position.w) + (1.0 + 0.0) * 0.5;

	// this gives eye space
	// eyepos.z / eyepos.w is depth
	//o.eyepos = mul(mul(Projection, View), o.fragpos);

	return o;
}