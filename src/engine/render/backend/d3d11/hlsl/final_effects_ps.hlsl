	#include "hlslglobal.hlsl"

Texture2D Input : register(t0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float saturation = ScreenAdjust.x;
	float brightness = ScreenAdjust.y;
	float contrast = ScreenAdjust.z;

	float teleScale = 2.5 - (saturate(pow((Time - LastTeleportTime), 2) * 2) * 1.5);
	
	float2 coord = texcoord;
	if (ScreenAdjust.w == 1) coord.x = 1 - coord.x;

	//if (abs(CamPitch) >= 90) coord = -coord;

	float4 diffuse = Input.Sample(SmpRenderScale, coord);
	//float4 diffuse = SampleBlurred(Input, coord, 1, 4);
	
	uint w, h, m;
	Input.GetDimensions(0, w, h, m);

	w = Resolution.x;
	h = Resolution.y;

	float rscale = RenderScale;

	const float x = 1.f / w * 1;
	const float y = 1.f / h * 1;

	float u = position.x / w * rscale;
	float v = position.y / h * rscale;
	if (ScreenAdjust.w == 1) u = 1 - u;
	float concuss = ConcussionStrength;
	concuss += pow((teleScale - 1) / 3, 3);

	[flatten]
	if (concuss > 0.01)
	{
		float blurSize = 25 * concuss * rscale;

		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - (y * 4) * blurSize)) * 0.05;
		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - (y * 3) * blurSize)) * 0.09;
		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - (y * 2) * blurSize)) * 0.12;
		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - (y) * blurSize)) * 0.15;
		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - y))  * 0.16;
		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - (y)* blurSize)) * 0.15;
		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - (y * 2) * blurSize)) * 0.12;
		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - (y * 3) * blurSize)) * 0.09;
		diffuse += Input.Sample(SmpUnfiltered, float2(u, v - (y * 4) * blurSize)) * 0.05;
		
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 4) * blurSize, v)) * 0.05;
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 3) * blurSize, v)) * 0.09;
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 2) * blurSize, v)) * 0.12;
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 1) * blurSize, v)) * 0.15;
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 1) * blurSize, v)) * 0.16;
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 1) * blurSize, v)) * 0.15;
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 2) * blurSize, v)) * 0.12;
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 3) * blurSize, v)) * 0.09;
		diffuse += Input.Sample(SmpUnfiltered, float2(u - (x * 4) * blurSize, v)) * 0.05;

		diffuse /= 3;

		float dist = distancesq(float2(u, v), float2(.5, .5));
		float adjust = 1 - dist * concuss;
		diffuse *= adjust;
	}

	diffuse += (brightness * 1.1) - 1;
	diffuse = pow(diffuse + .5, contrast * 1.4) - .5;

	diffuse *= float4(teleScale, 1, teleScale, 1);
	//if (Time < .5) diffuse *= saturate(sqrt(Time * 2));

	//const float shadesPerColor = 16.f;
	//diffuse.x = RoundNearest(diffuse.x, 1.f/shadesPerColor);
	//diffuse.y = RoundNearest(diffuse.y, 1.f/shadesPerColor);
	//diffuse.z = RoundNearest(diffuse.z, 1.f/shadesPerColor);

	float3 greyScale = float3(0.212, 0.7152, 0.0722) * saturation;
	float d = dot(diffuse.xyz, greyScale);
	diffuse += diffuse * float4(d, d, d, 1);

	//diffuse.x = floor(diffuse.x * 7.f) / 7.f;
	//diffuse.y = floor(diffuse.y * 7.f) / 7.f;
	//diffuse.z = floor(diffuse.z * 7.f) / 7.f;
	
	return diffuse;
}