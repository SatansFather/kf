#include "hlslglobal.hlsl"

Texture2D front : register(t0);
Texture2D up : register(t1);
Texture2D down : register(t2);
Texture2D left : register(t3);
Texture2D right : register(t4);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	// rendering all directions to the a single texture so only one gpu->cpu transfer is needed

	uint w, h, m;
	front.GetDimensions(0, w, h, m);

	float x = texcoord.x;//position.x / w / 2;
	float y = texcoord.y;//position.y / h / 2;

	/*float scale = distance(float2(x, y), float2(.5, .5));
	scale = map(scale, 0, .5, 1, 0);
	scale = saturate(scale);*/
	float scale = 1;
	if (x < .25) // left
	{
		if (y < .25 || y > .75) return 0;

		x = map(x, 0, .25, .5, 1);
		y = map(y, .25, .75, 0, 1);
		return left.Sample(SmpUnfiltered, float2(x, y)) * scale;
	}
	else if (x < .75) // top front down
	{
		if (y < .25) // top
		{
			x = map(x, .25, .75, 0, 1);
			y = map(y, 0, .25, .5, 1);
			return up.Sample(SmpUnfiltered, float2(x, y)) * scale;
		}
		else if (y < .75) // front
		{
			x = map(x, .25, .75, 0, 1);
			y = map(y, .25, .75, 0, 1);
			return front.Sample(SmpUnfiltered, float2(x, y)) * scale;
		}
		else // down
		{
			x = map(x, .25, .75, 0, 1);
			y = map(y, .75, 1, 0, .5);
			return down.Sample(SmpUnfiltered, float2(x, y)) * scale;
		}
	}
	else // right
	{
		if (y < .25 || y > .75) return 0;

		x = map(x, .75, 1, 0, .5);
		y = map(y, .25, .75, 0, 1);
		return right.Sample(SmpUnfiltered, float2(x, y)) * scale;
	}

	return 0;
}