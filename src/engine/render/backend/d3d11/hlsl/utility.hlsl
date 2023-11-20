#ifndef UTILITY
#define UTILITY

#include "hlslglobal.hlsl"

float map(float value, float in_min, float in_max, float out_min, float out_max)
{
	float perc = (value - in_min) / (in_max - in_min);
	return perc * (out_max - out_min) + out_min;
}

float distancesq(float2 a, float2 b)
{
	float2 c = a - b;
	return c.x * c.x + c.y * c.y;
}

float distancesq(float3 a, float3 b)
{
	float3 c = a - b;
	return c.x * c.x + c.y * c.y + c.z * c.z;
}

float lengthsq(float3 a)
{
	float3 b = 0;
	float3 c = a - b;
	return c.x * c.x + c.y * c.y + c.z * c.z;
}

float linearize(float a, float near, float far)
{
	a = a * 2.0 - 1.0;
	a = (2.0 * near * far) / (far + near - a * (far - near));
	return a / far;
}

float2 waterize(float2 texcoord, float speed, float size, float frequency, float scale = 1)
{
	float X = texcoord.x * 25 / size + Time * speed;
	float Y = texcoord.y * 25 / size + Time * speed;
	texcoord.x += scale * (cos(X + Y) * 0.01 * frequency * cos(Y));
	texcoord.y += scale * (sin(X - Y) * 0.01 * frequency * sin(Y));
	return texcoord;
}

float2 GetAtlasCoords(float2 texcoord, float2 minmaxX, float2 minmaxY)
{
	texcoord.x = map(texcoord.x, 0.f, 1.f, minmaxX.x, minmaxX.y);
	texcoord.y = map(texcoord.y, 0.f, 1.f, minmaxY.x, minmaxY.y);

	float x_min = minmaxX.x;
	float x_max = minmaxX.y;	
	texcoord.x = (((texcoord.x - x_min) % (x_max - x_min)) + (x_max - x_min)) % (x_max - x_min) + x_min;

	float y_min = minmaxY.x;
	float y_max = minmaxY.y;
	texcoord.y = (((texcoord.y - y_min) % (y_max - y_min)) + (y_max - y_min)) % (y_max - y_min) + y_min;

	return texcoord;
}

float RoundNearest(float num, float mult)
{
	num = num + mult / 2;
	num -= num % mult;
	return num;
}

float4 GetFogValue(float3 pos)
{	
	float4 color = 0;
	for (uint i = 0; i < ActiveFogCount; i++)
	{
		float startDist = ActiveFog[i].Data.x;
		float endDist = ActiveFog[i].Data.y;
		float pDist = distance(CamPosition.xyz, pos);

		float alpha = map(pDist, startDist, endDist, 0, 1);	
		alpha = saturate(alpha);
		alpha = sin(3.141592 * alpha + 1.5 * 3.141592) / 2 + .5;
		alpha *= ActiveFog[i].Color.a;

		color += float4(ActiveFog[i].Color.rgb, saturate(alpha));
	}
	return color;
}


float4 SampleBlurred(Texture2D tex, float2 coord, float radius = 4, float skip = 1)
{
	//return tex.Sample(SmpUnfiltered, coord);
	float3 sum = 0;
	float pixelSize = 1.0 / Resolution.y;

	float kernel = radius;
	float weight = 10;

	float w, h;
	tex.GetDimensions(w, h);

	//const float pixelY = 1.0 / Resolution.y * (Resolution.y / 1080);
	//const float pixelX = 1.0 / Resolution.x * (Resolution.y / 1080);
	const float pixelX = 1.0 / w;
	const float pixelY = 1.0 / h;

	float3 accumulation = 0;
	float3 weightsum = 0;
	float alpha = 0;
	for (float i = -kernel; i <= kernel; i++) 
	for (float j = -kernel; j <= kernel; j++) 
	{
	{
		float4 samp = tex.Sample(SmpFiltered, 
			saturate(coord + float2(i * pixelX, j * pixelY) * skip) * .999);

		accumulation += samp.xyz * weight;
		alpha = max(alpha, samp.a);
		weightsum += weight;
	}
	}

	sum = accumulation / weightsum;
	return float4(sum, alpha / weightsum.x);
}

float4 GetLightAtPixel(float3 pos, float3 norm, float4 lightColor, float3 lightPos, float lightRadius, float lightFalloff, bool negative, bool full)
{
	float3 toLight = lightPos - pos;
	float dist = length(toLight);
	toLight /= dist;

	float diffuseFactor = full ? 1 : dot(toLight, norm);

	if (lightColor.a == 0)
	{
		lightRadius *= norm.y;
		if (lightRadius < 0)
			lightRadius = 0;
	}

	float falloff = 1 - pow((dist / lightRadius), lightFalloff);
	diffuseFactor = pow(diffuseFactor, 1.f / (lightRadius / 256.f));
	diffuseFactor *= falloff;

	lightColor.a = 1;
	return lightColor * saturate(diffuseFactor);
}

float random(float2 p) { return frac(cos(dot(p, float2(23.14069263277926, 2.665144142690225))) * 12345.6789); }
float random(float p) { return frac(cos(dot(float2(p,p), float2(23.14069263277926, 2.665144142690225))) * 12345.6789); }

#endif