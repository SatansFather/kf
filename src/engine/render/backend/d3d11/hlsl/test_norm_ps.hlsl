#include "hlslglobal.hlsl"

Texture2D tex : register(t0);
Texture2D lightdepth : register(t1);
Texture2D lightmap : register(t2);

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
	return float4(abs(norm), 1);
	/*float3 n = norm;
	n.x = n.y;
	n.y = norm.x;
*/
	//float2 coord = texcoord;

	//coord.x = frac(texcoord.x);//map(coord.x, 0, 1, minmaxcoords.x, minmaxcoords.y);
	//coord.y = frac(texcoord.y);//map(coord.y, 0, 1, minmaxcoords.z, minmaxcoords.w);
	//
	//coord.x = map(coord.x, 0, 1, minmaxcoords.x, minmaxcoords.y);
	//coord.y = map(coord.y, 0, 1, minmaxcoords.z, minmaxcoords.w);

	// wrap for texture atlas
	//float x_min = minmaxcoords.x;
	//float x_max = minmaxcoords.y;
	//coord.x = (((coord.x - x_min) % (x_max - x_min)) + (x_max - x_min)) % (x_max - x_min) + x_min;
	//
	//float y_min = minmaxcoords.z;
	//float y_max = minmaxcoords.w;
	//coord.y = (((coord.y - y_min) % (y_max - y_min)) + (y_max - y_min)) % (y_max - y_min) + y_min;

	float2 coord = GetAtlasCoords(texcoord, float2(minmaxcoords.x, minmaxcoords.y), float2(minmaxcoords.z, minmaxcoords.w));

	//float d = (1024 - distance(viewpos, fragpos)) / 1024;

	//float adj = (int(abs(coord.x) * 4) % 2 == 0 && int(abs(coord.y) * 4) % 2 == 0) ? .9 : 1;

	/*float4 col =  float4(normalize(abs(norm)), 1) * clamp(d, .1 ,1);

	int adj = int(abs(texcoord.x) * 12) % 8;
	float a = float(adj) / 50.f;
	col += a;

	adj = int(abs(texcoord.y) * 12) % 8;
	a = float(adj) / 50.f;
	col -= a;


	return clamp(col, .15, 1);*/
	
	//return tex.Sample(filtered, lightcoord);
	float4 color = tex.Sample(SmpUnfiltered, coord);
	float c = color.a - 1;
	if (c < 0) clip(c);
	return float4(abs(norm), 1);
}