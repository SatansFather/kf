#include "hlslglobal.hlsl"

Texture2D Opaque : register(t0);
Texture2D WaterColor : register(t1);
Texture2D WaterDepth : register(t2);
Texture2D SceneDepth : register(t3);
Texture2D Transparency : register(t4);

float4 GetSample(Texture2D tex, float2 coord, bool underWater, bool waterTex = false)
{
	if (!underWater || waterTex) return tex.Sample(SmpUnfiltered, coord);

	const float kernel = 4;
	const float weight = 10;
	const float pixelY = 1.0 / Resolution.y * (Resolution.y / 1080);
	const float pixelX = 1.0 / Resolution.x * (Resolution.y / 1080);

	float3 accumulation = 0;
	float3 weightsum = 0;
	float3 sum = 0;
	float alpha = 0;

	for (float i = -kernel; i <= kernel; i++)
	{
		float4 samp = tex.Sample(SmpUnfiltered,				// prevent sampling other side
			saturate(coord + float2(i * pixelX, i * pixelY)) * .999);

		accumulation += samp.xyz * weight;
		alpha = max(alpha, samp.a);
		weightsum += weight;
	}

	sum = accumulation / weightsum;

	return float4(sum, alpha);// / weightsum.x);
}

float4 GetSampleWithWater(float2 coord)
{
	// all samples will be blurred if under water

	float4 depthSample = WaterDepth.Sample(SmpUnfiltered, coord);

	float waterDepth = depthSample.x;
	float waterizeScale = depthSample.z;
	float sceneDepth = SceneDepth.Sample(SmpUnfiltered, coord).r;

	const float4 waterColor = UnderWaterColor;
	const bool underWater = waterColor.a > 0;
	const bool behindWater = underWater;//waterDepth < 0; // likely underwater, but this means we are behind the water plane

	waterDepth = abs(waterDepth); // negative value only tells us front/behind

	const float depthDist = sceneDepth - waterDepth;
	const float floorDist = 2.f * 5 * 100000 / (100000 + 5 - (2.f * sceneDepth - 1.f) * (100000 - 5));
	const float waterDist = 2.f * 5 * 100000 / (100000 + 5 - (2.f * waterDepth - 1.f) * (100000 - 5));

	float4 diffuse;
	float4 water = GetSample(WaterColor, coord, underWater, true);

	const float staticDistance = 64;
	//const float fullDepth = 128 * (1 - water.a);
	const float fullDepth = depthSample.g * (1 - water.a);

	waterDepth = behindWater ? waterDist : floorDist - waterDist;

	if (depthDist > 0)
	{
		// water is rendered over top of the opaque surface

		//if ((round(water.a * 100) % 2) == 0) // if alpha is even, we want depth fade
		if (fullDepth > 0)
			water.a *= map(saturate(waterDepth / fullDepth), 0, 1, 0, 1);
		//else
		//	fullDepth = 0;

		float2 edgeScale = 1 - abs(coord - .5) * 2;
		float2 watercoord = waterize(coord, 1, 1, 1, saturate(waterDepth / fullDepth * (underWater ? .2 : 1)) * saturate(staticDistance / waterDist) * waterizeScale);
		watercoord.x = lerp(coord.x, watercoord.x, saturate(edgeScale.x * 10));
		watercoord.y = lerp(coord.y, watercoord.y, saturate(edgeScale.y * 10));

		diffuse = GetSample(Opaque, watercoord, underWater);

		if (underWater)
		{
			float dist = waterDist;
			water = lerp(water, UnderWaterColor, saturate(dist / 256) * UnderWaterColor.a);
		}
		else
		{
			float dist = abs(waterDist - floorDist);
			diffuse = lerp(diffuse, water, saturate(dist / 64) * water.a);
		}
	}
	else
	{
		if (underWater)
		{
			float2 edgeScale = 1 - abs(coord - .5) * 2;
			float2 watercoord = waterize(coord, 1, 1, 1, saturate(floorDist / 64) * .25);
			watercoord.x = lerp(coord.x, watercoord.x, saturate(edgeScale.x * 10));
			watercoord.y = lerp(coord.y, watercoord.y, saturate(edgeScale.y * 10));
			diffuse = GetSample(Opaque, watercoord, underWater);
		}
		else
		{
			diffuse = GetSample(Opaque, coord, underWater);
		}
	}

	if (underWater)
	{
		float dist = min(waterDist, floorDist);
		diffuse = lerp(diffuse, UnderWaterColor, saturate(dist / 64) * UnderWaterColor.a);
		water = lerp(water, UnderWaterColor, saturate(dist / 64) * UnderWaterColor.a);
	}

	return lerp(diffuse, water, water.a);
};

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float2 coord = texcoord;
	if (abs(CamPitch) >= 90) coord = -coord; 
	float4 trans = Transparency.Sample(SmpFiltered, coord);
	float4 diffuse = GetSampleWithWater(coord) + trans;
	//return trans;
	//diffuse.xyz += trans.xyz * trans.a;
	diffuse.xyz = lerp(diffuse.xyz, trans.xyz, trans.a);
	
	return diffuse;
}