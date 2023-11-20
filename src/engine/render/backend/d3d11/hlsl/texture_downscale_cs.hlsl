#include "hlslglobal.hlsl"

RWTexture2D<float4> Output : register(u0);
Texture2D<float4> Input : register(t0);

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint2 coord = dispatchThreadID.xy * 2;
	float2 texCoord = coord;
	texCoord.x /= Resolution.x;
	texCoord.y /= Resolution.y;


	//float4 color = 0.0f;
	//color += Input.SampleLevel(SmpUnfiltered, float2(texCoord.x / Resolution.x, texCoord.y / Resolution.y), 0);
	//color += Input.SampleLevel(SmpUnfiltered, float2((texCoord.x + 1) / Resolution.x, texCoord.y / Resolution.y), 0);
	//color += Input.SampleLevel(SmpUnfiltered, float2(texCoord.x / Resolution.x, (texCoord.y + 1) / Resolution.y), 0);
	//color += Input.SampleLevel(SmpUnfiltered, float2((texCoord.x + 1) / Resolution.x, (texCoord.y + 1) / Resolution.y), 0);
	Output[dispatchThreadID.xy] = Input.SampleLevel(SmpUnfiltered, texCoord, 0);//color / 4.0f;
}