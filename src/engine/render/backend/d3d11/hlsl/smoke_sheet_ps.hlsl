#include "hlslglobal.hlsl"

Texture2D tex : register(t0);

float4 main(float4 position : SV_POSITION, float2 texCoord : TEXCOORD) : SV_TARGET
{
	const float frameRate = 25;
	const uint frameNumber = Time * frameRate;
	const uint dimension = 8;
	const float row = frameNumber % dimension;
	const float col = frameNumber / dimension;
	
	float2 coord = texCoord;
	coord.x = map(coord.x, 0, 1, row / float(dimension), (row + 1) / float(dimension));
	coord.y = map(coord.y, 0, 1, col / float(dimension), (col + 1) / float(dimension));
	float4 color = tex.Sample(SmpUnfiltered, coord);
	
	return color;

}