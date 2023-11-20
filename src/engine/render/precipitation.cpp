#include "precipitation.h"
#include "engine/collision/trace.h"

KPrecipitationVolume::~KPrecipitationVolume() {}

void KPrecipitationVolume::SetupVolume(const TVector<GPolygon>& polygons, u32 id)
{
	ID = id;
	
	Bounds.Reset();
	for (const GPolygon& poly : polygons)
	  for (const GVec3& v : poly.Points)
		Bounds.Update(v);

	// find particle spawn positions
	GFlt xSpan = abs(Bounds.Max.x - Bounds.Min.x);
	GFlt ySpan = abs(Bounds.Max.y - Bounds.Min.y);
	u32 xCount = xSpan / Density;
	u32 yCount = ySpan / Density;
	u32 xOffset = u32(xSpan) % u32(Density);
	u32 yOffset = u32(ySpan) % u32(Density);

	if (xOffset == 0) xOffset = Density / 2;
	if (yOffset == 0) yOffset = Density / 2;

	SpawnPoints.resize(xCount);
	StopPoints.resize(xCount);
	for (TVector<GVec3>& v : SpawnPoints) v.resize(yCount);
	for (TVector<GVec3>& v : StopPoints) v.resize(yCount);

	for (u32 x = 0; x < xCount; x++)
	{
		GFlt xPos = Bounds.Min.x + xOffset + (Density * x);
		for (u32 y = 0; y < yCount; y++)
		{
			GFlt yPos = Bounds.Min.y + yOffset + (Density * y);
			SpawnPoints[x][y] = GVec3(xPos, yPos, Bounds.Max.z);
			// default stop point if no collision
			StopPoints[x][y] = GVec3(xPos, yPos, Bounds.Min.z);
		}
	}

	// trace down to find where particles stop
	for (u32 x = 0; x < xCount; x++)
	{
		for (u32 y = 0; y < yCount; y++)
		{
			GHitResult hit;
			TraceLine(GLineSegment(SpawnPoints[x][y], StopPoints[x][y]), hit);
			StopPoints[x][y] = hit.Point;
		}
	}
}
