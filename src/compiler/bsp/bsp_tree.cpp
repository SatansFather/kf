#include "bsp_tree.h"
#include "surface.h"
#include "bsp_node.h"
#include "../brush/brush_face.h"
#include "engine/utility/thread_pool.h"
#include "engine/run/k_main.h"
#include "../compiler.h"
#include "engine/system/terminal/terminal.h"
#include "engine/system/terminal/terminal_progress.h"
#include <future>
#include "portal.h"

static KTerminalProgressBar PartitionProgress;

KBspTree::KBspTree(TVector<UPtr<KBrushFace>>& faces, bool optimal)
{
	//SYSLOG("Building BSP Tree")
	KTimePoint start = KTime::Now();

	bOptimal = optimal;
	TotalFaces = faces.size();

	BuildSurfaces(faces);
	BuildTree();
	if (!optimal) BuildPortals();

	f64 time = KTime::Since(start);

	/*SYSLOG("BSP Generation took " << std::setprecision(3) << time << " seconds");
	SYSLOG("Max Depth: " << MaxDepth);
	SYSLOG("Min Depth: " << MinDepth);
	SYSLOG("Avg Depth: " << TotalDepth / LeafCount);
	SYSLOG("Leaf Count: " << LeafCount);*/
}

KBspTree::~KBspTree() {}

KBspNode* KBspTree::NodeFromPoint(const DVec3& point) const
{
	KBspNode* node = HeadNode.get();
	while (node && !node->bIsLeaf)
	{
		if (node->SplitPlane.ClassifyPoint(point) == EPointSide::Front)
		{
			node = node->Front.get();
		}
		else
		{
			node = node->Back.get();
		}
	}
	return node;
}

void KBspTree::BuildSurfaces(TVector<UPtr<KBrushFace>>& faces)
{
#if 1
	Surfaces.clear();

	// a surface is a collection of faces that sit on the same plane - front or back
	std::mutex countmut;
	KHashedPlaneMap planeFaces;

	KTerminalProgressBar faceprog("Creating Surfaces");
	u32 count = 0;
	
	// build grid surfaces first
	// bsp will be subdivided into a grid to prevent artifacts on complex maps
	DBoundingBox bounds;
	for (UPtr<KBrushFace>& face : faces) 
	{
		for (const KBrushVertex& v : face->GetVertices()) 
		{
			bounds.Update(v.Point);
			count++;
		}
	}

	// grid planes will be added to the plane map
	//planeFaces = BuildGridSurfaces(bounds, count);
	count = 0;

	const auto buildPlanes = [&](u32 start, u32 end) -> void
	{
		for (u32 i = start; i < end; i++)
		{
			UPtr<KBrushFace>& face = faces[i];
			//if (face->bIsCollisionPass) continue;

			KHashedPlane arr;
			face->Plane.ToByteArray(arr);
			if (!planeFaces.contains(arr))
				face->Plane.Inverted().ToByteArray(arr);

			planeFaces[arr].Faces.push_back(std::move(face));

			countmut.lock();
			count++;
			faceprog.UpdateProgress(count, faces.size());
			countmut.unlock();
		}
	};

	KThreadPool::Iterate(buildPlanes, 1, faces.size());

	faceprog.Finish();

	KTerminalProgressBar surfprog("Finalizing Surfaces");
	count = 0;
	for (auto& kv : planeFaces)
	{
		count++;
		
		auto surf = std::make_unique<KMapSurface>();
		surf->Tree = this;
		surf->bIsGridPlane = kv.second.bIsGridPlane;
		if (kv.second.Faces.size() > 0)
		{
			surf->Plane = kv.second.Faces[0]->GetPlane();
			surf->Faces = std::move(kv.second.Faces);
			surf->InitNewSurface();
		}
		else
		{
			surf->Plane = kv.second.Plane;
		}

		// create initial portals now while we have all the planes together
		Portals.push_back(std::move(PortalFromPlane(surf->Plane)));

		Surfaces.push_back(std::move(surf));

		surfprog.UpdateProgress(count, planeFaces.size());
	}
	surfprog.Finish();
#else

	Surfaces.clear();

	// a surface is a collection of faces that sit on the same plane - front or back
	std::mutex surfmut;
	std::mutex countmut;
	TVector<DPlane> planes;
	TMap<u32, TVector<UPtr<KBrushFace>>> planeFaces;

	KTerminalProgressBar faceprog("Creating Surfaces");
	u32 count = 0;

	const auto buildPlanes = [&](u32 start, u32 end) -> void
	{
		for (u32 i = start; i < end; i++)
		{
			UPtr<KBrushFace>& face = faces[i];

			bool contained = false;
			for (u32 i = 0; i < planes.size(); i++)
			{
				const DPlane& p = planes[i];

				K_ASSERT(p.Normal.Length() > .999, "normal length less than 1");
				K_ASSERT(p.Normal.Length() < 1.001, "normal length greater than 1");

				bool equal = p.Equals(face->GetPlane(), .001, .001);
				bool inverse = p.EqualsInverse(face->GetPlane(), .001, .001);

				if (equal || inverse)
				{
					// add face to an existing surface
					contained = true;
					surfmut.lock();
					face->Plane = inverse ? p.Inverted() : p;
					planeFaces[i].push_back(std::move(face));
					surfmut.unlock();
					break;
				}
			}
			if (!contained)
			{
				// no plane found, create a new entry and add the current face
				surfmut.lock();
				planes.push_back(face->GetPlane());
				planeFaces[planes.size() - 1].push_back(std::move(face));
				surfmut.unlock();
			}
		
			countmut.lock();
			count++;
			faceprog.UpdateProgress(count, faces.size());
			countmut.unlock();
		}
	};

	KThreadPool::Iterate(buildPlanes, 0, faces.size());

	faceprog.Finish();

	KTerminalProgressBar surfprog("Finalizing Surfaces");
	count = 0;
	for (auto& kv : planeFaces)
	{
		auto surf = std::make_unique<KMapSurface>();
		surf->Plane = planes[kv.first];
		surf->Faces = std::move(kv.second);
		surf->Tree = this;
		surf->InitNewSurface();
		
		// create initial portals now while we have all the planes together
		Portals.push_back(std::move(PortalFromPlane(surf->Plane)));

		Surfaces.push_back(std::move(surf));

		count++;
		surfprog.UpdateProgress(count, planeFaces.size());
	}
	surfprog.Finish();
#endif
}

KHashedPlaneMap KBspTree::BuildGridSurfaces(const DBoundingBox& bounds, u32 faceCount)
{
	// TODO sort this so that centered planes come first for a more balanced tree
	
	KHashedPlaneMap planes;	// for return value
	TVector<DPlane> iterPlanes; // for surface creation
	const u32 dimension = 64;

	// create grid planes
	for (i32 i = 0; i < 3; i++)
	{
		DVec3 norm;
		norm[i] = 1;
		DPlane plane;
		plane.Normal = norm;
		for (f64 comp = bounds.Min[i]; comp < bounds.Max[i] + dimension; comp += dimension)
		{
			plane.D = comp;

			// drop this plane if it is outside bounds
			if (plane.ClassifyPoint(bounds.Min) == plane.ClassifyPoint(bounds.Max)) 
				continue;

			iterPlanes.push_back(plane);

			// add to map
			KHashedPlane hashed;
			plane.ToByteArray(hashed);
			planes[hashed].bIsGridPlane = true;
			planes[hashed].Plane = plane;
		}
	}

	// create surfaces from planes
	/*for (const DPlane& plane : iterPlanes)
	{
		auto surf = std::make_unique<KMapSurface>();
		surf->Plane = plane;
		surf->Tree = this;
		surf->bIsGridPlane = true;

		// create initial portals now while we have all the planes together
		Portals.push_back(std::move(PortalFromPlane(surf->Plane)));

		Surfaces.push_back(std::move(surf));
	}*/


	return planes;
}

void KBspTree::BuildTree()
{
	HeadNode.reset();
	HeadNode = std::make_unique<KBspNode>();

	PartitionProgress = KTerminalProgressBar("Partitioning Surfaces");

	// split all surfaces by the grid planes
	/*for (i32 i = 0; i < Surfaces.size(); i++)
	{
		UPtr<KMapSurface>& splitter = Surfaces[i];
		if (splitter->bIsGridPlane)
		{

			TVector<UPtr<KMapSurface>> newSurfaces;

			for (UPtr<KMapSurface>& surface : Surfaces)
			{
				UPtr<KMapSurface> front, back;

				surface->SplitByPlane(
					splitter->Plane, 
					splitter->Plane.Equals(surface->Plane),
					front, back);


				newSurfaces.push_back(std::move(front));
				newSurfaces.push_back(std::move(back));
			}
			VectorRemoveAt(Surfaces, i);
			i -= 1;
			
			for (UPtr<KMapSurface>& surf : newSurfaces)
				Surfaces.push_back(std::move(surf));
		}
	}*/

	// recursively divides the surfaces until all faces are grouped into convex subspaces
	PartitionSurfaces(&Surfaces, HeadNode.get(), 0);

	PartitionProgress.Finish();
}

void KBspTree::BuildPortals()
{
	GeneratePortals();
	//RemoveInvalidPortals(Portals);
}

void KBspTree::PartitionSurfaces(TVector<UPtr<KMapSurface>>* surflist, KBspNode* node, u32 depth)
{
	node->Bounds.Reset();
	for (auto& surf : *surflist) node->Bounds.Update(surf->Bounds);

	TVector<UPtr<KMapSurface>>& surfaces = *surflist;
	KMapSurface* splitter = SelectSplit(surfaces);
	if (!splitter)
	{
		// if no splitter was found, every surface has already been a splitter
		// this is a leaf and the remaining faces form a convex shape
		node->bIsLeaf = true;

		u32 facecount = 0;

		// add all surface faces to this leaf
		for (UPtr<KMapSurface>& surf : surfaces)
		{
			for (UPtr<KBrushFace>& face : surf->Faces)
			{
				node->LeafFaces.push_back(std::move(face));
				facecount++;
			}
		}

		LeafedFaces += facecount;
		PartitionProgress.UpdateProgress(LeafedFaces, TotalFaces);

		if (!node->IsSolidLeaf())
		{
			if (depth > MaxDepth) MaxDepth = depth;
			if (depth < MinDepth) MinDepth = depth;
			TotalDepth += depth;
			LeafCount++;
		}

		// these surfaces are no longer needed
		surfaces.clear();
		return;
	}

	// not a leaf, set up another partition
	node->SplitPlane = splitter->Plane;

	node->Front = std::make_unique<KBspNode>();
	node->Front->Parent = node;

	node->Back = std::make_unique<KBspNode>();
	node->Back->Parent = node;

	// this value will be copied into any splits of the splitter
	splitter->bHasSplit = true;

	TVector<UPtr<KMapSurface>> frontsurfs;
	TVector<UPtr<KMapSurface>> backsurfs;

	// split surfaces along plane, splitter faces if needed
	for (i32 i = surfaces.size() - 1; i >= 0; i--)
	{
		UPtr<KMapSurface> front, back;

		// divide surface by the plane, storing the front and back
		surfaces[i]->SplitByPlane(node->SplitPlane, surfaces[i].get() == splitter, front, back);

		if (surfaces[i].get() == splitter) splitter = nullptr;
		VectorRemoveAt(surfaces, i); // no longer needed after being split

		// if a surface wasnt created, it had no faces or portals
		if (front.get())
		{
			front->InitNewSurface();
			frontsurfs.push_back(std::move(front));
		}
		if (back.get())
		{
			back->InitNewSurface();
			backsurfs.push_back(std::move(back));
		}
	}

	const auto part = [&]() -> void { PartitionSurfaces(&frontsurfs, node->Front.get(), depth + 1); };
	//std::future<void> fut = std::async(std::launch::async, part);
	part();

	PartitionSurfaces(&backsurfs, node->Back.get(), depth + 1);
}

KMapSurface* KBspTree::SelectSplit(TVector<UPtr<KMapSurface>>& surfaces)
{
	TVector<KMapSurface*> valid;

	//// check grid planes first
	for (UPtr<KMapSurface>& surf : surfaces)
	  if (surf->bIsGridPlane && !surf->bHasSplit)
		return surf.get();

	// grid planes come first
	for (UPtr<KMapSurface>& surf : surfaces)
	  if (!surf->bHasSplit)
		if ((surf->Plane.IsAxial()))
		  if (i32(std::round(surf->Plane.D)) % 16 == 0)
			valid.push_back(surf.get());

	// ignore 16 grid alignment
	if (valid.size() == 0)
 	  for (UPtr<KMapSurface>& surf : surfaces)
		if (!surf->bHasSplit)
		  if (surf->Plane.IsAxial())
			valid.push_back(surf.get());

	// no more axis aligned, just grab the rest
	if (valid.size() == 0)
	  for (UPtr<KMapSurface>& surf : surfaces)
		if (!surf->bHasSplit)
		  valid.push_back(surf.get());

	return valid.size() > 0 ? valid[0] : nullptr;
}

void KBspTree::GeneratePortals()
{
	TVector<UPtr<KLeafPortal>> fragments;
	TVector<UPtr<KLeafPortal>> finals;
	std::mutex fragmutex;

	std::function<void(UPtr<KLeafPortal>, KBspNode*, TVector<UPtr<KLeafPortal>>&)> ClipPortal =
		[&](UPtr<KLeafPortal> portal, KBspNode* node, TVector<UPtr<KLeafPortal>>& frags)
	{
		if (!node)
		{
			portal.reset();
			return;
		}

		if (node->IsLeaf())
		{
			if (!node->IsSolidLeaf() && portal->SurfaceArea > .25)
			{
				fragmutex.lock();
				portal->LeafNodes.push_back(node);
				frags.push_back(std::move(portal));
				fragmutex.unlock();
			}
			return;
		}

		switch (portal->CreatePoly().ClassifyToPlane(node->SplitPlane, .001))
		{
			case EPolyClassification::Front:
			{
				ClipPortal(std::move(portal), node->Front.get(), frags);
				break;
			}
			case EPolyClassification::Behind:
			{
				ClipPortal(std::move(portal), node->Back.get(), frags);
				break;
			}
			case EPolyClassification::Spanning:
			{
				UPtr<KLeafPortal> backportal = portal->SplitByPlane(node->SplitPlane, .001);
				ClipPortal(std::move(portal), node->Front.get(), frags);
				ClipPortal(std::move(backportal), node->Back.get(), frags);
				break;
			}
			case EPolyClassification::Coplanar:
			{
				TVector<UPtr<KLeafPortal>> ports;
				ClipPortal(std::move(portal), node->Front.get(), ports);

				for (UPtr<KLeafPortal>& port : ports)
				{
					TVector<UPtr<KLeafPortal>> backports;
					ClipPortal(std::move(port), node->Back.get(), backports);

					RemoveInvalidPortals(backports);

					for (UPtr<KLeafPortal>& bp : backports)
					{
						fragmutex.lock();
						finals.push_back(std::move(bp));
						fragmutex.unlock();
					}
				}

				break;
			}
		}
	};

	// clip our portals against the new tree
	u32 count = 0;
	KTerminalProgressBar progress("Portalizing Tree");
	std::mutex countmutex;

	const auto ClipPortals = [&](u32 start, u32 end) -> void
	{
		for (u32 i = start; i < end; i++)
		{
			ClipPortal(std::move(Portals[i]), HeadNode.get(), fragments);
			countmutex.lock();
			count++;
			progress.UpdateProgress(count, Portals.size());
			countmutex.unlock();
		}
	};

	KThreadPool::Iterate(ClipPortals, KApplication::GetCoreCount(), Portals.size());
	progress.Finish();

	Portals = std::move(finals);
}

static std::mutex LeafPortalAddMutex;
void KBspTree::RemoveInvalidPortals(TVector<UPtr<KLeafPortal>>& ports)
{
	for (i32 i = ports.size() - 1; i >= 0; i--)
	{
		// tiny-ass portals are stupid and probably an unintended leak
		if (ports[i]->SurfaceArea < .2)
		{
			VectorRemoveAt(ports, i);
			continue;
		}

		// portals need to connect two leaves
		if (ports[i]->LeafNodes.size() != 2)
		{
			VectorRemoveAt(ports, i);
			continue;
		}

		// a portal shouldnt connect a leaf to itself
		if (ports[i]->LeafNodes[0] == ports[i]->LeafNodes[1])
		{
			VectorRemoveAt(ports, i);
			continue;
		}

		// remove any portals that join a solid leaf
		bool remove = false;
		for (KBspNode* node : ports[i]->LeafNodes)
		{
			if (node->IsSolidLeaf())
			{
				VectorRemoveAt(ports, i);
				remove = true;
				break;
			}
		}
		if (remove) continue;

		// if we got this far in the loop, this portal is the real deal
		for (KBspNode* leaf : ports[i]->LeafNodes)
		{
			LeafPortalAddMutex.lock();
			leaf->LeafPortals.push_back(ports[i].get());
			LeafPortalAddMutex.unlock();
		}
	}
}

void KBspTree::FloodFill()
{
	//return;
	DVec3 startpos;

	for (auto& ent : KMapCompiler::Get().ParsedEntities)
	{
		if (ent->GetName() == "deathmatch_spawn")
			startpos = ent->GetOrigin();
	}

	TVector<KMapEntity*> detectors;
	for (auto& ent : KMapCompiler::Get().ParsedEntities)
	  if (ent->GetName().Contains("leak_detector"))
		detectors.push_back(ent.get());

	bool leakTesting = detectors.size() == 2;

	if (leakTesting) startpos = detectors[0]->GetOrigin();

	KBspNode* startnode = NodeFromPoint(startpos);

	KBspNode* targetnode = HeadNode.get();
	if (leakTesting) targetnode = NodeFromPoint(detectors[1]->GetOrigin());

	if (detectors.size() == 1) SYSLOG_WARNING("Only one leak detector found. Must have two to detect a leak.");
	if (detectors.size() > 2) SYSLOG_WARNING("More than two leak detectors placed in the map. Must have only two.");

	// to prevent a stack overflow from recursion, exit when we get too deep
	// store wherever we left off in this vector
	TVector<KBspNode*> checkpoints = { startnode };

	TMap<KBspNode*, TVector<DVec3>> leakTraces;
	TVector<DVec3> leakFinal; // final path after discovery
	bool leakFound = false;

	if (leakTesting)
	{
		leakFinal.push_back(detectors[0]->GetOrigin());
		leakTraces[startnode] = leakFinal;
	}

	// returns true when the leak target is reached (if leak test is active)
	std::function<void(KBspNode*, u32, TVector<DVec3>)> FillFromNode = 
		[&](KBspNode* node, u32 depth, TVector<DVec3> leakPath)
	{
		if (depth > 100)
		{
			checkpoints.push_back(node);
			if (leakTesting) leakTraces[node] = leakPath;
			return;
		}

		if (!node || node->bFilled) return;
		node->bFilled = true;

		if (leakTesting && targetnode && targetnode == node)
		{
			SYSLOG_WARNING("LEAK DETECTED");

			leakFound = true;
			leakTesting = false;
			leakPath.push_back(detectors[1]->GetOrigin());
			leakFinal = leakPath;
			leakTraces.clear();
		}

		for (KLeafPortal* portal : node->LeafPortals)
		{
			if (leakTesting) leakPath.push_back(portal->Center);
			else if (leakPath.size()) leakPath.clear();

			for (KBspNode* n : portal->LeafNodes)
			{
				// need to track every path since the recursion will be broken
				if (leakTesting) leakPath.push_back(portal->Center);
				FillFromNode(n, depth + 1, leakPath);
			}
		}
	};

	for (u32 i = 0; i < checkpoints.size(); i++)
	{	
		// checkpoint vector will grow as this runs
		FillFromNode(checkpoints[i], 0, leakTraces[checkpoints[i]]);
	}

	if (!leakFound) return;

	// need to straighten up the leak detector line
	if (leakFinal.size() > 0)
	{
		KMapCompiler::Get().LeakPath.push_back(leakFinal[0]);

		for (u32 i = 0; i < leakFinal.size(); i++)
		{
			for (u32 j = leakFinal.size() - 1; j > i; j--)
			{
				if (!TraceLine(DLineSegment(leakFinal[i], leakFinal[j])))
				{
					KMapCompiler::Get().LeakPath.push_back(leakFinal[j]);
					i = j - 1;
				}
			}
		}
	}
}

template <typename T>
static bool LineTestSingleFast(KBspNode* head, KLineSegment<T> line, KHitResult<T>* hit)
{
	using LineSegment = KLineSegment<T>;
	using Vec3 = KVec3<T>;
	using Plane = KPlane<T>;

	bool h = false;
	std::function<bool(LineSegment, KBspNode*, const Plane&)> ClipLine =
		[&](LineSegment seg, KBspNode* node, const Plane& last_split)
	{
		if (h) return false;
		if (!node) return true;
		if (node->IsLeaf())
		{
			if (node->IsSolidLeaf())
			{
				if (hit)
				{
					hit->bHit = true;
					Plane hitplane = last_split;
					T hitdist = 1;
					hitplane.Intersects(line, hitdist, 0);
					hit->Normal = ((line.b - line.a) | hitplane.Normal) > 0 ? -hitplane.Normal : hitplane.Normal;
					hit->Point = line.GetPoint(hitdist);
					hit->Time = hitdist;
				}
				h = true;
				return true;
			}
			return false;
		}

		Plane nodeplane = node->GetPlane().ToType<T>();
		EPointSide sideA = nodeplane.ClassifyPoint(seg.a, 0);
		EPointSide sideB = nodeplane.ClassifyPoint(seg.b, 0);

		if (sideA == EPointSide::On)
		{
			if (sideB == sideA)
			{
				sideA = EPointSide::Front;
				sideB = EPointSide::Front;
			}
			else
				sideA = sideB;

		}
		else if (sideB == EPointSide::On)
		{
			if (sideB == sideA)
			{
				sideA = EPointSide::Front;
				sideB = EPointSide::Front;
			}
			else
				sideB = sideA;
		}

		if (sideA == sideB)
		{
			if (sideA == EPointSide::Front)
				return ClipLine(seg, node->GetFront(), last_split);
			else
				return ClipLine(seg, node->GetBack(), last_split);
		}
		else
		{
			T dist = -1;
			if (nodeplane.Intersects(seg, dist, 0))
			{
				Vec3 p = seg.GetPoint(dist);

				bool blocked = ClipLine(LineSegment(seg.a, p), sideA == EPointSide::Behind ?
					node->GetBack() : node->GetFront(), nodeplane);

				if (!blocked)
					return ClipLine(LineSegment(p, seg.b), sideB == EPointSide::Behind ?
						node->GetBack() : node->GetFront(), nodeplane);

				return false;
			}
		}
		return false;
	};

	ClipLine(line, head, head->GetPlane().ToType<T>());
	return h;
}

bool KBspTree::TraceLine(DLineSegment line, DHitResult* hit)
{
	return LineTestSingleFast<f64>(HeadNode.get(), line, hit);
}

bool KBspTree::TraceLine(FLineSegment line, FHitResult* hit)
{
	return LineTestSingleFast<f32>(HeadNode.get(), line, hit);
}

static bool SphereTraceTest(DLineSegment line, f64 traceRadius, DHitResult* hit, KBspNode* headnode)
{

	const f64 EPSILON = .00001;
	f64 outputFraction;
	DVec3 outputEnd;
	bool outputStartsOut;
	bool outputAllSolid;
	DVec3 inputStart = line.a;
	DVec3 inputEnd = line.b;

	DBoundingBox linebounds;
	linebounds.Update(line.a);
	linebounds.Update(line.b);
	linebounds.Min -= traceRadius;
	linebounds.Max += traceRadius;

	std::function<void(KMapBrush*)> CheckBrush = [&](KMapBrush* brush)
	{
		f64 startFraction = -1.0f;
		f64 endFraction = 1.0f;
		bool startsOut = false;
		bool endsOut = false;

		DPlane best;

		if (!brush->Bounds.Overlaps(linebounds)) return;

		for (const DPlane& plane : brush->Planes)
		{
			
			f64 startDistance, endDistance;

			{
				startDistance = (inputStart | plane.Normal) - (plane.D + traceRadius - .01);
				endDistance = (inputEnd | plane.Normal) - (plane.D + traceRadius - .01);
			}

			if (startDistance > 0)
				startsOut = true;
			if (endDistance > 0)
				endsOut = true;

			// make sure the trace isn't completely on one side of the brush
			if (startDistance > 0 && endDistance > 0)
			{   // both are in front of the plane, its outside of this brush
				return;
			}
			if (startDistance <= 0 && endDistance <= 0)
			{   // both are behind this plane, it will get clipped by another one
				continue;
			}

			if (startDistance > endDistance)
			{   // line is entering into the brush
				float fraction = (startDistance - EPSILON) / (startDistance - endDistance);
				if (fraction > startFraction)
					startFraction = fraction;
				best = plane;
			}
			else
			{   // line is leaving the brush
				float fraction = (startDistance + EPSILON) / (startDistance - endDistance);
				if (fraction < endFraction)
					endFraction = fraction;
			}
		}

		if (startsOut == false)
		{
			outputStartsOut = false;
			if (endsOut == false)
				outputAllSolid = true;
			return;
		}

		if (startFraction < endFraction)
		{
			if (startFraction > -1 && startFraction < outputFraction)
			{
				if (startFraction < 0)
					startFraction = 0;
				outputFraction = startFraction;

				if (hit)
				{
					hit->bHit = true;
					hit->Normal = best.Normal;
					hit->Time = outputFraction;
					hit->Point = line.GetPoint(hit->Time);
				}
			}
		}
	};

	std::function<void(KBspNode*, f64, f64, DVec3, DVec3)> CheckNode =
		[&](KBspNode* node, f64 startFraction, f64 endFraction, DVec3 start, DVec3 end)
	{
		if (node->IsLeaf())
		{	
			for (int i = 0; i < node->GetLeafFaces().size(); i++)
			{
				KMapBrush* brush = node->GetLeafFaces()[i]->OwningBrush;
				if (brush && brush->Planes.size() > 0)
				{
					CheckBrush(brush);
				}
			}

			// don't have to do anything else for leaves
			return;
		}

		// this is a node

		DPlane plane = node->GetPlane();

		f64 startDistance, endDistance, offset;
		startDistance = (start | plane.Normal) - plane.D;
		endDistance = (end | plane.Normal) - plane.D;


		offset = traceRadius;

		if (startDistance >= offset && endDistance >= offset)
		{	// both points are in front of the plane
			// so check the front child
			CheckNode(node->GetFront(), startFraction, endFraction, start, end);
		}
		else if (startDistance < -offset && endDistance < -offset)
		{	// both points are behind the plane
			// so check the back child
			CheckNode(node->GetBack(), startFraction, endFraction, start, end);
		}
		else
		{	// the line spans the splitting plane
			i32 side;
			f64 fraction1, fraction2, middleFraction;
			DVec3 middle;

			// split the segment into two
			if (startDistance < endDistance)
			{
				side = 1; // back
				float inverseDistance = 1.0f / (startDistance - endDistance);
				fraction1 = (startDistance - offset + EPSILON) * inverseDistance;
				fraction2 = (startDistance + offset + EPSILON) * inverseDistance;
			}
			else if (endDistance < startDistance)
			{
				side = 0; // front
				float inverseDistance = 1.0f / (startDistance - endDistance);
				fraction1 = (startDistance + offset + EPSILON) * inverseDistance;
				fraction2 = (startDistance - offset - EPSILON) * inverseDistance;
			}
			else
			{
				side = 0; // front
				fraction1 = 1.0f;
				fraction2 = 0.0f;
			}

			// make sure the numbers are valid
			if (fraction1 < 0.0f) fraction1 = 0.0f;
			else if (fraction1 > 1.0f) fraction1 = 1.0f;
			if (fraction2 < 0.0f) fraction2 = 0.0f;
			else if (fraction2 > 1.0f) fraction2 = 1.0f;

			// calculate the middle point for the first side
			middleFraction = startFraction + (endFraction - startFraction) * fraction1;
			for (int i = 0; i < 3; i++)
				middle[i] = start[i] + fraction1 * (end[i] - start[i]);

			// check the first side
			CheckNode(side == 1 ? node->GetBack() : node->GetFront(), startFraction, middleFraction, start, middle);

			// calculate the middle point for the second side
			middleFraction = startFraction + (endFraction - startFraction) * fraction2;
			for (int i = 0; i < 3; i++)
				middle[i] = start[i] + fraction2 * (end[i] - start[i]);

			// check the second side
			CheckNode(side == 0 ? node->GetBack() : node->GetFront(), middleFraction, endFraction, middle, end);
		}
	};

	outputStartsOut = true;
	outputAllSolid = false;
	outputFraction = 1.0;

	// walk through the BSP tree
	CheckNode(headnode, 0, 1, inputStart, inputEnd);

	if (outputFraction == 1.0f)
	{	// nothing blocked the trace
		outputEnd = inputEnd;
	}
	else
	{	// collided with something 
		for (int i = 0; i < 3; i++)
		{
			outputEnd[i] = inputStart[i] + outputFraction * (inputEnd[i] - inputStart[i]);
		}
		hit->bHit = true;
		return true;
	}

	return false;
}

bool KBspTree::TraceSphere(DLineSegment line, f64 radius, DHitResult* hit /*= nullptr*/)
{
	return SphereTraceTest(line, radius, hit, HeadNode.get());


	bool h = false;

	f64 trace_dist = abs(HeadNode->GetPlane().SignedDistance(line.a)
						-HeadNode->GetPlane().SignedDistance(line.b));

	std::function<bool(DLineSegment, KBspNode*, DPlane)> ClipSphere =
		[&](DLineSegment seg, KBspNode* node, const DPlane& last_split)
	{
		if (!node) return true;
		if (node->IsLeaf())
		{
			if (node->IsSolidLeaf())
			{
				if (hit)
				{
					hit->bHit = true;
					DPlane hitplane = last_split;
					f64 hitdist = 1;
					hitplane.Intersects(line, hitdist, 0);
					if (hitdist < hit->Time || hit->Time == 0)
					{
						hit->Normal = ((hitplane.PointOnPlane() - line.a) | hitplane.Normal) > 0 ? -hitplane.Normal : hitplane.Normal;
						hit->Point = line.GetPoint(hitdist);
						hit->Time = hitdist;
					}
				}
				h = true;
				return true;
			}

			return false;
		}

		DPlane nodeplane = node->GetPlane();

		f64 distA = nodeplane.SignedDistance(seg.a);
		f64 distB = nodeplane.SignedDistance(seg.b);

		EPointSide pointA = (distA < 0) ? EPointSide::Behind : (distA == 0) ? EPointSide::On : EPointSide::Front;
		EPointSide pointB = (distB < 0) ? EPointSide::Behind : (distB == 0) ? EPointSide::On : EPointSide::Front;


		// shift "radius" units toward the plane
		distA += (distA < 0) ? radius : -radius;
		distB += (distB < 0) ? radius : -radius;

		EPointSide offsetA = (distA < 0) ? EPointSide::Behind : (distA == 0) ? EPointSide::On : EPointSide::Front;
		EPointSide offsetB = (distB < 0) ? EPointSide::Behind : (distB == 0) ? EPointSide::On : EPointSide::Front;

		// if a point is exactly on the plane, try to adjust it so its on the same side as the other point
		if (pointA == EPointSide::On)
		{
			if (pointB == pointA)
			{	
				// both on plane, just go front
				pointA = EPointSide::Front;
				pointB = EPointSide::Front;
			}
			else
				pointA = pointB;
		}
		else if (pointB == EPointSide::On)
		{
			if (pointB == pointA)
			{
				pointA = EPointSide::Front;
				pointB = EPointSide::Front;
			}
			else
				pointB = pointA;
		}

		if (offsetA == EPointSide::On)
		{
			offsetA = pointA == EPointSide::Behind ? EPointSide::Front : EPointSide::Behind;
		}
		if (offsetB == EPointSide::On)
		{
			offsetB = pointB == EPointSide::Behind ? EPointSide::Front : EPointSide::Behind;
		}

		bool splitA = offsetA != pointA;
		bool splitB = offsetB != pointB;

		if (splitB || splitA || (pointA != pointB))
		{
			// split down tree
			f64 total_dist = abs(distA - distB);

			distA = abs(distA) / total_dist;
			DVec3 midA = seg.GetPoint(std::clamp(distA, 0.0, 1.0));

			if (!ClipSphere(DLineSegment(seg.a, midA), pointA == EPointSide::Behind ?
				node->GetBack() : node->GetFront(), nodeplane))
			{
				distB = abs(distB) / total_dist;
				DVec3 midB = seg.GetPoint(std::clamp(distB, 0.0, 1.0));
				return ClipSphere(DLineSegment(midB, seg.b), pointB == EPointSide::Behind ?
					node->GetBack() : node->GetFront(), nodeplane);
			}
			
			return true;
		}
		else
		{
			// on same side, no split
			if (pointA == EPointSide::Front)
				return ClipSphere(seg, node->GetFront(), last_split);
			else
				return ClipSphere(seg, node->GetBack(), last_split);
		}
	};

	ClipSphere(line, HeadNode.get(), HeadNode->GetPlane());
	return h;
}

UPtr<KLeafPortal> KBspTree::PortalFromPlane(const DPlane& plane)
{
	// create a poly on this plane that extends the map bounding box

	DBoundingBox bounds = KMapCompiler::Get().MapBounds;
	bounds.Min -= DVec3(1, 1, 1);
	bounds.Max += DVec3(1, 1, 1);
	TVector<DLineSegment> edges;
	bounds.GetEdges(edges);

	// intersect each edge of the bounding box with the plane to get points
	TVector<DVec3> verts;
	for (const DLineSegment& edge : edges)
	{
		if (plane.ClassifyPoint(edge.a) == EPointSide::On
			&& plane.ClassifyPoint(edge.b) == EPointSide::On)
		{
			// if the line sits on the plane, add both points
			// realistically i dont think this should ever happen
			if (!VectorContains(verts, edge.a)) verts.push_back(edge.a);
			if (!VectorContains(verts, edge.b)) verts.push_back(edge.b);
		}
		else
		{
			// find intersection point
			f64 dist = -1;
			if (plane.Intersects(edge, dist) && dist >= -.001 && dist <= 1.001)
			{
				dist = std::clamp(dist, 0.0, 1.0);
				DVec3 p = edge.GetPoint(dist);
				if (!VectorContains(verts, p)) verts.push_back(p);
			}
		}
	}

	K_ASSERT(verts.size() > 2, "portal did not form a polygon on creation");

	// create a portal from the vertices
	UPtr<KLeafPortal> portal = std::make_unique<KLeafPortal>();
	//portal->bIsPortal = true;
	portal->Plane = plane;

	for (DVec3& v : verts) portal->Vertices.push_back(v);
	portal->InitFromVertices();

	return portal;
}