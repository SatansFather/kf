#if !_SERVER

#include "surface.h"
#include "compiler/brush/brush_face.h"
#include "compiler/brush/brush.h"
#include "compiler/compiler.h"
#include "engine/system/terminal/terminal.h"
#include "bsp_tree.h"

KMapSurface::~KMapSurface() {}

void KMapSurface::InitNewSurface()
{
	Bounds.Reset();
	SurfaceArea = 0;
	for (UPtr<KBrushFace>& face : Faces)
	{
		Bounds.Update(face->Bounds);
		SurfaceArea += face->SurfaceArea;
	}

	// no faces means portal only, dont let portals split
	if (Faces.size() == 0 && !bIsGridPlane) bHasSplit = true;
}

void KMapSurface::SplitByPlane(const DPlane& plane, bool self, UPtr<KMapSurface>& front, UPtr<KMapSurface>& back)
{
	TVector<UPtr<KBrushFace>> frontfaces, backfaces;
	//TVector<UPtr<KBrushFace>> frontportals, backportals;

	// self flag is passed to save a matching-plane check here
	// if self, just separate front and back
	if (self)
	{
		for (UPtr<KBrushFace>& face : Faces)
		{
			if ((face->GetPlane().Normal | plane.Normal) > 0)
				frontfaces.push_back(std::move(face));
			else
				backfaces.push_back(std::move(face));
		}

		/*for (UPtr<KBrushFace>& portal : Portals)
		{
			// just add portal to front, it will be copied and flipped later
			frontportals.push_back(std::move(portal));
		}*/
	}
	else // not self, split faces
	{
		const auto FaceSplit = [&](
			UPtr<KBrushFace>& face, 
			TVector<UPtr<KBrushFace>>& frontvec, 
			TVector<UPtr<KBrushFace>>& backvec) 
			-> void
		{
			switch (face->GetPolygon().ClassifyToPlane(plane, .001))
			{
				case EPolyClassification::Front:
				{
					frontvec.push_back(std::move(face));
					break;
				}
				case EPolyClassification::Behind:
				{
					backvec.push_back(std::move(face));
					break;
				}
				case EPolyClassification::Spanning:
				{
					UPtr<KBrushFace> newface = face->SplitByPlane(plane, .001);
					Tree->IncrementFaceCount();

					frontvec.push_back(std::move(face));
					backvec.push_back(std::move(newface));
					break;
				}
				case EPolyClassification::Coplanar:
				{
					// weird but it happens with tiny-ass (or krazy-ass) faces
					// front or back based on plane normal
					if (face->Plane.Normal | Plane.Normal > 0)
						frontvec.push_back(std::move(face));
					else
						backvec.push_back(std::move(face));
					break;
				}
			}
		};

		for (UPtr<KBrushFace>& face : Faces)
			FaceSplit(face, frontfaces, backfaces);

#if DUMBPORTALS
		for (UPtr<KBrushFace>& portal : Portals)
			FaceSplit(portal, frontportals, backportals);		
#endif
	}
	
#if !DUMBPORTALS
	// clear faces so we can copy the rest of this surface to the splits
	Faces.clear();
	if (!frontfaces.empty())
	{
		front = CopySurface();
		if (!frontfaces.empty()) front->Faces = std::move(frontfaces);
		front->InitNewSurface();
	}
	if (!backfaces.empty())
	{
		back = CopySurface();
		if (!backfaces.empty()) back->Faces = std::move(backfaces);
		back->InitNewSurface();
	}
#else
	Faces.clear();
	Portals.clear();

	if (!frontfaces.empty() || !frontportals.empty())
	{
		front = CopySurface();

		if (!frontfaces.empty()) front->Faces = std::move(frontfaces);
		if (!frontportals.empty()) front->Portals = std::move(frontportals);

		front->InitNewSurface();
	}

	if (!backfaces.empty() || !backportals.empty())
	{
		back = CopySurface();

		if (!backfaces.empty()) back->Faces = std::move(backfaces);
		if (!backportals.empty()) back->Portals = std::move(backportals);

		back->InitNewSurface();
	}
#endif
}

void KMapSurface::PortalizeSurface()
{
#if !DUMBPORTALS
	return;
#endif

	// go to PORTAL GENERATION section of mr gamemaker page

	// create a large poly over the entire area of the surface
	DBoundingBox bounds = KMapCompiler::Get().MapBounds;
	bounds.Min -= DVec3(1,1,1);
	bounds.Max += DVec3(1,1,1);
	TVector<DLineSegment> edges;
	bounds.GetEdges(edges);

	TVector<DVec3> verts;

	// intersect each plane with the surface plane to get lines
	for (const DLineSegment& edge : edges)
	{
		if (Plane.ClassifyPoint(edge.a) == EPointSide::On 
		 && Plane.ClassifyPoint(edge.b) == EPointSide::On)
		{
			// if the line sits on the plane, add both points
			if (!VectorContains(verts, edge.a)) verts.push_back(edge.a);
			if (!VectorContains(verts, edge.b)) verts.push_back(edge.b);
		}
		else
		{
			// find intersection point
			f64 dist = -1;
			if (Plane.Intersects(edge, dist) && dist >= -.001 && dist <= 1.001)
			{
				dist = std::clamp(dist, 0.0, 1.0);
				DVec3 p = edge.GetPoint(dist);
				if (!VectorContains(verts, p)) verts.push_back(p);
			}
		}
	}

	K_ASSERT(verts.size() > 2, "portal did not form a polygon on creation");

	TVector<DPlane> planes;
	{
		UPtr<KBrushFace> portal = std::make_unique<KBrushFace>();
		portal->bIsPortal = true;
		portal->Plane = Plane;

		for (DVec3& v : verts) portal->Vertices.push_back(KBrushVertex(v));
		portal->InitFromVertices();

		// split the portal face by the real surface faces, remove the overlaps

		// first create planes from all face edges
		for (UPtr<KBrushFace>& face : Faces)
		{
			for (u32 i = 0; i < face->Poly.NumEdges(); i++)
			{
				DLineSegment edge = face->Poly.Edge(i);

				DVec3 norm = (edge.b - edge.a).GetNormalized() ^ Plane.Normal;
				norm.Normalize();

				DPlane p(edge.a, norm);

				bool contained = false;
				for (const DPlane& plane : planes)
				{
					bool equal = p.Equals(face->GetPlane(), .0001, .001);
					bool inverse = p.EqualsInverse(face->GetPlane(), .0001, .001);

					if (equal || inverse)
					{
						contained = true;
						break;
					}
				}
				if (!contained)
				{
					planes.push_back(p);
				}
			}
		}

		//Portals.push_back(std::move(portal));
	}

	// split the portal face by all the planes
	/*for (u32 i = 0; i < Portals.size(); i++)
	{
		KBrushFace* portal = Portals[i].get();
		for (const DPlane& plane : planes)
		{
			if (portal->Poly.ClassifyToPlane(plane, .001) == EPolyClassification::Spanning)
			{
				UPtr<KBrushFace> fragment = portal->SplitByPlane(plane, .001);
				Portals.push_back(std::move(fragment));
			}
		}
	}

	// remove any portal faces that match an existing face
	for (i32 i = Portals.size() - 1; i >= 0; i--)
	{
		UPtr<KBrushFace>& portal = Portals[i];

		for (UPtr<KBrushFace>& face : Faces)
		{
			if (!portal->Bounds.Overlaps(face->Bounds)) continue;
			if (portal->Vertices.size() != face->Vertices.size()) continue;

			// compare verts
			u32 matches = 0;
			for (KBrushVertex& v1 : portal->Vertices)
			{
				for (KBrushVertex& v2 : face->Vertices)
				{
					if (v1.Point.Equals(v2.Point, .001))
					{
						matches++;
						break;
					}
				}
			}
			if (matches == portal->Vertices.size())
			{
				// if the vertices match, remove this portal
				VectorRemoveAt(Portals, i);
				break;
			}
		}
	}*/

	// at this point the remaining portal faces should fill the gaps in the surface
	// and nothing else
}

UPtr<KMapSurface> KMapSurface::CopySurface()
{
	// faces and portals are not copied

	UPtr<KMapSurface> out = std::make_unique<KMapSurface>();

	out->Bounds = Bounds;
	out->Plane = Plane;
	out->bHasSplit = bHasSplit;
	out->SurfaceArea = SurfaceArea;
	out->Tree = Tree;

	return out;
}

#endif