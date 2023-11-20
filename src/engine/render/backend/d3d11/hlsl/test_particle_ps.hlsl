#include "hlslglobal.hlsl"

Texture2D tex : register(t0);
Texture2D tex1 : register(t1);

float4 main(float4 positon : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float2 coord = texcoord;
	if (abs(CamPitch) >= 90) coord = -coord; 

	return tex.Sample(SmpUnfiltered, coord);

	float4 r = tex1.Sample(SmpUnfiltered, texcoord);
	float d = r.r;
		
	d = pow(d, 5);
	
	r = float4(d, d, d, d);
	return r;
}