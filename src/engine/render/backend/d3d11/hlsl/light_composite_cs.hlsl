#include "hlslglobal.hlsl"

RWTexture2D<float4> Output : register(u0);
Texture2D<float4> Color : register(t0);
Texture2D<float4> Light : register(t1);

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float2 coord = float2(dispatchThreadID.xy) / Resolution;

	float4 color = Color.SampleLevel(SmpUnfiltered, coord, 0);
	float4 light = Light.SampleLevel(SmpFiltered, coord, 0);
	
	Output[dispatchThreadID.xy] = color * light;
}