#include "hlslglobal.hlsl"

StructuredBuffer<KWaterData> WaterBuffer : T_REGISTER(MAT_BUFFER_SLOT);

Texture2D tex : register(t0);
Texture2D lightmap : register(t1);

struct PSout
{
	float4 Water : SV_Target0;
	float4 WaterDepth : SV_Target1;
};

PSout main(
	float4 position : SV_POSITION,
	float4 fragpos : FRAGPOS,
	float2 texcoord : TEXCOORD,
	float3 norm : NORMAL,
	float2 lightcoord : LIGHTCOORD,
	float4 lightfragpos : LIGHTSPACE,
	float4 minmaxcoords : COORDCLAMP,
	float3 lightcolor : LIGHTCOLOR,
	uint bufferindex : BUFFERINDEX
)// : SV_TARGET
{
	float4 fog = GetFogValue(fragpos);
	if (fog.a == 1) 
	{
		//return fog;
		PSout o;
		o.Water = fog.a;
		o.Water.a = 1;
		o.WaterDepth.x = 1;
		o.WaterDepth.y = 0;
		return o;
	}

	float2 coord = texcoord;

	// pixelate slightly
	coord.x = float(int(coord.x * 256)) / 256;
	coord.y = float(int(coord.y * 256)) / 256;

	float2 uv = waterize(coord, 1.5, 1.5, 2) * 1024 - Time * 50;
	float3x3 m = float3x3(-2, -1, 2, 3, -2, 1, 1, 2, 2);

	float3 a = mul(float3(uv / 4e2, Time / 4), m);
	float3 b = mul(mul(a, m), .4);
	float3 c = mul(mul(b, m), .3);

	float4 causticColor =
		pow(min(min(
			length(.5 - frac(a)),
			length(.5 - frac(b))),
			length(.5 - frac(c))),
			9) * 45;

	b = mul(mul(a, m), .4);
	c = mul(mul(b, m), .3);
	float4 causticColor2 =
		pow(min(min(
			length(.5 - frac(a)),
			length(.5 - frac(b))),
			length(.5 - frac(c))),
			3) * 3;

	//float4 waterColor = float4(0, .05, .15, 1);
	//float4 waterColor = float4(.71, .51, .36, 3) / 3;
	//causticColor *= float4(.1, .4, .9, 1);
	//causticColor2 *= float4(.1, .7, .9, 1) / 2;

	// kastle
	//float4 waterColor = float4(.71, .51, .46, 2) / 2;
	//causticColor *= float4(.5, .5, .2, 1);
	//causticColor2 *= float4(.5, .5, .5, 1) / 2;
	
	//waterColor = float4(.5, 0, 0, 1);
	//causticColor *= float4(-.6, 0, 0, 1);
	//causticColor2 *= float4(-.5, 0, 0, 1);

	// aerowalk
	//waterColor = float4(.2, .7, .5, 1);
	//causticColor *= float4(0, .2, .5, 1);
	//causticColor2 *= float4(0, 1, .2, 1) / 2;

	KWaterData data = WaterBuffer[bufferindex];
	const float4 waterColor = data.GetWaterColor();
	causticColor *= data.GetCaustic1();
	causticColor2 *= data.GetCaustic2();
	const float maxDepth = data.GetMaxDepth();
	const float waterizeScale = data.GetWaterizeScale();

	float rand = random(causticColor.xy);
	float4 noised = (waterColor + causticColor + causticColor2) *
		map(rand, 0, 1, .9, 1);
	float4 lit = noised * (lightmap.Sample(SmpFiltered, lightcoord) + float4(lightcolor, 1));
	lit.a = .76;
	//lit.a = .64;
	//return lerp(lit, float4(fog.xyz, 1), fog.w);
	PSout o;
	o.Water = lerp(lit, float4(fog.xyz, 1), fog.w);
	o.Water.a = lit.a;	// odd alpha ignores edge transparency
	bool underWater = dot((fragpos - CamPosition), norm) > 0;
	o.WaterDepth.x = position.z;// * (underWater ? -1 : 1);
	o.WaterDepth.y = maxDepth;
	o.WaterDepth.z = waterizeScale;

	return o;
}