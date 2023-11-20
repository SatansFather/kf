#include "hlslglobal.hlsl"

RWTexture2D<float4> Output : register(u0);
Texture2D<float4> Input : register(t0);

#define RADIUS 12
#define KERNEL_SIZE 12//(2 * RADIUS + 1)

float weights[] = { 0.0044, 0.0540, 0.2420, 0.3989, 0.2420, 0.0540, 0.0044 };

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int2 coord = dispatchThreadID.xy;
	float4 sum = 0.0;

	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		int2 offset = int2(i - RADIUS, 0);
		float2 texCoord = coord + offset;
		texCoord.x /= Resolution.x;
		texCoord.y /= Resolution.y;
		float4 smp = Input.SampleLevel(SmpFiltered, texCoord, 0);
		sum += smp;
	}

	Output[coord] = sum;
}