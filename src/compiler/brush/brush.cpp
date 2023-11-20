

#include "brush.h"
#include "engine/utility/kstring.h"
#include "engine/math/glm.h"
#include "brush_face.h"
#include "../compiler.h"
#include <future>
#include "engine/utility/thread_pool.h"
#include "engine/run/k_main.h"
#include "../light/face_attribs.h"
#include "engine/collision/brush.h"
#include "../surface_flags.h"
#include "engine/collision/broadphase/bvh_grid.h"

#if !_COMPILER
#include "engine/game/match.h"
#endif
#include "../../game/collision_mask.h"

#define	NORMAL_EPSILON	0.00001
#define	DIST_EPSILON	0.01

KMapBrush::KMapBrush() {}
KMapBrush::~KMapBrush() {}

void KMapBrush::CreateFaces(bool loadingMap /*=false*/)
{
	if (loadingMap)
	{
		// recreate faces
		for (const DPlane& p : Planes)
		{
			UPtr<KBrushFace> face = std::make_unique<KBrushFace>();
			face->Plane = p;
			Faces.push_back(std::move(face));
		}
	}

	// we have the planes that shape this brush, now create vertices

	Bounds.Reset();

	i32 faceCount = i32(Faces.size());
	TVector<DVec3> intersections;
	for (i32 i = 0; i < faceCount - 2; i++)
	{
		for (i32 j = i; j < faceCount - 1; j++)
		{
			for (i32 k = j; k < faceCount; k++)
			{
				if (i != j && i != k && j != k)
				{
					DVec3 intersection = DVec3(0, 0, 0);
					if (Faces[i]->Plane.Intersects(Faces[j]->Plane, Faces[k]->Plane, &intersection))
					{
						if (PointInsideBrush(intersection))
						{
							if (!VectorContains(intersections, intersection))
							{
								intersections.push_back(intersection);
								Vertices.push_back(intersection);

								Bounds.Update(intersection);
							}

							// only add unique vertices to polys
							auto AddUnique = [&](UPtr<KBrushFace>& face, KBrushVertex& v, DVec3& vertNorm)
							{
								for (KBrushVertex& vert : face->Vertices)
									if (vert.Point.Equals(v.Point))
										return;

								v.Normal = vertNorm;
								face->Vertices.push_back(v);
								face->OriginalVertices.push_back(v);
							};

							//intersection.x = RoundNearest(intersection.x, .001);
							//intersection.y = RoundNearest(intersection.y, .001);
							//intersection.z = RoundNearest(intersection.z, .001);
							KBrushVertex v(intersection);
							Bounds.Update(intersection);

							AddUnique(Faces[i], v, Faces[i]->Plane.Normal);
							AddUnique(Faces[j], v, Faces[j]->Plane.Normal);
							AddUnique(Faces[k], v, Faces[k]->Plane.Normal);
						}
					}
				}
			}
		}
	}

	// create pre-collision data from new faces
	// need to do this now because when collision actually happens, some faces are gone
	KPendingCollisionBrush col;
	col.OwningBrush = this;
	if (OwningEntity)
	{
		col.EntityID = OwningEntity->GetCompiledID();
		col.CollisionChannels = OwningEntity->GetCollisionChannels();
		col.CollisionPass = OwningEntity->GetCollisionPass();
	}

	for (auto& face : Faces)
	{
		face->InitFromVertices();
#if _COMPILER
		if (!loadingMap)
		{
			face->SurfaceIndex = OwningEntity && OwningEntity->GetName() == "skybox";

			if (KMapCompiler::Get().FaceAttribMap.contains(face->EntityID))
				face->FaceAttribs = KMapCompiler::Get().FaceAttribMap[face->EntityID].get();

			if (face->FaceAttribs)
				for (i32 i : face->FaceAttribs->ExtraSmoothingGroups)
					face->SmoothingGroups.push_back(i);

			KMapCompiler::Get().LoadFaceSurface(face->TexName);

			if (face->FaceAttribs && !face->FaceAttribs->Shader.IsEmpty())
			{
				face->SetMaterial(face->FaceAttribs->Shader);
			}
			else
			{
				if (OwningEntity && OwningEntity->GetName() == "water_volume")
				{
					face->SetMaterial("water");
					face->bVolumeMaterial = true;
				}
				else if (OwningEntity && OwningEntity->GetName() == "portal_volume")
				{
					face->SetMaterial("portal");
					face->bVolumeMaterial = true;
				}
			}

			face->DetermineTextureCoordinates();
		}
#endif
		col.Faces.push_back({ face->Plane.ToType<GFlt>(), face->Poly.ToType<GFlt>() });
		for (GVec3& v : face->Poly.ToType<GFlt>().Points) col.Bounds.Update(v);
	}

	if (OwningEntity && OwningEntity->HasCollisionBrushes())
	{
#if !_COMPILER
		if (loadingMap)
			GetGameMatch()->Grid->PendingCollision.push_back(col);

		// average all points to find position
		GVec3 pos;
		u32 vertCount = 0;
		for (auto& face : Faces)
		{
			for (GVec3& v : face->Poly.ToType<GFlt>().Points)
			{
				pos += v;
				vertCount++;
			}
		}
		pos /= vertCount;
		OwningEntity->SetOrigin(pos.ToType<f64>());
#else

		KMapCompiler::Get().Grid->PendingCollision.push_back(col);
#endif
	}
}

bool KMapBrush::PointInsideBrush(const DVec3& point)
{
	for (auto& face : Faces)
	{
		if ((face->Plane.Normal | (point - face->Plane.PointOnPlane())) > .01)
			return false;
	}

	return true;
}

#if _COMPILER

void KMapBrush::ParseBrush(std::ifstream& file)
{
	if (OwningEntity)
	{
		if (OwningEntity->GetName() == "water_volume")
		{
			bIsPenetrable = true;
			bIsWater = true;
		}
		else if (OwningEntity->GetName() == "collision_pass" &&
		         OwningEntity->GetCollisionPass() != ECollisionMask::Light)
		{
			bIsPenetrable = true;
		}
	}

	std::string l;
	while (std::getline(file, l))
	{
		KMapCompiler::Get().UpdateParseProgress();

		KString line(l);
		line.TrimInPlace();
		if (line.StartsWith("//") || line.IsEmpty()) continue;

		if (line.StartsWith("}"))
		{
			break;
		}
		else if (line.StartsWith("("))
		{
			UPtr<KBrushFace> face = std::make_unique<KBrushFace>();
			face->OwningBrush = this;

			KString current_number;
			TVector<DVec3> vectors;
			TVector<f64> numbers;

			bool in_vector = false;
			bool vector_ready = false;
			bool finished_vectors = false;

			KString texname, ux, uy, uz, Xoffset, vx, vy, vz, Yoffset, rotation, 
				Xscale, Yscale, content, surface, value, smoothing, entityID;

			KString buffer;
			KString* current;

			u16 index = 0;

			// entering new vector
			for (char c : line)
			{
				index++;

				if (c == '[' || c == ']') continue;

				// process vectors
				if (!finished_vectors)
				{
					if (c == '(')
					{
						in_vector = true;
						continue;
					}

					if (c == ' ')
					{
						if (in_vector)
						{
							// dump float if we have it
							if (!current_number.IsEmpty())
							{
								f64 num = current_number.ToNum<f64>();
								current_number = "";
								numbers.push_back(num);
								continue;
							}
						}
					}

					if (c == ')')
					{
						in_vector = false;
						if (numbers.size() == 3)
						{
							// between vectors, dump it				
							DVec3 new_point = DVec3(numbers[0], numbers[1], numbers[2]);

							vectors.push_back(new_point);
							numbers.clear();
							if (vectors.size() == 3)
							{
								DPlane plane(vectors[0], vectors[1], vectors[2]);

								// snap normal to axial value if very close
								/*for (i32 i = 0; i < 3; i++)
								{
									if (abs(plane.Normal[i] - 1) < NORMAL_EPSILON)
									{
										plane.Normal = 0;
										plane.Normal[i] = 1;
										break;
									}
									else if (abs(plane.Normal[i] + 1) < NORMAL_EPSILON)
									{
										plane.Normal = 0;
										plane.Normal[i] = -1;
										break;
									}
								}

								// snap distance to int if very close
								if (abs(plane.D - std::rint(plane.D)) < DIST_EPSILON)
									plane.D = std::rint(plane.D);*/

								bool matching = false;
								for (DPlane& p : KMapCompiler::Get().OriginalSurfacePlanes)
								{
									//if (p.Equals(plane, .0001, .001))
									if (p.Equals(plane, NORMAL_EPSILON, DIST_EPSILON))
									{
										plane.D = p.D;
										plane.Normal = p.Normal;
										matching = true;
										break;
									}
								}
								
								if (!matching)
								{
									KMapCompiler::Get().OriginalSurfacePlanes.push_back(plane);
									KMapCompiler::Get().OriginalSurfacePlanes.push_back(plane.Inverted());
								}

								Planes.push_back(plane);
								face->Plane = plane;
								face->BrushPlaneIndex = Planes.size() - 1;
								finished_vectors = true;
							}
							continue;
						}
					}

					if (in_vector)
					{
						current_number += c;
					}
				}
				else
				{
					// get texture info

					current = texname.Length() == 0 ? &texname :
						ux.Length() == 0 ? &ux :
						uy.Length() == 0 ? &uy :
						uz.Length() == 0 ? &uz :
						Xoffset.Length() == 0 ? &Xoffset :
						vx.Length() == 0 ? &vx :
						vy.Length() == 0 ? &vy :
						vz.Length() == 0 ? &vz :
						Yoffset.Length() == 0 ? &Yoffset :
						rotation.Length() == 0 ? &rotation :
						Xscale.Length() == 0 ? &Xscale :
						Yscale.Length() == 0 ? &Yscale :
						content.Length() == 0 ? &content :
						surface.Length() == 0 ? &surface :
						value.Length() == 0 ? &value :
						smoothing.Length() == 0 ? &smoothing :
						entityID.Length() == 0 ? &entityID
						: nullptr;

					if (current)
					{
						if (c == ' ' && buffer.Length() > 0)
						{
							*current = buffer;
							buffer = "";
							continue;
						}

						if (current->Length() == 0)
						{
							if (c != ' ')
							{
								buffer += c;
							}

							if (index == line.Length())
							{
								*current = buffer;
								buffer = "";
							}

							continue;
						}
					}
				}
			}

			face->TexName = texname;
			face->TexOffsetX = Xoffset.ToNum<f64>();
			face->TexOffsetY = Yoffset.ToNum<f64>();
			face->TexRotation = rotation.ToNum<f64>();
			face->TexScaleX = Xscale.ToNum<f64>();
			face->TexScaleY = Yscale.ToNum<f64>();
			face->AxisU = DVec3(ux.ToNum<f64>(), uy.ToNum<f64>(), uz.ToNum<f64>()) / face->TexScaleX;
			face->AxisV = DVec3(vx.ToNum<f64>(), vy.ToNum<f64>(), vz.ToNum<f64>()) / face->TexScaleY;

			face->Content = content.ToI32();
			face->Surface = surface.ToI32();
			face->EntityID = entityID.ToI32();

			face->LightmapResolution = value.ToNum<f64>();
			face->LightmapResolution = std::clamp(face->LightmapResolution, 0.0, 999999.0);

			face->AddSmoothingGroup(smoothing.ToI32());

			face->bIsCollisionPass = bIsPenetrable;

			Faces.push_back(std::move(face));
		}	
	}
}

void KMapBrush::RemoveFacesFromOtherBrushes(const TVector<UPtr<KMapBrush>>& brushSet)
{
	// start by creating a copy of each face
	// faces will be chopped and mutilated and tortured, but we need originals for later tests
	TVector<UPtr<KBrushFace>> brushFaces;
	for (UPtr<KBrushFace>& face : Faces)
	{
		KBrushFace* faceCopy = new KBrushFace;
		*faceCopy = *(face.get());
		brushFaces.push_back(UPtr<KBrushFace>(faceCopy));
	}

	// if faces are too big, divide them
	/*const f64 max_dimension = 512;

	// reverse because we will add to brush_faces in the loop
	for (i32 i = brush_faces.size() - 1; i >= 0; i--)
		brush_faces[i]->DivideLargeFace(max_dimension, brush_faces);*/

	bool overwrite = false;

	for (const UPtr<KMapBrush>& clippingBrush : brushSet)
	{
		KMapBrush* clipBrush = clippingBrush.get();

		if (clipBrush == this) 
		{
			// overlapping faces will not cancel each other out
			// instead the later occurring face will take the place of the older one, only one survives
			overwrite = true;
			continue;
		}

		if (clipBrush->IsWater() && !IsWater()) continue;
		if (clipBrush->IsPenetrable() && !IsPenetrable()) continue;
	
		// if this brush has any transparent faces, dont let it clip
		// TODO the transparent face might be removed, leaving only non transparent faces
		bool skip = false;
		for (UPtr<KBrushFace>& face : clipBrush->Faces)
		{
			if (face->HasTransparency()/* && !IsPenetrable()*/) skip = true;
			if (face->GetSurface() & (u32)ESurfaceFlags::NO_DRAW) skip = true;

			// TODO check other crap that should prevent clipping
			if (skip) break;
		}
		if (skip) continue;

		// dont check this brush if it isnt close
		if (!Bounds.Overlaps(clipBrush->Bounds, .25)) continue;

		// store our new faces here and move them into brush_faces later if theyre outside
		TVector<UPtr<KBrushFace>> newFaces;

		std::mutex m;

		// std::function allows us to recursively split new faces
		std::function<void(KBrushFace*)> SplitFace = [&](KBrushFace* face)
		{
			TVector<std::future<void>> futures;
			for (UPtr<KBrushFace>& splitter : clipBrush->Faces)
			{
				//if (!face->BoundingBoxesOverlap(splitter.get(), .25)) continue;
				if (!face->Bounds.Overlaps(splitter->Bounds, .25)) continue;

				EPolyClassification polyclass = face->Poly.ClassifyToPlane(splitter->Plane, .001);
				
				if (polyclass == EPolyClassification::Coplanar)
				{
					// coplanar face is inside only if it is facing opposite
					//if ((face->Plane.Normal | splitter->Plane.Normal) < 0) clipped.push_back(face);
					continue;
				}
				else if (polyclass == EPolyClassification::Spanning)
				{
					// only a poly that crosses over the splitter's plane could possibly be bisected by it
					// does not yet mean the polygons overlap but this filters out a lot of checks

					//if (face->Poly.Intersects(splitter->Poly, 0))
					{
						m.lock();
						auto splitface = face->SplitByPlane(splitter->Plane);
						newFaces.push_back(std::move(splitface));
						KBrushFace* split = newFaces[newFaces.size() - 1].get();
						m.unlock();

						// recursively split the new face
						// the current face is now smaller and will continue to be split within this loop
						SplitFace(split);
						//futures.push_back(std::async(std::launch::async, SplitFace, split));
					}
				}
			}
		};

		// multi thread brushes with huge face counts (usually due to division)
		if (false && brushFaces.size() > 64)
		{
			const auto SplitMT = [&](u32 start, u32 end) -> void
			{
				for (u32 i = start; i < end; i++)
				{
					SplitFace(brushFaces[i].get());
				}
			};
			u32 threadcount = KApplication::GetCoreCount();
			threadcount = std::clamp(threadcount, 1u, 12u);
			KThreadPool::Iterate(SplitMT, threadcount, brushFaces.size());
		}
		else
		{
			for (auto& face : brushFaces)
			  //if (!(face->Surface & ESurfaceFlags::NOT_JUNK))
				SplitFace(face.get());
		}

		for (auto& newf : newFaces)
			brushFaces.push_back(std::move(newf));

		for (i32 i = brushFaces.size() - 1; i >= 0; i--)
		{
			bool behind = true;
			for (auto& splitter : clipBrush->Faces)
			{
				if (splitter->Plane.IsCoplanar(brushFaces[i]->Plane)
					&& (splitter->Plane.Normal | brushFaces[i]->Plane.Normal) > 0)
				{
					// these faces overlap and face the same direction, only keep one
					// skipping this check would delete both
					behind = overwrite;
				}
				else if (splitter->Plane.ClassifyPoint(brushFaces[i]->Center) == EPointSide::Front)
				{
					behind = false;
					break;
				}
			}
			if (behind) VectorRemoveAt(brushFaces, i);
		}
	}

	// hold faces in PendingNewFaces until all brushes have been clipped
	for (UPtr<KBrushFace>& face : brushFaces)
		PendingNewFaces.push_back(std::move(face));
}

void MergePlaneFaces(TVector<UPtr<KBrushFace>>* fv, bool planeSplit)
{
	// fv represents a vector of faces on one plane
	TVector<UPtr<KBrushFace>>& faces = *fv;

	// check edges of faces for matches
	// merge two faces if they form a convex polygon
	for (i32 i = 0; i < faces.size(); i++)
	{
		for (i32 j = 0; j < faces.size(); j++)
		{
			if (i == j) continue;

			if (UPtr<KBrushFace> newface = faces[i]->MergeWith(faces[j].get()))
			{
				// remove i and j since theyve been merged
				// start with higher one so shifting doesnt fuck it all up
				VectorRemoveAt((faces), (std::max)(i, j));
				VectorRemoveAt((faces), (std::min)(i, j));

				faces.push_back(std::move(newface));

				// swap new face with index 0 to keep trying immediately
				std::swap(faces[fv->size() - 1], faces[0]);

				i = -1; // start over
				break;
			}
		}
	}

	if (faces.size() == 1 || !planeSplit) return;

	// create a plane from every line of every face
	TVector<DPlane> linePlanes;
	for (UPtr<KBrushFace>& face : faces)
	{
		for (u32 i = 0; i < face->Poly.NumEdges(); i++)
		{
			DLineSegment line = face->Poly.Edge(i);
			//DVec3 sideNorm = face->Plane.Normal.Cross(line.DirectionUnNormalized());
			DPlane p(line.a, line.b, line.b + face->Plane.Normal);

			bool found = false;
			for (const DPlane& plane : linePlanes)
			  if (p.Equals(plane) || p.EqualsInverse(plane))
				found = true;

			if (!found) linePlanes.push_back(p);
		}
	}
	
	// split every face by those planes
	for (u32 i = 0; i < faces.size(); i++)
	{
		for (const DPlane& split : linePlanes)
		{
			UPtr<KBrushFace>& face = faces[i];
			if (face->Poly.ClassifyToPlane(split, .001) == EPolyClassification::Spanning)
			{
				UPtr<KBrushFace> splitface = face->SplitByPlane(split);
				faces.push_back(std::move(splitface));
			}
		}
	}

	// try again
	MergePlaneFaces(fv, false);
}

void KMapBrush::MergeFaces()
{
	// groups of faces will match the index of their plane
	// faces will be moved from Faces
	TVector<TVector<UPtr<KBrushFace>>> faces;
	faces.resize(Planes.size());
	u32 remaining = Faces.size();
	for (i32 i = Faces.size() - 1; i >= 0; i--)
	{
		UPtr<KBrushFace>& face = Faces[i];
		bool found = false;
		for (u32 i = 0; i < Planes.size(); i++)
		{
			if (face->BrushPlaneIndex == i)
			{
				found = true;
				faces[i].push_back(std::move(face));
				remaining--;
				break;
			}
		}
		K_ASSERT(found, "could not find matching plane for face with normal " + face->Plane.Normal.ToString());
	}

	// brush_faces should be empty
	K_ASSERT(remaining == 0, "did not move all faces out of Faces vector");
	Faces.clear();

	// try to merge this faces of this brush that are on the same plane
	{
		TVector<std::future<void>> futures;
		for (auto& fv : faces)
		{
			if (fv.size() > 0 && !(fv[0]->Surface & ESurfaceFlags::NOT_JUNK))
				futures.push_back(std::async(std::launch::async, MergePlaneFaces, &fv, true));
		}
	}

	for (auto& fv : faces)
	  for (i32 i = fv.size() - 1; i >= 0; i--)
	    Faces.push_back(std::move(fv[i]));
}

UPtr<KMapBrush> KMapBrush::CopyExtendedBrush(DVec3 extent)
{
	// extended brushes allow a bsp tree to created for line tests representing an AABB
	// we really just need to copy the planes to make faces
	// and face properties ( do we really need any properties for collision? )

	UPtr<KMapBrush> newbrush = std::make_unique<KMapBrush>();
	newbrush->Planes = Planes;

	for (DPlane& p : newbrush->Planes)
		//p = DPlane( (p.PointOnPlane() + (p.Normal * extent)), p.Normal );
		p.D += (extent.x * abs(p.Normal.x))
			 + (extent.y * abs(p.Normal.y))
			 + (extent.z * abs(p.Normal.z));

	for (UPtr<KBrushFace>& face : Faces)
	{
		UPtr<KBrushFace> newface = std::make_unique<KBrushFace>();
		*(newface.get()) = *(face.get());
		newface->Plane.D += (extent.x * abs(newface->Plane.Normal.x))
						  + (extent.y * abs(newface->Plane.Normal.y))
						  + (extent.z * abs(newface->Plane.Normal.z));

		newface->Vertices.clear();
		newface->Triangles.clear();
		newface->Indices.clear();
		newface->Poly.Points.clear();

		newbrush->Faces.push_back(std::move(newface));
	}

	newbrush->CreateFaces();

	//KMapCompiler::Get().MapBounds.Update(newbrush->Bounds);

	return newbrush;
}

#endif