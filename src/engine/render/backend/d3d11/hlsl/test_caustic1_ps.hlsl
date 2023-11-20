#include "hlslglobal.hlsl"

Texture2D tex : register(t0);
Texture2D lightmap : register(t1);

#define mod(x,y) (x-y*floor(x/y))
#define TAU 6.28318530718

float roundnearest(float num, float mult)
{
	num = num + mult / 2;
	num -= num % mult;
	return num;
}

float4 main(
	float4 position : SV_POSITION,
	float4 fragpos : FRAGPOS,
	float2 texcoord : TEXCOORD,
	float3 norm : NORMAL,
	float2 lightcoord : LIGHTCOORD,
	float4 lightfragpos : LIGHTSPACE,
	float4 minmaxcoords : COORDCLAMP,
	float3 lightcolor : LIGHTCOLOR
) : SV_TARGET
{
	const float timescale = 1.5;
	const float framerate = 144;
	const int maxiter = 8;
	//const float3 colorA = float3(.5, 0, 0);
	//const float3 colorB = float3(0, 0, 0);

	const float3 colorA = float3(0, .35, .5);
	const float3 colorB = float3(1, 1, 1);

	float t = Time * timescale;
	//t = roundnearest(t, 1 / framerate * timescale);

	//float2 texcoord = coord;
	//texcoord.y = roundnearest(texcoord.y, 1.0 / 96.0);
	//texcoord.x = roundnearest(texcoord.x, 1.0 / 96.0);

	float time = t / 2 + 23.0;// + length(texcoord * 12);
	float2 uv = texcoord.xy / float2(.5, .5);

	float2 p = mod(uv * TAU, TAU) - 250.0;

	float2 i = float2(p);
	float c = 1.0;
	float inten = .005;

	for (int n = 0; n < maxiter; n++)
	{
		float t = time * (1.0 - (3.5 / float(n + 1)));
		i = p + float2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0 / length(float2(p.x / (sin(i.x + t) / inten), p.y / (cos(i.y + t) / inten)));
	}
	c /= float(maxiter);
	c = 1.17 - pow(c, 1.4);
	float3 color = (pow(abs(c), 8.0));
	//color = saturate(color + float3(0.0, 0.35, 0.5));

	color = lerp(colorA, colorB, color.r);

	return float4(color, .5) * (lightmap.Sample(SmpFiltered, lightcoord) + float4(lightcolor, 1));// * tex.Sample(SmpUnfiltered, GetAtlasCoords(texcoord, float2(minmaxcoords.x, minmaxcoords.y), float2(minmaxcoords.z, minmaxcoords.w)));
}


#if 0

#include "cbuff.hlsl"

Texture2D tex : register(t0);
SamplerState unfiltered : register(s0);

#define mod(x,y) (x-y*floor(x/y))
#define TAU 6.28318530718
#define MAX_ITER 8

float roundnearest(float num, float mult)
{
	num = num + mult / 2;
	num -= num % mult;
	return num;
}

float4 main(
	float4 position : SV_POSITION,
	float4 fragpos : FRAGPOS,
	float2 coord : TEXCOORD,
	float3 norm : NORMAL,
	float4 lightfragpos : LIGHTSPACE
) : SV_TARGET
{
	const float timescale = 1.5;
	const float framerate = 144;
	const int maxiter = 5;
	const float3 colorA = float3(1, 1, 1);
	const float3 colorB = float3(0, .35, .5);

	float t = Time * timescale;
	//t = roundnearest(t, 1 / framerate * timescale);

	float2 texcoord = coord;
	//texcoord.y = roundnearest(texcoord.y, 1.0 / 96.0);
	//texcoord.x = roundnearest(texcoord.x, 1.0 / 96.0);

	float time = t + 23.0 + length(texcoord * 12);
	float2 uv = texcoord.xy / float2(.5, .5);

	float2 p = mod(uv * TAU, TAU) - 250.0;

	float2 i = float2(p);
	float c = 1.0;
	float inten = .005;

	for (int n = 0; n < maxiter; n++)
	{
		float t = time * (1.0 - (3.5 / float(n + 1)));
		i = p + float2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0 / length(float2(p.x / (sin(i.x + t) / inten), p.y / (cos(i.y + t) / inten)));
	}
	c /= float(MAX_ITER);
	c = 1.17 - pow(c, 1.4);
	float3 color = (pow(abs(c), 8.0));
	color = clamp(color + float3(0.0, 0.35, 0.5), 0.0, 1.0);
	return float4(color, 1);

	//return float4(lerp(colorA, colorB, diffuse), 1.0);// * tex.Sample(unfiltered, coord);
}

#endif