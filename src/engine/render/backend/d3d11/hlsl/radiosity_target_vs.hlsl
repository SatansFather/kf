#include "hlslglobal.hlsl"

struct vs_out
{
	float4 position : SV_POSITION;
	float2 coord : TEXCOORD;
	float3 UV : _UV;
	float2 UVDot : _UVDOT;
};

vs_out main(uint id : SV_VERTEXID)
{
	vs_out o;
	o.position.x = (float)(id / 2) * 4.f - 1.f;
	o.position.y = (float)(id % 2) * 4.f - 1.f;
	o.position.z = 0.f;
	o.position.w = 1.f;

	o.coord.x = (float)(id / 2) * 2.f;
	o.coord.y = 1.f - (float)(id % 2) * 2.f;

	o.coord *= 64;

	float strength = 1;
	float aspectRatio = 1;
	float height = tan(3.12414 / 2.f);
	float cylindricalRatio = 1;

	float scaledHeight = strength * height;
	float cylAspectRatio = aspectRatio * cylindricalRatio;
	float aspectDiagSq = aspectRatio * aspectRatio + 1.0;
	float diagSq = scaledHeight * scaledHeight * aspectDiagSq;
	float2 signedUV = (2.0 * float2(o.coord.x, o.coord.y) + float2(-1.0, -1.0));

	float z = 0.5 * sqrt(diagSq + 1.0) + 0.5;
	float ny = (z - 1.0) / (cylAspectRatio * cylAspectRatio + 1.0);

	o.UVDot = sqrt(ny) * float2(cylAspectRatio, 1.0) * signedUV;
	o.UV = float3(0.5, 0.5, 1.0) * z + float3(-0.5, -0.5, 0.0);
	o.UV.xy += float2(o.coord.x, o.coord.y);

	return o;
}