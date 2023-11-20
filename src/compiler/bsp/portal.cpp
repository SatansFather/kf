#include "portal.h"
#include "engine/utility/k_assert.h"
#include "engine/utility/kstring.h"

DPolygon KLeafPortal::CreatePoly()
{
	DPolygon p;
	for (DVec3& v : Vertices) p.Points.push_back(v);
	return p;
}

UPtr<KLeafPortal> KLeafPortal::SplitByPlane(DPlane plane, f64 epsilon /*= .001*/)
{
	TVector<DVec3> frontverts;
	TVector<DVec3> backverts;

	u32 vertcount = Vertices.size();

	const auto SplitEdge = [&](const DVec3& va, const DVec3& vb) -> void
	{
		// edge (a, b) straddles, output intersection point to both sides
		f64 dist = -1;
		DLineSegment seg(va, vb);
		bool intersect = plane.Intersects(seg, dist, 0);
		K_ASSERT(intersect && dist != -1, "spanning edge did not intersect plane");
		DVec3 i = seg.GetPoint(dist);

		KString err = "point did not lay on plane after splitting spanning edge\nsigned dist: ";
		err += KString(plane.SignedDistance(i), false);
		K_ASSERT(plane.ClassifyPoint(i) == EPointSide::On, err.CStr());

		DVec3 mid(i);

		frontverts.push_back(mid);
		backverts.push_back(mid);
	};

	// place existing points where they should be
	for (DVec3 v : Vertices)
	{
		EPointSide side = plane.ClassifyPoint(v);
		switch (side)
		{
			case EPointSide::Front:
			{
				frontverts.push_back(v);
				break;
			}
			case EPointSide::Behind:
			{
				backverts.push_back(v);
				break;
			}
			case EPointSide::On:
			{
				frontverts.push_back(v);
				backverts.push_back(v);
				break;
			}
		}
	}

	// check for split edges
	DVec3 a = Vertices[vertcount - 1];
	EPointSide aSide = plane.ClassifyPoint(a);
	for (u32 n = 0; n < vertcount; n++)
	{
		DVec3 b = Vertices[n];
		EPointSide bSide = plane.ClassifyPoint(b, epsilon);

		if (aSide == EPointSide::Front && bSide == EPointSide::Behind
			|| aSide == EPointSide::Behind && bSide == EPointSide::Front)
			SplitEdge(a, b); // adds split point to front and back

		// keep b as the starting point of the next edge
		a = b;
		aSide = bSide;
	}

	K_ASSERT(frontverts.size() >= 3, "face front vertices < 3 after split");
	K_ASSERT(backverts.size() >= 3, "face back vertices < 3 after split");

	// clear this portal and recreate it with the front of the split
	Vertices.clear();

	std::unique_ptr<KLeafPortal> out = std::make_unique<KLeafPortal>();
	*(out.get()) = *this; // copy

	for (const DVec3& v : frontverts) Vertices.push_back(v);
	InitFromVertices();

	for (const DVec3& v : backverts) out->Vertices.push_back(v);
	out->InitFromVertices();

	return out;
}

void KLeafPortal::InitFromVertices()
{
	SortVerticesCW();

	SurfaceArea = CreatePoly().Area();

	//Bounds.Reset();
	//for (DVec3& v : Vertices) Bounds.Update(v);
}

void KLeafPortal::SortVerticesCW()
{
	Center = DVec3(0);
	for (DVec3& v : Vertices) Center += v;
	Center /= Vertices.size();
	DVec3 base = Vertices[0] - Center;
	DVec3 norm = Plane.Normal;
	DVec3 cross = base ^ norm;

	auto AngleTo = [&](const DVec3& point) -> f64
	{
		f64 angle = base.AngleBetween(point);
		if ((cross | point) < 0) angle = (2.0 * PI<f64>()) - angle;
		return angle;
	};

	std::sort
	(
		Vertices.begin(),
		Vertices.end(),
		[&](const DVec3& a, const DVec3& b) -> bool
		{
			return AngleTo(a - Center) < AngleTo(b - Center);
		}
	);
}
