#include "hlslglobal.hlsl"

StructuredBuffer<KPortalData> PortalBuffer : T_REGISTER(MAT_BUFFER_SLOT);

Texture2D reftex : register(t0);

#define mod(x,y) (x-y*floor(x/y))
#define TAU 6.28318530718

float4 main(
	float4 position : SV_POSITION,
	float4 fragpos : FRAGPOS,
	float2 texcoord : TEXCOORD,
	float3 norm : NORMAL,
	float2 lightcoord : LIGHTCOORD,
	float4 lightfragpos : LIGHTSPACE,
	float4 minmaxcoords : COORDCLAMP,
	float3 lightcolor : LIGHTCOLOR,
	uint bufferindex : BUFFERINDEX
) : SV_TARGET
{
	float4 fog = GetFogValue(fragpos);
	if (fog.a == 1)
		return fog;

	KPortalData data = PortalBuffer[bufferindex];

	const float timescale = data.GetTimeScale();
	const float framerate = 0;
	const float rippleRate = data.GetRippleRate();
	const float heightRatio = data.GetHeightRatio();
	const float unitsPerPixel = .5;
	const int maxiter = 4;
	//const float3 colorA = float3(.5, 0, 0);
	//const float3 colorB = float3(0, 0, 0);

	//const float3 colorA = float3(0, .35, .5);
	//const float3 colorB = float3(1, 1, 1);

	const float3 colorA = data.GetBackColor();
	const float3 colorB = data.GetRippleColor();

	float t = Time * timescale;

	if (framerate > 0)
		t = RoundNearest(t, 1.f / framerate * timescale);

	texcoord.y = RoundNearest(texcoord.y, 1.0 / (32.f / unitsPerPixel));
	texcoord.x = RoundNearest(texcoord.x, 1.0 / (32.f / unitsPerPixel));
	if (heightRatio > 1)
		texcoord.y /= heightRatio;
	else if (heightRatio < 1)
		texcoord.x *= heightRatio;

	float time = t / 2 + 23.0 + length(texcoord * 16);
	float2 uv = texcoord.xy / 2;

	float2 p = mod(uv * TAU, TAU) - 250.0;

	float2 i = float2(p);
	float c = 1.0;
	float inten = .02;

	for (int n = 0; n < maxiter; n++)
	{
		float t = time * (1.0 - (rippleRate / float(n + 1)));
		i = p + float2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0 / length(float2(p.x / (sin(i.x + t) / inten), p.y / (cos(i.y + t) / inten)));
	}
	c /= float(maxiter);
	c = 1.17 - pow(c, 1.4);
	float3 color = (pow(abs(c), 8.0));
	//color = saturate(color + float3(0.0, 0.35, 0.5));

	color = lerp(colorA, colorB, color.r);
	return lerp(float4(color, .5), float4(fog.xyz, 1), fog.w);
}