#if !_SERVER

#include "lightning_bolt.h"
#include "engine/math/line_segment.h"
#include "engine/utility/random.h"

KEntity_LightningBolt::KEntity_LightningBolt()
{

}

void KEntity_LightningBolt::Create(const GVec3& start, const GVec3& end, f32 thickness /*= 1*/)
{
	u8 iterations = 6;

	TVector<KBoltSegment> segments;
	TVector<KBoltSegment> lastSegments;

	segments.push_back({ GLineSegment(start, end), thickness });

	GVec3 dir = (end - start).GetNormalized();

	f32 offset = 128;

	for (u8 i = 0; i < iterations; i++)
	{
		lastSegments = segments;
		segments.clear();
		segments.reserve(lastSegments.size() * 2);

		for (KBoltSegment& seg : lastSegments)
		{
			GVec3 midpoint = GVec3::Lerp(seg.Line.a, seg.Line.b, .5);
			midpoint += dir.Perpendicular(GVec3::RandomDir()) * RandFloat(-offset, offset);

			segments.push_back({ GLineSegment(seg.Line.a, midpoint), seg.Thickness });
			segments.push_back({ GLineSegment(midpoint, seg.Line.b), seg.Thickness });

			if (seg.Thickness > (.35 / 16.f) && Random() < (175.f / seg.Thickness))
			{
				GVec3 split = GVec3(midpoint - seg.Line.a).GetRotated(RandRange(-25, 25), dir) * .7 + midpoint;
				segments.push_back({ GLineSegment(midpoint, split), seg.Thickness * .5f });
			}
		}
		offset *= .5;
	}
}
/*

KBufferUpdateResult KEntity_LightningBolt::UpdateBuffers(KLightningBolt& entry)
{
	
	
	return false;
}*/

#endif
