#include "hlslglobal.hlsl"

float4 main(float4 positon : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	return color;
}