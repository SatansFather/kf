#include "hlslglobal.hlsl"

Texture2D tex : register(t0);
Texture2D lightdepth : register(t1);
Texture2D lightmap : register(t2);

#define MINSHADOW 0.2

float ShadowCalculation(float4 lightfragpos)
{
	float halfsq = .25;

	float3 frag = lightfragpos.xyz / lightfragpos.w;
	float pixeldepth = frag.z;

	if (pixeldepth > 1) return MINSHADOW;

	float2 projcoords = frag.xy;
	projcoords = projcoords * .5 + .5;
	projcoords.y = 1 - projcoords.y;

	float dist = distancesq(projcoords.xy, float2(.5, .5));
	if (dist > halfsq) return MINSHADOW;

	float falloff_start = .3;
	falloff_start *= falloff_start;
	
	float max_range = 1024;
	if (lightfragpos.z > max_range) return MINSHADOW;

	// circle fade
	float alpha = map(dist, falloff_start, halfsq, 1, MINSHADOW);
	
	// distance fade
	alpha *= map(lightfragpos.z, 0, max_range, 1, MINSHADOW);

	alpha = clamp( alpha, MINSHADOW, 1 );

	float closestdepth = lightdepth.Sample(SmpShadow, projcoords).r;
	if (closestdepth == 0) return MINSHADOW;
	
	//float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
	float bias = 0;

	return closestdepth == 0 ? MINSHADOW : (pixeldepth - bias) > closestdepth ? MINSHADOW : 1 * alpha;
}

float4 main(
	float4 position : SV_POSITION,
	float4 fragpos : FRAGPOS,
	float2 texcoord : TEXCOORD,
	float3 norm : NORMAL,
	float2 lightcoord : LIGHTCOORD,
	float4 lightfragpos : LIGHTSPACE,
	float4 minmaxcoords : COORDCLAMP
) : SV_TARGET
{
	float2 coord = GetAtlasCoords(texcoord, float2(minmaxcoords.x, minmaxcoords.y), float2(minmaxcoords.z, minmaxcoords.w));
	float4 color = tex.Sample(SmpUnfiltered, coord);
	float c = color.a - 1;
	if (c < 0)
	{
		clip(c);
		return color;
	}
	
	float3 normal = normalize(norm);

	float3 lightcolor = float3(.7, .65, 1);
	
	float shadow = ShadowCalculation(lightfragpos);
	lightcolor.x = map(shadow, 1, MINSHADOW, lightcolor.x, MINSHADOW);
	lightcolor.y = map(shadow, 1, MINSHADOW, lightcolor.y, MINSHADOW);
	lightcolor.z = map(shadow, 1, MINSHADOW, lightcolor.z, MINSHADOW);
	//float3 lightdir = viewpos.xyz - lightpos.xyz;

	//float3 V = normalize(viewpos.xyz - fragpos.xyz);
	//float3 R = reflect(normalize(lightdir), normal);
	//float strength = .6;
	//float spread = 5;
	//float spec = length(strength * pow(saturate(dot(R, V)), spread) * (pow(color, 4.5) / .2) * float3(1, 1, 1)) * length(lightcolor);
	
	//spec *= saturate( -dot( normalize(lightpos.xyz - viewpos.xyz), normalize(lightfacing.xyz) ) );

	color *= shadow;
	//color *= map(shadow, 0, 1, 0, 1);
	
	return color * float4(lightcolor, 1);
}