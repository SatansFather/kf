#include "hlslglobal.hlsl"

struct LineInfo
{
	float3 PointA;
	float3 PointB;
	float3 Color;
	float Thickness;
};

StructuredBuffer<LineInfo> LineBuffer : register(t0);

struct vs_out
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

vs_out main(uint id : SV_VERTEXID)
{
	const uint indicesPerLine	= 16;
	const uint lineIndex		= id / indicesPerLine; 
	const uint pointInLine		= id % indicesPerLine;
	const uint lineQuadIndex	= pointInLine / 4;
	const uint pointInQuad		= pointInLine % 4;

	float3 pointA = LineBuffer[lineIndex].PointA;
	float3 pointB = LineBuffer[lineIndex].PointB;
	float3 color = LineBuffer[lineIndex].Color;
	float thickness = LineBuffer[lineIndex].Thickness;

	const float3 away = pointA.z == pointB.z ? float3(1, 0, 0) : float3(0, 0, 1);
	const float3 lineDir = pointB - pointA;
	const float3 lineCrossA = cross(lineDir, away);
	const float3 lineCrossB = cross(lineDir, lineCrossA);

	bool evenQuad = (lineQuadIndex % 2) == 0;
	const float3 lineSide = evenQuad ? lineCrossA : lineCrossB
		* (((pointInQuad % 2) == 0) ? 1 : -1);
	const float3 lineOut = (evenQuad ? lineCrossA : lineCrossB)
		* (lineQuadIndex > 1 ? -1 : 1);

	float3 offPoint = pointInQuad > 1 ? pointB : pointA;
	float3 pointPos = offPoint + (lineOut * thickness) + (lineSide * thickness);

	vs_out o;
	o.position = mul(float4(pointPos.xyz, 1), ModelViewProjection);
	o.color = float4(1, 1, 1, 1);

	return o;
}