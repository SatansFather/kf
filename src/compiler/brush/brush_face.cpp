#include "brush_face.h"

#if _COMPILER

#include "brush.h"
#include "engine/render/interface/render_interface.h"
#include "engine/render/interface/texture2d.h"
#include "engine/math/glm.h"
#include "engine/collision/hit_result.h"
#include "compiler/light/light_ent.h"
#include "compiler/light/face_attribs.h"
#include "compiler/bsp/bsp_tree.h"
#include "compiler/surface_flags.h"
#include "compiler/compiler.h"
#include "engine/render/surface2d.h"
#include "engine/collision/trace.h"
#include "engine/render/scene.h"
#include "../../game/collision_mask.h"
#include "brush.h"
#endif

KBrushFace::KBrushFace() {}
KBrushFace::~KBrushFace() {}

static TMap<KString, u32> MaterialToBufferIndex;

#if !_SERVER
void KBrushFace::SetMaterial(const KString& mat)
{
	Material = mat;
	if (MaterialToBufferIndex.contains(mat))
	{
		u32& idx = MaterialToBufferIndex[mat];
		BufferIndex = idx;
		idx++;
	}
	else
	{
		BufferIndex = 0;
		MaterialToBufferIndex[mat] = 1;
	}
}
#endif 


void KBrushFace::InitFromVertices()
{
	SortVerticesCW();
#if _COMPILER
	SetupIndexBuffer();
#endif

	SurfaceArea = Poly.Area();

	// bounding box
	Bounds.Reset();
	for (KBrushVertex& v : Vertices)
		Bounds.Update(v.Point);
}



void KBrushFace::SortVerticesCW()
{
	Center = DVec3(0);
	for (KBrushVertex& v : Vertices) Center += v.Point;
	Center /= Vertices.size();

	Poly.Points.clear();
	for (KBrushVertex& v : Vertices)
		Poly.Points.push_back(v.Point);

	/*DVec3 sumCenter;
	f64 sumWeight = 0;
	for (i32 i  = 0; i < Poly.Points.size(); i++)
	{
		i32 prevIndex = (i - 1);// % Poly.Points.size();
		if (prevIndex < 0) prevIndex = Poly.Points.size() + prevIndex;

		const DVec3& point = Poly.Points[i];
		const DVec3& next  = Poly.Points[(i + 1) % Poly.Points.size()];
		const DVec3& prev  = Poly.Points[prevIndex];

		f64 weight = (point - next).Length() + (point - prev).Length();
		sumCenter += point * weight;
		sumWeight += weight;
	}
	Center = sumCenter / sumWeight;*/
	// TODO 
	/*
	def poly_center(poly):
	sum_center = (0, 0)
	sum_weight = 0.0
	for point in poly:
		weight = ((point - point.next).length +
				  (point - point.prev).length)
		sum_center += point * weight
		sum_weight += weight

	return sum_center / sum_weight

	// also, height scale doesnt work on the same direction depending on face normal

	*/ 

	DVec3 base = Vertices[0].Point - Center;
	DVec3 norm = Plane.Normal;
	DVec3 cross = base.Cross(norm);

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
		[&](const KBrushVertex& a, const KBrushVertex& b) -> bool
		{
			return AngleTo(a.Point - Center) < AngleTo(b.Point - Center);
		}
	);
}

#if _COMPILER
TMap<KString, KString>& KBrushFace::GetMaterialPropertyMap()
{
	return
	bVolumeMaterial ? 
		(OwningBrush->bMovedEntity ? 
			OwningBrush->OriginalEntityProperties : 
			OwningBrush->OwningEntity->Properties) 
		: FaceAttribs->PropertyMap;
}
#endif

#if _COMPILER

void KBrushFace::DivideLargeFace(f64 max_dimension, TVector<UPtr<KBrushFace>>& out_faces)
{
	// find furthest vertices and see if its more than max_dimension
	DVec3 a;
	DVec3 b;
	f64 furthest = -1;
	for (u32 i = 0 ; i < Vertices.size(); i++)
	{
		for (u32 j = 1; j < Vertices.size(); j++)
		{
			if (i == j) continue;

			KBrushVertex& v1 = Vertices[i];
			KBrushVertex& v2 = Vertices[j];
			f64 dist = v1.Point.DistanceSq(v2.Point);
			if (dist > furthest) 
			{
				furthest = dist;
				a = v1.Point;
				b = v2.Point;
			}
		}
	}

	K_ASSERT(furthest >= 0, "furthest distance between vertices was < 0");

	if (furthest >= max_dimension)
	{
		// get x and y axis according to the furthest distance
		DVec3 axisA = (b - a).GetNormalized();
		DVec3 axisB = (axisA ^ Plane.Normal).GetNormalized();

		// find extreme points in each direction
		DVec3 minA = Poly.ExtremePoint(-axisA);
		DVec3 maxA = Poly.ExtremePoint(axisA);
		DVec3 minB = Poly.ExtremePoint(-axisB);
		DVec3 maxB = Poly.ExtremePoint(axisB);

		// distance from min point to a plane containing max point is the span along that axis
		DPlane planeA(maxA, axisA);
		DPlane planeB(maxB, axisB);
		f64 distA = planeA.Distance(minA);
		f64 distB = planeB.Distance(minB);

		// actual slice counts
		u32 cutsA = 0;
		u32 cutsB = 0;
		if (distA > max_dimension) cutsA = (u32)(distA / max_dimension);
		if (distB > max_dimension) cutsB = (u32)(distB / max_dimension);
	
		// loop A will add to this
		TVector<KBrushFace*> splits = { this };

		const auto DivideAxis = [&](bool isA) -> void
		{
			// reverse because we will add to splits in the A loop
			for (i32 i = splits.size() - 1; i >= 0; i--)
			{
				// the face we will cut, might become a split result 
				KBrushFace* target = splits[i];

				u32 cut_count = isA ? cutsA : cutsB;
				f64 dist = isA ? distA : distB;
				DVec3 axis = isA ? axisA : axisB;
				DVec3 minpoint = isA ? minA : minB;

				for (u32 i = 1; i <= cut_count; i++)
				{
					// prepare the split plane at the proper distance
					f64 cut_dist = (f64(i) / f64(cut_count + 1)) * dist;
					DVec3 cut_point = minpoint + (axis * cut_dist);
					DPlane cut_plane(cut_point, axis);

					if (target->Poly.ClassifyToPlane(cut_plane) == EPolyClassification::Spanning)
					{
						// get a unique ptr to the new plane
						auto split = target->SplitByPlane(cut_plane);
						out_faces.push_back(std::move(split));
						KBrushFace* newface = out_faces[out_faces.size() - 1].get();

						// B loop will need to iterate each split generated by A
						if (isA) splits.push_back(newface);

						// check if this or new face should be split next
						// plane is facing AWAY from the starting point
						target = (cut_plane.ClassifyPoint(target->Center) == EPointSide::Front) ? target : newface;
					}
					else 
					{
						// B faces might not fill the entire A axis
						K_ASSERT(!isA, "polygon along axisA did not span the cutting plane");
						continue;
					}
				}
			}		
		};

		if (cutsA > 0)
		{
			DivideAxis(true);
		}
		if (cutsB > 0)
		{
			DivideAxis(false);
		}
	}
}

UPtr<KBrushFace> KBrushFace::MergeWith(KBrushFace* other)
{	
	if (!Bounds.Overlaps(other->Bounds)) return UPtr<KBrushFace>(nullptr);

	// check for shared edge
	bool match = false;
	for (u32 i = 0; i < Poly.NumEdges(); i++)
	{
		for (u32 j = 0; j < other->Poly.NumEdges(); j++)
		{
			if (Poly.Edge(i).Equals(other->Poly.Edge(j)))
			{
				// direct edge match
				match = true;
				break;
			}
		}
	}

#if 0
	if (!match)
	{
		const auto addVertex = [](KBrushFace* face, const DVec3& point, const KBrushVertex& va, const KBrushVertex& vb) -> void
		{
			f64 dist = va.Point.Distance(point) / va.Point.Distance(vb.Point);
			KBrushVertex v;
			glm::vec<2, f64> tex = glm::lerp(glm::vec<2, f64>(va.TexU, va.TexV), glm::vec<2, f64>(vb.TexU, vb.TexV), dist);
			v.TexU = tex.x;
			v.TexV = tex.y;
			v.Normal = DVec3::FromGLM(glm::lerp(va.Normal.ToGLM(), vb.Normal.ToGLM(), dist));
			v.Normal.Normalize();
			v.Point = point;
			face->Poly.Points.push_back(point);
			face->Vertices.push_back(v);
			face->SortVerticesCW();
		};

		const auto vertFromPoint = [](KBrushFace* face, const GVec3& point) -> KBrushVertex
		{
			for (const KBrushVertex& v : face->Vertices)
			  if (v.Point == point)
				return v;

			return KBrushVertex();
		};

		const auto pointCheck = [addVertex, vertFromPoint](KBrushFace* a, KBrushFace* b) -> bool
		{
			// if these faces share two vertices, we can still merge them
			for (u32 i = 0; i < a->Poly.NumEdges(); i++)
			{
				const GLineSegment& lineA = a->Poly.Edge(i);
				for (u32 j = 0; j < b->Poly.NumEdges(); j++)
				{
					const GLineSegment& lineB = a->Poly.Edge(j);
					bool containsA = lineA.Contains(lineB.a);
					bool containsB = lineA.Contains(lineB.b);
					if (containsA != containsB)
					{
						// one point is contained, check if other edge contains a point from this edge
						if (lineA.Contains(lineB.a))
						{
							addVertex(a, lineB.a, vertFromPoint(a, lineA.a), vertFromPoint(a, lineA.b));
							return true;
						}
						if (lineA.Contains(lineB.b))
						{
							addVertex(a, lineB.b, vertFromPoint(a, lineA.a), vertFromPoint(a, lineA.b));
							return true;
						}
					}
					else if (containsA)
					{
						// both points are contained
						addVertex(a, lineB.b, vertFromPoint(a, lineA.a), vertFromPoint(a, lineA.b));		
						return true;
					}
					// this edge contains no points of the other edge
				}
			}
			return false;
		};

		match = pointCheck(this, other);
		if (!match) match = pointCheck(other, this);
	}
#endif

	// no shared edge
	if (!match)return UPtr<KBrushFace>(nullptr);

	// merge vertices
	TVector<KBrushVertex> verts;
	for (KBrushVertex& v : Vertices) verts.push_back(v);
	for (KBrushVertex& v : other->Vertices) verts.push_back(v);

	// kinda dumb to do it this way, but creating a temporary face gives us a sort function
	{
		KBrushFace temp;
		temp.Plane = Plane;
		temp.Vertices = verts;
		temp.SortVerticesCW();

		// now sorted clockwise
		verts = temp.Vertices;
	}

	// create a polygon and test for convexity
	DPolygon poly;
	for (KBrushVertex& v : verts) poly.Points.push_back(v.Point);
	
	// poly points and brushvertex vector have matching indices

	// check if any point falls between two collinear edges, remove it
	for (i32 i = 0; i < poly.Points.size(); i++)
	{
		// if dot(ab, ac) == 1 then b can be removed
		i32 j = (i + 1) % poly.Points.size();
		i32 k = (i + 2) % poly.Points.size();
		DVec3 a = poly.Points[i];
		DVec3 b = poly.Points[j];
		DVec3 c = poly.Points[k];
		DVec3 ab = (b - a).GetNormalized(); // theres a way to do this without sqrt
		DVec3 ac = (c - a).GetNormalized(); // but oh well!!!
		f64 dot = ab | ac;
		if ( abs(dot - 1) < 1e-15 )
		{
			VectorRemoveAt(poly.Points, j);
			VectorRemoveAt(verts, j);
			i = -1; // start over
		}
	}

	// somehow the whole process is faster if this check comes AFTER point removal
	// convex check is probably more robust afterward anyway
	if (!poly.IsConvex()) return std::unique_ptr<KBrushFace>(nullptr);

	// we have the points ready to make a new (merged) face
	UPtr<KBrushFace> out = std::make_unique<KBrushFace>();
	*(out.get()) = *this; // copy
	out->Vertices.clear();
	out->Poly.Points.clear();
	out->Vertices = verts;
	out->InitFromVertices();

	return out;
}

u32 KBrushFace::SetupIndexBuffer()
{
	Triangles.clear();
	Indices.clear();
	for (u32 i = 2; i < Vertices.size(); i++)
	{
		Indices.push_back(0);
		Indices.push_back(i - 1);
		Indices.push_back(i);

		Triangles.push_back(DTriangle(
			Vertices[0].Point,
			Vertices[i - 1].Point,
			Vertices[i].Point));
	}
	return Vertices.size();
}

void KBrushFace::PrepareFace()
{
	//LoadTexture();
	//DetermineTextureCoordinates();
	PrepareLightingData();
}

bool KBrushFace::ShouldHaveLight()
{
	return (!((Surface & (u32)ESurfaceFlags::NO_LIGHT)
		|| (Surface & (u32)ESurfaceFlags::NO_DRAW)
		|| (Vertices.size() == 0)));
}

DVec3 KBrushFace::GetNormalAtPoint(DVec3 point)
{
	DVec3 norm = Plane.Normal;

	if (bSmoothed)
	{
		bool foundTri = false;
		for (DTriangle& tri : Triangles)
		{
			if (!tri.Contains(point, Plane.Normal)) continue;
			foundTri = true;
			DVec3 bary = tri.BarycentricUVW(point);
			KBrushVertex a, b, c;

			for (KBrushVertex& v : Vertices)
			{
				if (v.Point == tri.a) a = v;
				if (v.Point == tri.b) b = v;
				if (v.Point == tri.c) c = v;
			}

			norm = (a.Normal * bary.x) +
			       (b.Normal * bary.y) +
			       (c.Normal * bary.z);

			norm.Normalize();
			break;
		}
		//if (!foundTri)
		//	LOG("POINT NOT INSIDE ANY TRIANGLE: " + point.ToString() + " - Texture: " + TexName);
	}

	return norm;
}

void KBrushFace::AddSmoothingGroup(i32 group)
{
	SmoothingGroups.push_back(std::clamp(group, 0, MAX_I32));
}

#if 0
UPtr<KSurface2D> KBrushFace::LoadTextureAsSurface()
{
	if (TexName == "") return std::unique_ptr<KSurface2D>(nullptr);
	if (KMapCompiler::Get().LoadedTextureSurfaces.contains(TexName)) 
		return KMapCompiler::Get().LoadedTextureSurfaces[TexName];

	UPtr<KSurface2D> surface = std::make_unique<KSurface2D>();
	surface->LoadPNG(TexName);

	return surface;
	
	/*if (TexName == "") return;

	KTexture2D* tex = GetRenderInterface()->LoadTexture2D(TexName);
	K_ASSERT(tex, KString("could not create new texture or find existing texture ") + KString("\"TexName\""));

	// let the compiler know this texture needs to be packed
	KMapCompiler::Get().AddTexture(tex);

	Texture = tex;*/
}
#endif

UPtr<KBrushFace> KBrushFace::SplitByPlane(DPlane plane, f64 epsilon)
{
	TVector<KBrushVertex> frontverts;
	TVector<KBrushVertex> backverts;

	u32 vertcount = Vertices.size();

	const auto SplitEdge = [&](const KBrushVertex& va, const KBrushVertex& vb) -> void
	{
		// edge (a, b) straddles, output intersection point to both sides
		f64 dist = -1;
		DLineSegment seg(va.Point, vb.Point);
		bool intersect = plane.Intersects(seg, dist, 0);
		K_ASSERT(intersect && dist != -1, "spanning edge did not intersect plane");
		DVec3 i = seg.GetPoint(dist);

		KString err = "point did not lay on plane after splitting spanning edge\nsigned dist: ";
		err += KString(plane.SignedDistance(i), false);
		K_ASSERT(plane.ClassifyPoint(i) == EPointSide::On, err.CStr());

		KBrushVertex mid(i);
		
		// TODO make actual lerp functions in my vectors so i dont have to use glm
		glm::vec<2, f64> tex = glm::lerp(glm::vec<2, f64>(va.TexU, va.TexV), glm::vec<2, f64>(vb.TexU, vb.TexV), dist);
		mid.TexU = tex.x;
		mid.TexV = tex.y;

		mid.Normal = DVec3::FromGLM(glm::lerp(va.Normal.ToGLM(), vb.Normal.ToGLM(), f32(dist)));
		mid.Normal.Normalize();

		frontverts.push_back(mid);
		backverts.push_back(mid);
	};

	// place existing points where they should be
	for (KBrushVertex v : Vertices)
	{
		EPointSide side = plane.ClassifyPoint(v.Point);
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
	KBrushVertex a = Vertices[vertcount - 1];
	EPointSide aSide = plane.ClassifyPoint(a.Point);
	for (u32 n = 0; n < vertcount; n++)
	{
		KBrushVertex b = Vertices[n];
		EPointSide bSide = plane.ClassifyPoint(b.Point, epsilon);

		if (aSide == EPointSide::Front && bSide == EPointSide::Behind
		 || aSide == EPointSide::Behind && bSide == EPointSide::Front)
			SplitEdge(a, b); // adds split point to front and back

		// keep b as the starting point of the next edge
		a = b;
		aSide = bSide;
	}
	
	K_ASSERT(frontverts.size() >= 3, "face front vertices < 3 after split");
	K_ASSERT(backverts.size() >= 3, "face back vertices < 3 after split");

	// clear this face and recreate it with the front of the split
	Vertices.clear();
	Poly.Points.clear();

	UPtr<KBrushFace> out = std::make_unique<KBrushFace>();
	*(out.get()) = *this; // copy

	for (const KBrushVertex& v : frontverts) Vertices.push_back(v);
	InitFromVertices();

	for (const KBrushVertex& v : backverts) out->Vertices.push_back(v);
	out->InitFromVertices();

	return out;
}	

bool KBrushFace::HasTransparency() const
{
#if _SERVER
	return false;
#else
	//return Texture && Texture->GetSurface()->HasTransparency();
	return KMapCompiler::Get().LoadedTextureSurfaces[SurfaceIndex].contains(TexName)
		&& KMapCompiler::Get().LoadedTextureSurfaces[SurfaceIndex][TexName]->HasTransparency();
#endif
}

TVector<KBrushVertex>& KBrushFace::GetLitVertices()
{
	return (Surface & ESurfaceFlags::PRE_SPLIT_VERTEX) ? OriginalVertices : Vertices;
}

void KBrushFace::InterpolateVertexLighting()
{
	if ((Surface & ESurfaceFlags::VERTEX_LIGHTING)
		&& Surface & ESurfaceFlags::PRE_SPLIT_VERTEX)
	{
		
	}
}

DVec2 compute2DPolygonCentroid(const DVec2* vertices, int vertexCount)
{
	DVec2 centroid = { 0, 0 };
	double signedArea = 0.0;
	double x0 = 0.0; // Current vertex X
	double y0 = 0.0; // Current vertex Y
	double x1 = 0.0; // Next vertex X
	double y1 = 0.0; // Next vertex Y
	double a = 0.0;  // Partial signed area

	// For all vertices
	int i = 0;
	for (i = 0; i < vertexCount; ++i)
	{
		x0 = vertices[i].x;
		y0 = vertices[i].y;
		x1 = vertices[(i + 1) % vertexCount].x;
		y1 = vertices[(i + 1) % vertexCount].y;
		a = x0 * y1 - x1 * y0;
		signedArea += a;
		centroid.x += (x0 + x1) * a;
		centroid.y += (y0 + y1) * a;
	}

	signedArea *= 0.5;
	centroid.x /= (6.0 * signedArea);
	centroid.y /= (6.0 * signedArea);

	return centroid;
}

void KBrushFace::DetermineTextureCoordinates()
{
	if (!KMapCompiler::Get().LoadedTextureSurfaces[SurfaceIndex].contains(TexName)) return;

	KSurface2D* surf = KMapCompiler::Get().LoadedTextureSurfaces[SurfaceIndex][TexName].get();
	K_ASSERT(surf, "no texture surface for face existed despite having an entry for texname");

	if (Material == "portal")
	{
		const f32 texSize = 32;

		TVector<DVec2> verts;
		for (const KBrushVertex& v : Vertices)
			verts.push_back(Poly.MapTo2D(v.Point, true));

		//DVec2 center = Poly.MapTo2D(Center);
		
		DVec2 center = compute2DPolygonCentroid(verts.data(), verts.size());
		for (u32 i = 0; i < verts.size(); i++)
		{
			const DVec2& v = verts[i];
			Vertices[i].TexU = (v.x - center.x) / texSize;
			Vertices[i].TexV = (v.y - center.y) / texSize;
		}
	}
	else
	{
		// determine the texture coordinates for each vertex
		for (KBrushVertex& v : Vertices)
		{
			v.TexU = v.Point.x * AxisU.x + v.Point.y * AxisU.y + v.Point.z * AxisU.z;
			v.TexU += TexOffsetX;
			v.TexU /= surf->GetPaddedWidth();

			v.TexV = v.Point.x * AxisV.x + v.Point.y * AxisV.y + v.Point.z * AxisV.z;
			v.TexV += TexOffsetY;
			v.TexV /= surf->GetPaddedHeight();
		}
	}
}

void KBrushFace::PrepareLightingData()
{
	KMapCompiler& compiler = KMapCompiler::Get();
	if (true || ShouldHaveLight())
	{
		compiler.AddLitFace(this);

		if (SmoothingGroups[0] > 0)
			compiler.AddFaceToSmoothingGroup(SmoothingGroups[0], this);

		if (SmoothingGroups[1] > 0)
			compiler.AddFaceToSmoothingGroup(SmoothingGroups[1], this);

		if (LightmapResolution <= 0)
			LightmapResolution = compiler.GetMapSettings().LightmapResolution;
	}
}

void KBrushFace::SetupLightmapDimensions()
{
	if (LightmapResolution == 0) 
		LightmapResolution = KMapCompiler::Get().GetMapSettings().LightmapResolution;

	if (Surface & u8(ESurfaceFlags::NO_LIGHT)) return;
	if (Surface & u8(ESurfaceFlags::FULL_LIGHT)) return;
	if (Surface & u8(ESurfaceFlags::NO_DRAW)) return;
	if (Surface & u8(ESurfaceFlags::VERTEX_LIGHTING)) return;

	DPolygon poly2d;
	for (i32 i = 0; i < Poly.NumVertices(); i++)
	{
		DVec2 point = Poly.MapTo2D(Poly.Vertex(i));
		poly2d.Points.push_back(DVec3(point.x, point.y, 0));
	}

	DVec3 u = DVec3(1, 0, 0);
	DVec3 v = DVec3(0, 1, 0);

	// find most X-Treme x and y, in both directions so we know where to start and end sampling
	f64 minX = poly2d.ExtremePoint(-u).x;
	f64 maxX = poly2d.ExtremePoint(u).x;
	f64 minY = poly2d.ExtremePoint(-v).y;
	f64 maxY = poly2d.ExtremePoint(v).y;

	// round known start and end points to lightmap resolution step
	f64 startX = round(minX / LightmapResolution) * LightmapResolution + (LightmapResolution * .5);
	f64 startY = round(minY / LightmapResolution) * LightmapResolution + (LightmapResolution * .5);
	f64 endX = round(maxX / LightmapResolution) * LightmapResolution + (LightmapResolution * .5);
	f64 endY = round(maxY / LightmapResolution) * LightmapResolution + (LightmapResolution * .5);
	//
	//f64 startX = RoundNearest(minX, LightmapResolution);
	//f64 startY = RoundNearest(minY, LightmapResolution);
	//f64 endX = RoundNearest(maxX, LightmapResolution);
	//f64 endY = RoundNearest(maxY, LightmapResolution);

	// figure out the lightmap size in pixels
	u32 width = (endX - startX) / LightmapResolution;
	u32 height = (endY - startY) / LightmapResolution;

	// add a one pixel border for padding
	width += 2;
	height += 2;

	// find percentage of a each dimension that is occupied by a single pixel
	f64 pixel_x = 1.f / f64(width);
	f64 pixel_y = 1.f / f64(height);

	// find lightmap coords for each vertex
	for (KBrushVertex& vert : Vertices)
	{
		DVec2 vert2d = Poly.MapTo2D(vert.Point);

		// 0 and 1 shouldnt be on the padded edges
		vert.LocalLightU = MapRange(vert2d.x, minX, maxX, 0.0 + pixel_x, 1.0 - pixel_x);
		vert.LocalLightV = MapRange(vert2d.y, minY, maxY, 0.0 + pixel_y, 1.0 - pixel_y);
	}

	// prepare luxels
	Lightmap.resize(width);
	for (TVector<KLuxel>& arr : Lightmap) arr.resize(height);

	i32 indexX = 0;
	for (f64 x = startX/* + (LightmapResolution * .5)*/; x <= endX; x += LightmapResolution)
	{
		indexX++;
		i32 indexY = 0;
		for (f64 y = startY/* + (LightmapResolution * .5)*/; y <= endY ; y += LightmapResolution)
		{
			indexY++;

			KLuxel& luxel = Lightmap[indexX][indexY];

			// this will be set false of found otherwise
			luxel.SetOriginallyOnPoly(true);

			DVec3 point = Poly.MapFrom2D(DVec2(x, y)); 

			for (f64 i = .25; i > 0 && !Poly.Contains(point, 9999.f, Plane.Normal); i -= .03)
			{
				luxel.SetOriginallyOnPoly(false);	

				// push points that are just outside the polygon back inside to prevent artifacts
				DVec3 closest = Poly.ClosestPoint(point);
				f64 dist = point.DistanceSq(closest);
				if (dist < .01)
				{
					// sitting on directly on an edge, push .1 units toward center
					point += (Center - point).GetNormalized() * i;
				}
				else if (dist < std::pow(LightmapResolution * 3, 2))
				{
					// less than 3 lightmap units away, move to the poly
					const GVec3 adjustDir = (closest - point).GetNormalized();
					point += adjustDir * (sqrt(dist) + i);
				}
			}

			// points nearest to an edge need to be moved to that edge 
			// makes lighting between faces look smoother
			for (u32 i = 0; i < Poly.NumEdges(); i++)
			{
				const DLineSegment& edge = Poly.Edge(i);
				f64 dist;
				DVec3 closest = edge.ClosestPoint(point, dist);

				if (luxel.EdgeIndex == MAX_U32 || dist < luxel.EdgeDistance)
				{
					luxel.EdgeDistance = dist;
					luxel.EdgeIndex = i;
				}
			}

			if (luxel.EdgeIndex != MAX_U32 && luxel.EdgeDistance <= LightmapResolution)
			{
				if (luxel.EdgeDistance > .25) 
				{
					// we are close enough to this line
					const DLineSegment& edge = Poly.Edge(luxel.EdgeIndex);
					f64 dist;
					DVec3 closest = edge.ClosestPoint(point, dist);

					DVec3 edgeNorm = (edge.b - edge.a).Cross(Plane.Normal).GetNormalized();

					// ensure its facing out from the center
					DVec3 fromCenter = closest - GetCenter();
					if (fromCenter.Dot(edgeNorm) < 0)
						edgeNorm = -edgeNorm;

					f64 scale = 1;
					if (!Poly.Contains(point, 9999, Plane.Normal))
						scale = -1; // point outside needs to move in

					// move point just to the edge
					point += edgeNorm * (luxel.EdgeDistance - .2) * scale;
				}

				//if (luxel.IsOriginallyOnPoly())
				{
					// luxels on the edges are start points for radiosity
					luxel.SetOnPolyEdge(true);
				}
			}

			DVec2 point2d = Poly.MapTo2D(point);
			luxel.Point2D = point2d;

			luxel.SetOnPoly(Poly.Contains(point, 9999.f, Plane.Normal));
			luxel.Normal = GetNormalAtPoint(point);
			luxel.Point3D = point + 
				(Plane.Normal * (FaceAttribs ? FaceAttribs->LightPositionOffset : .25));

		}

		K_ASSERT(indexY == Lightmap[indexX].size() - 1, 
			"did not fill entire lightmap - " + KString(indexY + 1) + 
			" of " + KString(Lightmap[indexX].size()) + " filled");
	}
}

void KBrushFace::FindValidLights(TVector<KLightEntity*>& lights)
{
	for (KLightEntity* light : lights)
	{
		bool valid = true;
		if (light->GetColor() == DVec3(0, 0, 0) && light->GetGIScale() == DVec3(1, 1, 1)) continue;
		if (Surface & u32(ESurfaceFlags::VERTEX_LIGHTING))
		{
			bool inRange = false;
			for (const KBrushVertex& v : Vertices)
			{
				if (light->PointInRange(v.Point))
				{
					inRange = true;
					break;
				}
			}
			if (!inRange) continue;
		}
		else if (!light->PolyBoundsInLightBouds(Poly)) continue;

		// check if normals face light
		if (!bSmoothed)
		{
			if (!light->CanHitPlane(Plane)) continue;
		}
		else
		{
			if (Surface & ESurfaceFlags::VERTEX_LIGHTING)
			{
				// since we're only lighting the vertices, just check the vertices
				for (const KBrushVertex& v : GetLitVertices())
				{
					if (light->PointInRange(v.Point))
					{
						valid = true;
						break;
					}
				}
			}
			else
			{
				// we can technically have 0 vertices that face the light, but have surface points that do face it
				// probably a better way to do this, but checking each point is fast enough
				for (TVector<KLuxel>& luxels : Lightmap)
				{
					if (valid) break;
					for (KLuxel& luxel : luxels)
					{
						if (luxel.IsOnPoly() && light->CanHitPlane(DPlane(luxel.Point3D, luxel.Normal)))
						{
							valid = true;
							break;
						}
					}
				}
			}
		}

		if (valid) ValidLights.push_back(light);
	}
}

void KBrushFace::BuildDirectLighting()
{
	// TODO radius isnt working


	bool allShadow = true;
	
	if (Surface & ESurfaceFlags::VERTEX_LIGHTING)
	{
		for (KLightEntity* light : ValidLights)
		{
			for (KBrushVertex& v : GetLitVertices())
			{
				const DVec3 toCenter = (Center - v.Point).GetNormalized() * .1;
				const DVec3 point = v.Point + toCenter 
					+ (Plane.Normal * (FaceAttribs ? FaceAttribs->LightPositionOffset : .25));

				DVec3 color;
				DVec3 gi(1, 1, 1);

				if (CalculateLightAtPoint(point, v.Normal, color, light, &gi))
				{
					v.GIScale *= gi;
					//luxel.AffectingLights.push_back(light);
					if (color.x > 0 || color.y > 0 || color.z > 0)
					{
						allShadow = false;
						v.Red += color.x;
						v.Green += color.y;
						v.Blue += color.z;
					}
				}
			}
		}
		
		return;
	}

	for (KLightEntity* light : ValidLights)
	{
		for (TVector<KLuxel>& luxels : Lightmap)
		{
			for (KLuxel& luxel : luxels)
			{
				//if (!luxel.bOnPoly) continue;

				DVec3 point = luxel.Point3D;
				DVec3 color;
				DVec3 gi(1, 1, 1);


				if (CalculateLightAtPoint(point, luxel.Normal, color, light, &gi))
				{
					if (color == DVec3(0, 0, 0))
						luxel.SetShadowed(true);

					luxel.GIScale *= gi;
					luxel.AffectingLights.push_back(light);
					if (color.x > 0 || color.y > 0 || color.z > 0)
					{
						allShadow = false;
						luxel.Color += color;
					}
				}
			}
		}
	}
	
	if (allShadow) return;
	bLightmapAllBlack = false;

	// if neighboring pixels have a large color difference, trace between them
	// this softens shadow edges
	bool needRadius = false; // true when a single pixel needs this
	const f64 tolerance = .05;

	for (u32 x = 0; x < Lightmap.size() - 1; x++)
	{
		for (u32 y = 0; y < Lightmap[x].size() - 1; y++)
		{
			DVec3 color = Lightmap[x][y].Color;
			if (!Lightmap[x][y].IsOnPoly()) continue;
			//if (!Poly.Contains(Lightmap[x][y].Point3D, 1)) continue;

			if (Lightmap[x][y + 1].IsOnPoly() && 
				DVec3::DifferenceBetweenColors(color, Lightmap[x][y + 1].Color) > tolerance)
			{
				Lightmap[x][y].SetNeedRadius(true);
				Lightmap[x][y + 1].SetNeedRadius(true);
				needRadius = true;
			}
			if (Lightmap[x + 1][y + 1].IsOnPoly() &&
				DVec3::DifferenceBetweenColors(color, Lightmap[x + 1][y + 1].Color) > tolerance)
			{
				Lightmap[x][y].SetNeedRadius(true);
				Lightmap[x + 1][y + 1].SetNeedRadius(true);
				needRadius = true;
			}
			if (Lightmap[x + 1][y].IsOnPoly() &&
				DVec3::DifferenceBetweenColors(color, Lightmap[x + 1][y].Color) > tolerance)
			{
				Lightmap[x][y].SetNeedRadius(true);
				Lightmap[x + 1][y].SetNeedRadius(true);
				needRadius = true;
			}
		}	
	}

	if (needRadius)
	{
		const f64 radius = LightmapResolution / 2;

		// at least one luxel needed a radius sample
		for (u32 x = 0; x < Lightmap.size() - 1; x++)
		{
			for (u32 y = 0; y < Lightmap[x].size() - 1; y++)
			{
				if (Lightmap[x][y].DoesNeedRadius())
				{
					u32 sampleCount = 1;
					for (f32 posX = Lightmap[x][y].Point2D.x - radius * .9; posX <= Lightmap[x][y].Point2D.x + radius; posX += radius * 1.9)
					{
						for (f32 posY = Lightmap[x][y].Point2D.y - radius * .9; posY <= Lightmap[x][y].Point2D.y + radius; posY += radius * 1.9)
						{
							DVec2 point2d(posX, posY);
							if (point2d.Equals(Lightmap[x][y].Point2D, .001)) continue;

							DVec3 point = Poly.MapFrom2D(point2d);
							if (!Poly.Contains(point, 9999, Plane.Normal)) continue;

							for (KLightEntity* light : ValidLights)
							{
								DVec3 color;
								CalculateLightAtPoint(point, GetNormalAtPoint(point), color, light, nullptr);

								if (color.x > 0 || color.y > 0 || color.z > 0)
								{
									allShadow = false;
									Lightmap[x][y].Color += color;
								}
							}
							sampleCount++;
						}
					}

					Lightmap[x][y].Color /= sampleCount;
				}
			}
		}
	}

	PadLightmap();
}

bool KBrushFace::CalculateLightAtPoint(const DVec3& point, const DVec3& norm, DVec3& outcolor, class KLightEntity* light, DVec3* gi)
{
	// unsmoothed faces already have their valid lights
	if (bSmoothed && !light->CanHitPlane(DPlane(point, norm))) return false;

	DVec3 power(1, 1, 1);
	if (light->bCastShadows) power = LightStrengthAtPoint(point, light);

	if (power.LengthSq() > 0)
	{
		// actual light calculation here
		DVec3 toLight = light->FromPoint(point);
		f64 dist = toLight.Length();
		toLight /= dist;
		f32 diffuseFactor = toLight | norm;

		if (diffuseFactor > 0)
		{
			f64 strength = light->Strength(diffuseFactor, dist);

			if (gi)
			{
				// probably dont need this but just to be safe
				f64 str = std::clamp(strength, 0.0, 1.0);

				*gi = Lerp(DVec3(1, 1, 1), light->GIScale, str);
			}

			DVec3 diffuse = light->Color * power;
			diffuse *= strength;
			outcolor = diffuse;
		}
	}

	return true;
}

DVec3 KBrushFace::LightStrengthAtPoint(const DVec3& point, class KLightEntity* light) const
{
	DVec3 power(1, 1, 1);
	GHitResult hit;
	hit.SearchCollision = ECollisionMask::WorldStatic;
	hit.TraceCollision = ECollisionMask::Light;
	if (!(Surface & ESurfaceFlags::NO_SHADOW_RECEIVE) && light->bCastShadows)
		TraceLine(GLineSegment((point + Plane.Normal).ToType<GFlt>(), light->GetPosition(point).ToType<GFlt>()), hit);
		//KMapCompiler::Get().BspTree->TraceLine(GLineSegment((point + Plane.Normal).ToType<GFlt>(), light->GetPosition(point).ToType<GFlt>()), &hit);
		                      // ^^^ i dont know why adding to the normal fixes fucked up artifacts
							  // the point was already moved away from the plane before, wtf
	
	return (hit.bHit && hit.Normal.Dot(point - light->GetPosition(point)) > 0) ? DVec3(0, 0, 0) : DVec3(1, 1, 1);
}

DVec3 KBrushFace::MovePointTowardCenter(const DVec3& point, f64 distance /*= 0.5*/) const
{
	return (Center - point).GetNormalized() * distance;
}

void KBrushFace::PadLightmap()
{
	i32 width = Lightmap.size();
	i32 height = Lightmap[0].size();

	for (i32 x = 0; x < width; x++)
	  for (i32 y = 0; y < height; y++)
		Lightmap[x][y].Color.Clamp( { 0, 0, 0 }, { 1, 1, 1 } );

	/*for (i32 x = 0; x < width; x++)
	{
		{
			bool on = false;
			GVec3 onColor;
			for (i32 y = 0; y < height; y++)
			{
				if (Lightmap[x][y].bOnPoly)
				{
					on = true;
					onColor = Lightmap[x][y].Color;
				}
				else if (on)
				{
					Lightmap[x][y].Color = onColor;
				}
			}
		}
		{
			bool on = false;
			GVec3 onColor;
			for (i32 y = height - 1; y >= 0; y--)
			{
				if (Lightmap[x][y].bOnPoly)
				{
					on = true;
					onColor = Lightmap[x][y].Color;
				}
				else if (on)
				{
					Lightmap[x][y].Color = onColor;
				}
			}
		}
	}

	for (i32 y = 0; y < height; y++)
	{
		{
			bool on = false;
			GVec3 onColor;
			for (i32 x = 0; x < width; x++)
			{
				if (Lightmap[x][y].bOnPoly)
				{
					on = true;
					onColor = Lightmap[x][y].Color;
				}
				else if (on)
				{
					Lightmap[x][y].Color = onColor;
				}
			}
		}
		{
			bool on = false;
			GVec3 onColor;
			for (i32 x = width - 1; x >= 0; x--)
			{
				if (Lightmap[x][y].bOnPoly)
				{
					on = true;
					onColor = Lightmap[x][y].Color;
				}
				else if (on)
				{
					Lightmap[x][y].Color = onColor;
				}
			}
		}
	}

	return;*/

	for (i32 x = 0; x < width; x += width - 1)
	{
		for (i32 y = 0; y < height; y++)
		{
			if (x == 0)
			{
				Lightmap[x][y].Color = Lightmap[x + 1][y].Color;
			}
			else
			{
				Lightmap[x][y].Color = Lightmap[x - 1][y].Color;
			}
		}
	}

	for (i32 y = 0; y < height; y += height - 1)
	{
		for (i32 x = 0; x < width; x++)
		{
			if (y == 0)
			{
				Lightmap[x][y].Color = Lightmap[x][y + 1].Color;
			}
			else
			{
				Lightmap[x][y].Color = Lightmap[x][y - 1].Color;
			}
		}
	}
}

void KBrushFace::RadiosityPass()
{
	if (Surface & ESurfaceFlags::VERTEX_LIGHTING)
	{
		for (KBrushVertex& v : Vertices)
		{
			const DVec3 toCenter = (Center - v.Point).GetNormalized() * .1;
			const DVec3 point = v.Point + toCenter
				+ (Plane.Normal * (FaceAttribs ? FaceAttribs->LightPositionOffset : .25));

			DVec3 color = GetRenderInterface()->CalculateRadiosityAtPoint(point, v.Normal, this) * v.GIScale;

			if (FaceAttribs)
			{
				color.x *= FaceAttribs->RedIntensity;
				color.y *= FaceAttribs->GreenIntensity;
				color.z *= FaceAttribs->BlueIntensity;
			}

			v.Red += color.x;
			v.Green += color.y;
			v.Blue += color.z;
		}

		return;
	}

	if (Lightmap.size() == 0) return;

	i32 width = Lightmap.size();
	i32 height = Lightmap[0].size();

	struct RadiositySample
	{
		u32 x, y;
		DVec3 Color;
		bool bSampled = false;
	};

	TVector<TVector<RadiositySample>> radiosity;
	radiosity.resize(width);
	for (i32 i = 0; i < width; i++)
		radiosity[i].resize(height);

	// init x y values
	for (u32 x = 0; x < width; x++)
	{
		for (u32 y = 0; y < height; y++)
		{
			radiosity[x][y].x = x;
			radiosity[x][y].y = y;
		}
	}

#if 0

	// dont sample at pad luxels
	u32 xStart = 1;
	u32 xEnd = width - 1;
	u32 yStart = 1;
	u32 yEnd = height - 1;

	// sampling function that will be used a lot
	const auto samplePoint = [&](u32 x, u32 y) -> void
	{
		DVec3 point = Lightmap[x][y].Point3D;
		DVec3 norm = Lightmap[x][y].Normal;;

		point += norm * (FaceAttribs ? FaceAttribs->LightPositionOffset : .25);
		radiosity[x][y].Color += GetRenderInterface()->CalculateRadiosityAtPoint(point, norm, this);
		radiosity[x][y].bSampled = true;	
	};

	// build initial samples
	// look for edge luxels first
	// scan each column from the top and then from the bottom
	for (u32 x = xStart; x < xEnd; x++)
	{
		// keep track of what we found
		i32 bottomY = -1;
		i32 topY = -1;

		// top down
		for (u32 y = yStart; y < yEnd; y++)
		{
			KLuxel& luxel = Lightmap[x][y];
			if (luxel.IsOnPolyEdge())
			{
				topY = y;
				samplePoint(x, y);
				break;
			}
		}

		// bottom up
		for (i32 y = yEnd - 1; y >= 0; y--)
		{
			KLuxel& luxel = Lightmap[x][y];
			if (Lightmap[x][y].IsOnPolyEdge())
			{
				// sample this only if we didnt just do it
				if (y != topY) samplePoint(x, y);

				bottomY = y;
				break;
			}
		}

		// make sure we got either 2 points or 0 points, 1 point is fucked up
		K_ASSERT((bottomY == -1) == (topY == -1), "only found one edge point in a column");
	}

	// assign sample values to all other nearby luxels on the same edge
	for (u32 x = xStart; x < xEnd; x++)
	{
		for (u32 y = yStart; y < yEnd; y++)
		{
			RadiositySample& rad = radiosity[x][y];
			if (rad.bSampled)
			{
				// we found a sample, check nearby luxels
				KLuxel& luxel = Lightmap[x][y];
				const DVec3& point = luxel.Point3D;

				for (u32 xx = xStart; xx < xEnd; xx++)
				{
					for (u32 yy = yStart; yy < yEnd; yy++)
					{
						KLuxel& compLux = Lightmap[xx][yy];
						if (point.DistanceSq(compLux.Point3D) < LightmapResolution &&
							compLux.EdgeIndex == luxel.EdgeIndex)
						{
							// this point is close to the same edge
							RadiositySample& compRad = radiosity[xx][yy];
							compRad.bSampled = true;
							compRad.Color = rad.Color;
						}
					}
				}
			}
		}
	}

	// TODO maybe spill edge colors outside poly for better bilinear filtering?
	// might be unnecessary since we pulled in nearby points anyway

	// sample every stepInterval number of pixels
	const u32 stepInterval = 4;
	for (u32 x = xStart; x < xEnd; x++)
	{
		// skip luxels that dont fall on the step interval
		if ( (x - xStart) % stepInterval != 0) continue;

		for (u32 y = yStart; y < yEnd; y++)
		{
			if ( (y - yStart) % stepInterval != 0) continue;

			// skip luxels that arent on the poly
			if (!Lightmap[x][y].IsOnPoly()) continue;

			RadiositySample& rad = radiosity[x][y];
			if (!rad.bSampled) samplePoint(x, y);
		}
	}

	// interpolate between samples
	// if the color difference is too big, sample in between

	// start going down step-interval-columns
	RadiositySample* lastSample = nullptr;
	for (u32 x = xStart; x < xEnd; x++)
	{
		if ( (x - xStart) % stepInterval != 0) continue;

		for (u32 y = yStart; y < yEnd; y++)
		{
			// dont color off poly
			if (!Lightmap[x][y].IsOnPoly()) continue;

			RadiositySample& rad = radiosity[x][y];


			if (rad.bSampled) 
			{
				if (lastSample)
				{
					// compare this sample with the last one
					// interpolate or sample in between
					
#if 0
					f64 colorDiff = DVec3::DifferenceBetweenColors(rad.Color, lastSample->Color);
					if (colorDiff > .1)
					{
						// sample
					}
					else
#endif
					{
						// interpolate
						for (u32 yy = lastSample->y + 1; yy < y; yy++)
						{
							RadiositySample& interp = radiosity[x][yy];
							f64 alpha = f64(yy - lastSample->y) / f64(y);
							DVec3 lerpColor = DVec3::Lerp(lastSample->Color, rad.Color, alpha);
							interp.Color = lerpColor;
							interp.bSampled = true;
						}
					}
				}

				lastSample = &rad;
			}
		}
	}

	// we now have fully colored columns
	// interpolate between them for each row
	for (u32 y = yStart; y < yEnd; y++)
	{
		for (u32 x = xStart; x < xEnd; x++)
		{
			if (!Lightmap[x][y].IsOnPoly()) continue;
			RadiositySample& rad = radiosity[x][y];

			if (rad.bSampled) 
			{
				if (lastSample)
				{
					for (u32 xx = lastSample->x + 1; xx < x; xx++)
					{
						RadiositySample& interp = radiosity[xx][y];
						f32 alpha = f32(xx - lastSample->x) / f32(x);
						DVec3 lerpColor = DVec3::Lerp(lastSample->Color, rad.Color, alpha);
						interp.Color = lerpColor;
						interp.bSampled = true;
					}
				}
				lastSample = &rad;
			}
		}
	}

#else 
#define PIXEL_STEP 8
	i32 lastX = 0;
	for (i32 x = 1; x < width - 1; x++)
	{
		if ((x + PIXEL_STEP) % (PIXEL_STEP + 1) != 0 && x != width - 2) continue;

		i32 lastY = 0;

		for (i32 y = 1; y < height - 1; y++)
		{
			if ((y + PIXEL_STEP) % (PIXEL_STEP + 1) != 0 && y != height - 2) continue;

			//if (!Lightmap[x][y].bOnPoly) continue;

			DVec3 point = Lightmap[x][y].Point3D;
			DVec3 norm = Lightmap[x][y].Normal;//GetNormalAtPoint(point);

			point += norm * (FaceAttribs ? FaceAttribs->LightPositionOffset : .25);
			radiosity[x][y].Color += GetRenderInterface()->CalculateRadiosityAtPoint(point, norm, this) * 1;
			radiosity[x][y].bSampled = true;


			if (y == 1 && lastX >= 1)
			{
				for (i32 i = lastX + 1; i < x; i++)
				{
					DVec3 interp = Lerp(radiosity[lastX][1].Color, radiosity[x][1].Color, f32(i - lastX) / f32(x - lastX));
					radiosity[i][1].Color = interp;
				}
			}
			else if (lastY >= 1)
			{
				for (i32 i = lastY + 1; i < y; i++)
				{
					
					DVec3 interp = Lerp(radiosity[x][lastY].Color, radiosity[x][y].Color, f32(i - lastY) / f32(y - lastY));
					radiosity[x][i].Color = interp;
				}

				if (lastX >= 1)
				{
					for (i32 i = lastX + 1; i < x; i++)
					{
						for (i32 yy = lastY + 1; yy <= y; yy++)
						{
							DVec3 interp = Lerp(radiosity[lastX][yy].Color, radiosity[x][yy].Color, f32(i - lastX) / f32(x - lastX));
							radiosity[i][yy].Color = interp;
						}
					}
				}
			}

			lastY = y;
		}

		lastX = x;
	}
#endif

	for (i32 x = 1; x < width - 1; x++)
	{
		for (i32 y = 1; y < height - 1; y++)
		{
			DVec3 c = radiosity[x][y].Color * Lightmap[x][y].GIScale;

			if (c != DVec3(0, 0, 0)) bLightmapAllBlack = false;

			if (FaceAttribs)
			{
				c.x *= FaceAttribs->RedIntensity;
				c.y *= FaceAttribs->GreenIntensity;
				c.z *= FaceAttribs->BlueIntensity;
			}

			Lightmap[x][y].Color += c;
		}
	}

	PadLightmap();
}

void KBrushFace::BuildDirectLighting_GPU()
{
	
}

#endif