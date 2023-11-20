#include "hlslglobal.hlsl"

Texture2D tex : register(t2);
Texture2D mask : register(t3);

Texture2D depthbuffer : register(t5);
Texture2D WaterColor : register(t11);
Texture2D WaterDepth : register(t12);
float4 main(float4 position : SV_POSITION, float4 color : COLOR, float2 texCoord : TEXCOORD) : SV_TARGET
{
#if 1
	{
		float4 c = color;
		const float waterDepth = WaterDepth.Sample(SmpUnfiltered, position.xy).r;
		const float4 underWaterColor = UnderWaterColor;
		bool underWater = underWaterColor.a > 0;
		if ( (underWater && position.z < waterDepth) || (!underWater && position.z > waterDepth) )
		{
			float4 waterColor = underWater ? underWaterColor : WaterColor.Sample(SmpUnfiltered, position.xy);
			c.xyz = waterColor.xyz * 2 * ( (1 - waterColor.a) * 2);
		}
		float2 tex = texCoord - .5;
		float lenSq = tex.x * tex.x + tex.y * tex.y;
		c.a *= 1 - (lenSq * 4);

		//float2 depthcoord = float2(position.x / 1920 * 2, position.y / 1080 * 2);
		//if (depth > depthbuffer.Sample(SmpUnfiltered, depthcoord).r)
		//	c.a = 0;

		if (c.a <= 0)
		{
			discard;
			return 0;
		}
		/*c.a *= 4;
		c.a = saturate(c.a);
		c.a *= color.a;*/
		float a = c.a;

		//float2 roundedCoord;
		//roundedCoord.y = RoundNearest(texCoord.y, 1.0 / 4.0);
		//roundedCoord.x = RoundNearest(texCoord.x, 1.0 / 4.0);
		//c *= random(roundedCoord);

		c.a = a;
		return c;
	}
#else
	//return float4(1, 1, 1, .5);

	/*
	float4 position = PixelPosition;

	position.x = map(position.x, LightVolumeStart.x, LightVolumeStart.x + LightVolumeDimensions.x, 0, 1);
	position.y = map(position.y, LightVolumeStart.y, LightVolumeStart.y + LightVolumeDimensions.y, 0, 1);
	position.z = map(position.z, LightVolumeStart.z, LightVolumeStart.z + LightVolumeDimensions.z, 0, 1);

	float4 light = tex3.Sample(smplr, float3(position.z, position.x, position.y));

	// scale light up to 1
	float lightmax = max(max(light.x, light.y), light.z);
	float lightscale = 1 / lightmax;
	light *= lightscale;

	light.a = 1;*/
	
	float t = Time + TimeOffset * 10;

	float2 coord = texCoord;
	coord.y += t * .1;
	coord *= .25;
	float4 diffuse1 = tex.Sample(SmpUnfiltered, coord);
	//diffuse1.a = length(diffuse1.xyz) / 1.7320508075688772;
	diffuse1.a = diffuse1.x;

	coord = texCoord;
	coord.y += t * .15;
	coord.x += .25;
	coord *= .5;
	float4 diffuse2 = tex.Sample(SmpUnfiltered, coord);
	diffuse2.a = diffuse2.x;
	//diffuse2.a = length(diffuse2.xyz) / 1.7320508075688772;

	coord = texCoord;
	coord.y += t * .25;
	float4 diffuse3 = tex.Sample(SmpUnfiltered, coord);
	//diffuse3.a = length(diffuse3.xyz) / 1.7320508075688772;
	diffuse3.a = diffuse3.x;

	diffuse3 *= 4;

	float4 diffuse = diffuse1 * diffuse2 * diffuse3;
	float m = mask.Sample(SmpUnfiltered, texCoord).r;
	diffuse *= m;

	//diffuse.a = diffuse.x;

	diffuse *= color;
	clip(diffuse.a - .01);
	return pow(diffuse, .8);// * light;
#endif
}