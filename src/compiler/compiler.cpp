
#include "compiler.h"
#include "engine/utility/kstring.h"
#include "engine/system/terminal/terminal_progress.h"
#include "entity.h"
#include "engine/system/terminal/terminal.h"
#include "brush/brush_face.h"
#include <mutex>
#include <future>
#include "bsp/bsp_tree.h"
#include "bsp/bsp_node.h"
#include "engine/utility/thread_pool.h"
#include "engine/run/k_main.h"
#include "light/light_ent.h"
#include "light/light_point.h"
#include "light/light_para.h"
#include "light/light_spot.h"
#include "light/face_attribs.h"
#include "engine/render/surface2d.h"

// DELETE (collision shouldnt be made in here)
#include "../engine/collision/brush.h"
#include "../engine/collision/broadphase/bvh_grid.h"
#include "../engine/game_instance.h"
#include "../engine/render/interface/render_interface.h"
#include "../engine/utility/bin_pack.h"
#include "engine/render/precipitation.h"
#include "../engine/render/interface/texture2d.h"
#include "surface_flags.h"
#include "../engine/render/vertex_types.h"
#include "../engine/render/scene.h"
#include "map_file.h"
#include "engine/render/material.h"

#if COMPILER_SHOW_WINDOW
#include "engine/render/interface/window.h"
#endif

static u32 FileLine = 0;
static u32 FileLineCount = 0;
static KTerminalProgressBar ParseProgress;
static u32 NextEntID = 1;

#define SECTION_HEADER "----------------------------------"

KMapCompiler::~KMapCompiler() {}

KMapCompiler& KMapCompiler::Get()
{
	static KMapCompiler instance;
	return instance;
}

static KString MapFileName;

void KMapCompiler::CompileMap(const KString& path)
{
	KSystemTerminal::Show();
	
	std::ifstream f(path.CStr());
	if (!f.good())
	{
		SYSLOG("Could not open file: " << path);
		return;
	}
	f.close();

	KTimePoint compileStart = KTime::Now();

	Grid = new KBvhGrid(512, 96);

	RenderInterface = KRenderInterface::CreateD3D11();

	RenderInterface->InitializeInterface();
	// InitializeInterface spawns the render thread 
	// wait for renderer tell us that thread has finished init
	while (!RenderInterface->IsInitialized()) {}

	GetRenderInterface()->WorldScene.reset();
	GetRenderInterface()->WorldScene = std::make_unique<KRenderScene>();
	GetRenderInterface()->SkyboxScene.reset();
	GetRenderInterface()->SkyboxScene = std::make_unique<KRenderScene>();
	
	// extracts and organizes data from a .map file
	ParseFile(path);
	//ParseFile("D:/karnagefreak/res/maps/lighttest.map");

	for (auto& ent : ParsedEntities)
	  if (ent->GetName() == "worldspawn")
		ProcessWorldSpawn(ent.get());

	// creates actual brush geometry from the planes that were parsed
	CreateBrushFaces();

	MergeRenderableEntityBrushes();
	
	// adjusts normals of vertices that are part of a smoothing group and touching a neighbor
	BuildSmoothingGroups();

	// gets all the static world brushes into one array "WorldBrushes"
	//CombineWorldBrushes();

	// removes faces that are inside brushes or sitting in out of bounds areas
	RemoveJunkFaces();

	MergeTextures();

	BuildSpatialGrid();
	CreateEntities();

	// calculates all static lighting data	
	BuildLightmaps();

	WriteFile();
	
	f64 endTime = KTime::Since(compileStart);
	SYSLOG("Map compilation took " << std::fixed << std::setprecision(3) << endTime << " seconds");
#if _WIN32
	system("PAUSE");
#endif
}

void KMapCompiler::ParseFile(const KString& path)
{
	// files are basically lists of entities

	KString fixed = path.FixDirectory();
	TVector<KString> folders;
	fixed.SplitByChar(folders, '/');
	SYSLOG("Compiling " << folders[folders.size() - 1] << "\n");

	TVector<KString> mapName;
	folders[folders.size() - 1].SplitByChar(mapName, '.');
	MapFileName = mapName[0];

	ParseProgress = KTerminalProgressBar("Parsing Map File");
	FileLine = 0;
	FileLineCount = 0;

	std::ifstream file(fixed.CStr());
	std::string l;
	{
		std::string s;
		while (!file.eof())
		{
			std::getline(file, s);
			FileLineCount++;
		}
		file.clear();
		file.seekg(0, std::ios::beg);
	}

	while (std::getline(file, l))
	{
		UpdateParseProgress();

		KString line(l);
		line.TrimInPlace();

		// skip comments and empty lines
		if (line.StartsWith("//") || line.IsEmpty()) continue;

		if (line.StartsWith("{"))
		{
			// if we arent parsing an entity already, open brace means a new one
			// create a new entity in our vector
			ParsedEntities.push_back(std::make_unique<KMapEntity>());
			ParsedEntities[ParsedEntities.size() - 1]->CompiledID = NextEntID;
			NextEntID++;
			ParsedEntities[ParsedEntities.size() - 1]->ParseEntity(file);			
		}
	}

	ParseProgress.Finish();
}

void KMapCompiler::CreateBrushFaces()
{
	for (UPtr<KMapEntity>& ent : ParsedEntities)
	{
		ent->CreateBrushFaces();

		for (const UPtr<KMapBrush>& brush : ent->GetBrushes())
		{
			// update the global map bounds from the new geometry
			KMapCompiler::Get().MapBounds.Update(brush->GetBoundingBox());
		}
	}
}

/*
void KMapCompiler::CombineWorldBrushes()
{
	for (auto& ent : ParsedEntities)
	  if (ent->HasStaticWorldBrushes())
		ent->MoveBrushesToVector(WorldBrushes);
}*/

void KMapCompiler::CreateEntities()
{
	u32 precCount = 0;
	

	// lights are created during the lightmapping stage
	for (auto& ent : ParsedEntities)
	{
		if (ent->GetName() == "launch_volume")
		{
			
		}
		else if (ent->GetName() == "precipitation_volume")
		{
			TVector<UPtr<KMapBrush>> brushes;
			ent->MoveBrushesToVector(brushes);

			TVector<GPolygon> polys;
			for (UPtr<KMapBrush>& b : brushes)
			  for (UPtr<KBrushFace>& face : b->Faces)
				polys.push_back(face->Poly.ToType<GFlt>());
				 
			UPtr<KPrecipitationVolume> prec = std::make_unique<KPrecipitationVolume>();
			prec->Density = ent->GetProperty("density").ToFloat();
			prec->Sway = ent->GetProperty("sway").ToFloat();
			prec->Speed = ent->GetProperty("speed").ToFloat();
			prec->Spacing = ent->GetProperty("spacing").ToFloat();
			prec->SetupVolume(polys, precCount);
			PendingPrecipitation.push_back(std::move(prec));
			precCount++;
		}
		else if (ent->GetName() == "skybox")
		{
			TempSky = ent.get();
		}
	}
}

void KMapCompiler::BuildSmoothingGroups()
{
	if (KApplication::HasCommandLineArg("nolight")) return;

	i32 smoothCounted = 0;
	std::mutex countMutex;
	TVector<std::future<void>> futures;



	const auto CalculateNormals = [&](KBrushFace* face1, i32 u) -> void
	{
		for (KBrushVertex& v1 : face1->Vertices)
		{
			for (KBrushFace* face2 : SmoothingGroups[u])
			{
				if (face1 == face2) continue;
				if (face1->Plane.Normal.Equals(face2->Plane.Normal, .001)) continue;
				bool found = false;

				for (KBrushVertex& v2 : face2->Vertices)
				{
					if (v1.Point.Equals(v2.Point, .1))
					{
						v1.AddSmoothedNormal(face2->Plane.Normal);
						found = true;
					}
				}

				// if we dont have a matching vertex with this face, see if the vertex falls on a line
				if (!found)
				{
					u32 edges = face2->Poly.NumEdges();
					for (u32 i = 0; i < edges; i++)
					{
						DLineSegment line = face2->Poly.Edge(i);
						if (line.Contains(v1.Point, .1))
						{
							v1.AddSmoothedNormal(face2->Plane.Normal);
						
							// smooth the line vertices on the other face
							KBrushVertex& vA = face2->Vertices[i];
							KBrushVertex& vB = face2->Vertices[(i + 1) % face2->Vertices.size()];
							vA.AddSmoothedNormal(face1->Plane.Normal);
							vB.AddSmoothedNormal(face1->Plane.Normal);
						}
					}
				}
			}
		}

		std::lock_guard<std::mutex> lock(countMutex);
		smoothCounted++;
	};

	KTerminalProgressBar progress("Smoothing Groups");

	u32 smoothed_count = 0;

	for (UPtr<KMapEntity>& ent : ParsedEntities)
	{
		for (UPtr<KMapBrush>& brush : ent->Brushes)
		{
			for (UPtr<KBrushFace>& face : brush->Faces)
			{
				bool smoothed = false;
				for (i32 i : face->SmoothingGroups)
				{
					if (i != 0)
					{
						SmoothingGroups[i].push_back(face.get());
						smoothed = true;
					}
				}

				if (smoothed) smoothed_count++;
			}
		}
	}

	for (const auto& kv : SmoothingGroups)
	{
		i32 u = kv.first;
		{
			if (u == 0) continue;

			for (KBrushFace* face : SmoothingGroups[u])
			{
				if (0)
				{
					futures.push_back(std::async(std::launch::async, CalculateNormals, face, u));
				}
				else
				{
					CalculateNormals(face, u);
					progress.UpdateProgress(smoothCounted, smoothed_count);
				}
			}
		}

		for (KBrushFace* face : SmoothingGroups[u])
		{
			for (KBrushVertex& v : face->Vertices)
			{
				if (v.PendingSmoothNormals.size() > 0)
				{
					DVec3 sum = v.Normal;
					for (const DVec3& n : v.PendingSmoothNormals) sum += n;
					v.Normal = sum.GetNormalized();
					face->bSmoothed = true;
					v.PendingSmoothNormals.clear();
				}
			}
		}
	}

	//if (CompilerSettings.bMultithreaded)
	while (smoothCounted < smoothed_count)
	{
		Sleep(20);
		progress.UpdateProgress(smoothCounted, smoothed_count);
	}

	progress.Finish();
}

void KMapCompiler::MergeRenderableEntityBrushes()
{
	for (UPtr<KMapEntity>& ent : ParsedEntities)
	  if (ent->GetName() == "worldspawn")
		WorldSpawn = ent.get();

	K_ASSERT(WorldSpawn, "no worldspawn parsed");

	for (i32 i = ParsedEntities.size() - 1; i >= 0; i--)
	{
		KMapEntity* ent = ParsedEntities[i].get();
		if (ent != WorldSpawn && ent->GetName() != "skybox" && ent->HasRenderableBrushes())
		{
			for (UPtr<KMapBrush>& brush : ent->Brushes)
			{
				if (brush->OwningEntity) 
				{
					brush->bMovedEntity = true;
					brush->OriginalEntityProperties = brush->OwningEntity->Properties;
				}

				brush->OwningEntity = WorldSpawn;
				WorldSpawn->Brushes.push_back(std::move(brush));
			}	

			ent->Brushes.clear();
			VectorRemoveAt(ParsedEntities, i);
		}
	}	
}

void KMapCompiler::RemoveInsideFaces()
{
	/*KTerminalProgressBar bar("Splitting faces...");
	u32 counts = 0;
	std::mutex mm;

	const auto IterBrushes = [&](u32 start, u32 end) -> void
	{
		for (u32 i = start; i < end; i++)
		{
			WorldBrushes[i]->RemoveFacesFromOtherBrushes();
			mm.lock();
			counts++;
			bar.UpdateProgress(counts, WorldBrushes.size());
			mm.unlock();
		}
	};

	KThreadPool::Iterate(IterBrushes, KApplication::GetCoreCount(), WorldBrushes.size());
	
	bar.Finish();

	for (UPtr<KMapBrush>& brush : WorldBrushes)
	{
		brush->Faces.clear();
		brush->Faces = std::move(brush->PendingNewFaces);
	}*/
}

void KMapCompiler::MergeTextures()
{
	// surfaces will be moved into a bigger surface atlas
	// keep surface placement tracked so faces can adjust texcoords
	struct SurfaceData
	{
		UPtr<KSurface2D> Surface;
		TVector<KBrushFace*> Faces;
		DVec2 AtlasPositionTopLeft; // texel offset
	};
	TMap<KString, SurfaceData> surfdata[2];
	
	// start filling surface data with surfaces and their faces
	// any textures that are no longer in use (due to face removal) wont be packed
	for (UPtr<KMapEntity>& ent : ParsedEntities)
	{
		u8 index = ent->GetName() == "skybox" ? 1 : 0;
		for (UPtr<KBrushFace>& face : ent->Faces)
		{
			if (!face->TexName.IsEmpty())
			{
				surfdata[index][face->TexName].Faces.push_back(face.get());
				if (!surfdata[index][face->TexName].Surface)
					surfdata[index][face->TexName].Surface =
					std::move(LoadedTextureSurfaces[index][face->TexName]);
			}
		}
	}

	for (u8 i = 0; i < 2; i++)
	{
		if (surfdata[i].size() == 0) continue;

		u32 total_pixels = 0;
		u32 maxW = 0, maxH = 0;
		TVector<KBinPacker::Rect> rects;
		for (auto& kv : surfdata[i])
		{
			u32 w = kv.second.Surface->GetPaddedWidth();
			u32 h = kv.second.Surface->GetPaddedHeight();
			if (w > maxW) maxW = w;
			if (h > maxH) maxH = h;

			total_pixels += w * h;

			KBinPacker::Rect rect;
			rect.Data = &kv.second;
			rect.size.x = w;
			rect.size.y = h;
			rects.push_back(rect);
		}

		if (total_pixels == 0) return;

		// sort by area
		std::sort
		(
			rects.begin(),
			rects.end(),
			[&](const KBinPacker::Rect& a, const KBinPacker::Rect& b) -> bool
			{
				return (a.size.x * a.size.y) < (b.size.x * b.size.y);
			}
		);

		// brute force attempts to fit within a size
		bool success = false;
		f64 scale = 1;
		while (!success)
		{
			KBinPacker packer;

			f64 sq = sqrt(total_pixels);
			if (sq < maxH) sq = maxH;
			if (sq < maxW) sq = maxW;

			packer.set_size( { u32(sq * scale), u32(sq * 1) } );

			for (KBinPacker::Rect& rect : rects)
			{
				success = packer.insert(rect, { rect.size.x, rect.size.y } );
				if (!success) break;
			}

			// if it didnt fit, try a little bigger
			scale += .01;
		}

		// find actual used dimensions
		u32 width = 0; u32 height = 0;
		for (KBinPacker::Rect& rect : rects)
		{
			if (rect.pos.x + rect.size.x > width)
				width = rect.pos.x + rect.size.x;
			if (rect.pos.y + rect.size.y > height)
				height = rect.pos.y + rect.size.y;
		}

		// create a new surface to merge all existing surfaces
		UPtr<KSurface2D> surf = std::make_unique<KSurface2D>(width, height);

		for (KBinPacker::Rect& rect : rects)
		{
			SurfaceData* data = (SurfaceData*)(rect.Data);
			for (u32 x = 0; x < data->Surface->GetPaddedWidth(); x++)
			  for (u32 y = 0; y < data->Surface->GetPaddedHeight(); y++)
				surf->SetPixel(x + rect.pos.x, y + rect.pos.y, data->Surface->GetPaddedPixel(x, y));
		}

		// adjust texcoords
		for (KBinPacker::Rect& r : rects)
		{
			for (KBrushFace* face : ((SurfaceData*)r.Data)->Faces)
			{
				for (KBrushVertex& v : face->Vertices)
				{
					f64 startX = f64(r.pos.x) / f64(width);
					f64 endX = f64(r.pos.x + r.size.x) / f64(width);
					f64 startY = f64(r.pos.y) / f64(height);
					f64 endY = f64(r.pos.y + r.size.y) / f64(height);
	      
					//v.TexU = MapRange(v.TexU, 0.f, 1.f, startX, endX);
					//v.TexV = MapRange(v.TexV, 0.f, 1.f, startY, endY);

					v.ClampU.x = startX;
					v.ClampU.y = endX;
					v.ClampV.x = startY;
					v.ClampV.y = endY;
				}	
			}
		}

		KRenderScene* scene = i == 1 ? GetRenderInterface()->GetSkyboxScene() 
									 : GetRenderInterface()->GetWorldScene();

		scene->TextureAtlas = GetRenderInterface()->CreateTexture2D();
		scene->TextureAtlas->CreateFromSurface(surf);
	}
}

void KMapCompiler::BuildLightmaps()
{
	if (KApplication::HasCommandLineArg("nolight"))
	{
		PackLightmaps();
		return;
	}
	//PackLightmaps();
	//return;

	SYSLOG_GREEN(SECTION_HEADER);
	SYSLOG_GREEN("Building Lighting");

	TVector<KBrushFace*> faces;
	TVector<u32> radiosityPasses;
	
	for (UPtr<KMapEntity>& ent : ParsedEntities)
	  if (ent->GetName() != "skybox")
		for (UPtr<KBrushFace>& f : ent->Faces)
		  faces.push_back(f.get());

	TVector<UPtr<KLightEntity>> lights;
	TVector<KLightEntity*> lightptrs;

	// get all light entities
	for (UPtr<KMapEntity>& ent : ParsedEntities)
	{
		KString name = ent->GetName();
		if (name.EndsWith("_light"))
		{
			if (name.StartsWith("point"))
			{
				if (UPtr<KLightEntity_Point> light = KLightEntity_Point::Create(ent.get()))
					lights.push_back(std::move(light));
			}
			else if (name.StartsWith("para"))
			{
				if (UPtr<KLightEntity_Parallel> light = KLightEntity_Parallel::Create(ent.get()))
					lights.push_back(std::move(light));
			}
			else if (name.StartsWith("spot"))
			{
				if (UPtr<KLightEntity_Spot> light = KLightEntity_Spot::Create(ent.get()))
					lights.push_back(std::move(light));
			}
		}
	}

	for (UPtr<KLightEntity>& light : lights)
		lightptrs.push_back(light.get());

	// prepare to be lit
	{
		KTerminalProgressBar progress("Preparing Faces");
		std::mutex m;
		u32 count = 0;

		const auto prepareFaces = [&](u32 start, u32 end) -> void
		{
			for (u32 i = start; i < end; i++)
			{
				KBrushFace* face = faces[i];
				face->SetupLightmapDimensions();
				face->FindValidLights(lightptrs);

				if (face->RadiosityPasses > radiosityPasses.size())
					radiosityPasses.resize(face->RadiosityPasses);

				for (u32 i = 0; i < face->RadiosityPasses; i++)
					radiosityPasses[i]++;

				m.lock();
				count++;
				progress.UpdateProgress(count, faces.size());
				m.unlock();
			}
		};

		KThreadPool::Iterate(prepareFaces, 0, faces.size());
		progress.Finish();
	}

	// separate from previous loop for better caching
	{
		KTerminalProgressBar progress("Direct Lighting");
		std::mutex m;
		u32 count = 0;

		const auto calculateLighting = [&](u32 start, u32 end) -> void
		{
			for (u32 i = start; i < end; i++)
			{
				faces[i]->BuildDirectLighting();

				m.lock();
				count++;
				progress.UpdateProgress(count, faces.size());
				m.unlock();
			}
		};

		KThreadPool::Iterate(calculateLighting, 0, faces.size());
		progress.Finish();
	}

	PackLightmaps();

	// global illumination
	if (!KApplication::HasCommandLineArg("noradiosity"))
	{
		// apply fog
		if (MapSettings.FogColor[3] > 0)
		{
			KFogBuffer f;
			memcpy(&f.Color, MapSettings.FogColor, sizeof(f32) * 4);
			f.Data.x = MapSettings.FogStart;
			f.Data.y = MapSettings.FogEnd;
			TVector<KFogBuffer> fog = { f };
			GetRenderInterface()->UpdateFog(fog);
		}

#if COMPILER_SHOW_WINDOW
		GetRenderInterface()->GetGameWindow()->Show();
#endif

		GetRenderInterface()->SetBuildingRadiosity(true);

		for (u32 i = 0; i < radiosityPasses.size(); i++)
		{
			KTerminalProgressBar rad(KString("Radiosity Pass ") + KString(i + 1) + "/" + KString(radiosityPasses.size()));

			u32 finished = 0;
			std::mutex mut;

			const auto radPasses = [&rad, &faces, &mut, &finished, &radiosityPasses, i](u32 start, u32 end) mutable -> void
			{
				for (u32 f = start; f < end; f++)
				{
					KBrushFace* face = faces[f];
					if (face->RadiosityPasses > i)
					{
						face->RadiosityPass();
						mut.lock();
						finished++;
						rad.UpdateProgress(finished, radiosityPasses[i]);
						mut.unlock();
					}
				}
			};

			KThreadPool::Iterate(radPasses, KApplication::GetCoreCount(), faces.size());

			// update renderer with new textures for next pass
			PackLightmaps();

			rad.Finish();
		}

		GetRenderInterface()->SetBuildingRadiosity(false);

#if COMPILER_SHOW_WINDOW
		GetRenderInterface()->GetGameWindow()->Hide();
#endif
	}

	// interpolate original vertex lighting to the actual ones
	for (KBrushFace* face : faces)
		face->InterpolateVertexLighting();
}

void KMapCompiler::PackLightmaps()
{
	u32 totalPixels = 0;
	u32 maxW = 0, maxH = 0;
	TVector<KBinPacker::Rect> rects;

	for (UPtr<KMapEntity>& ent : ParsedEntities)
	{
		for (UPtr<KBrushFace>& f : ent->Faces)
		{
			if (KApplication::HasCommandLineArg("nolight") || f->Surface & ESurfaceFlags::FULL_LIGHT)
			{
				for (KBrushVertex& vert : f->Vertices)
				{
					vert.Red = 1;
					vert.Green = 1;
					vert.Blue = 1;
				}
				continue;
			}

			if (!f->bLightmapAllBlack)
			{
				if (f->Lightmap.size() > maxW) maxW = f->Lightmap.size();
				if (f->Lightmap[0].size() > maxH) maxH = f->Lightmap[0].size();
				totalPixels += (f->Lightmap.size() * f->Lightmap[0].size());

				KBinPacker::Rect rect;
				rect.Data = f.get();
				rect.size.x = f->Lightmap.size();
				rect.size.y = f->Lightmap[0].size();
				rects.push_back(rect);
			}
			else
			{
				for (KBrushVertex& v : f->Vertices)
				{
					v.LightU = 1;
					v.LightV = 1;
				}
			}
		}
	}

	if (totalPixels == 0)
	{
		// something still needs to exist to be sampled, created 1x1 texture with black pixel

		UPtr<KSurface2D> surface = std::make_unique<KSurface2D>();
		surface->ClearAndResizeData(1, 1);
		//surface->SetPixel(0, 0, FColor8(255, 255, 255, 255));
		surface->SetPixel(0, 0, FColor8(0, 0, 0, 255));
		GetRenderInterface()->GetWorldScene()->LightmapTexture = GetRenderInterface()->CreateTexture2D();
		GetRenderInterface()->GetWorldScene()->LightmapTexture->CreateFromSurface(surface);

		// temporary and weird? why was this in here
		GetRenderInterface()->GetWorldScene()->SubmitFaces(GetWorldSpawn()->Faces);
		if (TempSky) GetRenderInterface()->GetSkyboxScene()->SubmitFaces(TempSky->Faces);

		return;
	}

	// sort by area
	for (u32 i = 0; i < rects.size() - 1; i++)
	{
		for (u32 j = 0; j < rects.size() - i - 1; j++)
		{
			u32 a1, a2;
			a1 = rects[j].size.x * rects[j].size.y;
			a2 = rects[j + 1].size.x * rects[j + 1].size.y;
			if (a1 < a2)
			{
				KBinPacker::Rect temp = rects[j];
				rects[j] = rects[j + 1];
				rects[j + 1] = temp;
			}
		}
	}

	bool success = false;
	f32 scale = 1;
	while (!success)
	{
		KBinPacker packer;

		f32 sq = sqrt(totalPixels);
		if (sq < maxH) sq = maxH;
		if (sq < maxW) sq = maxW;

		packer.set_size({ u32(sq * scale), u32(sq * 1) });

		for (KBinPacker::Rect& rect : rects)
		{
			success = packer.insert(rect, { rect.size.x, rect.size.y });
			if (!success) break;
		}
		scale += .01;
	}

	// find actual used dimensions
	u32 width = 0, height = 0;
	for (KBinPacker::Rect& rect : rects)
	{
		if ((rect.pos.x + rect.size.x) > width)
			width = rect.pos.x + rect.size.x;
		if ((rect.pos.y + rect.size.y) > height)
			height = rect.pos.y + rect.size.y;
	}

	UPtr<KSurface2D> surface = std::make_unique<KSurface2D>();
	surface->ClearAndResizeData(width, height);

	for (KBinPacker::Rect& rect : rects)
	{
		KBrushFace* face = ((KBrushFace*)(rect.Data));
		for (u32 x = 0; x < face->Lightmap.size(); x++)
		{
			for (u32 y = 0; y < face->Lightmap[0].size(); y++)
			{
				DVec3 c = face->Lightmap[x][y].Color;

				// clamp to highest pixel
				f64 m = KMax(KMax(c.x, c.y), c.z);
				if (m > 1) c /= m;
				  
				FColor32 c32(c.x, c.y, c.z, 1);
				surface->SetPixel(x + rect.pos.x, y + rect.pos.y, c32.To8());
			}
		}
	}
	surface->SetPixel(width - 1, height - 1, FColor8(0, 0, 0, 1));

	//LightmapWidth = width;
	//LightmapHeight = height;
	GetRenderInterface()->GetWorldScene()->LightmapTexture = GetRenderInterface()->CreateTexture2D();
	
	GetRenderInterface()->GetWorldScene()->LightmapTexture->CreateFromSurface(surface);

	// adjust vertices
	for (KBinPacker::Rect& r : rects)
	{
		for (KBrushVertex& v : ((KBrushFace*)(r.Data))->Vertices)
		{
			f32 start_x = f32(r.pos.x) / f32(width);
			f32 end_x = f32(r.pos.x + r.size.x) / f32(width);
			f32 start_y = f32(r.pos.y) / f32(height);
			f32 end_y = f32(r.pos.y + r.size.y) / f32(height);
			v.LightU = MapRange(v.LocalLightU, 0.f, 1.f, start_x, end_x);
			v.LightV = MapRange(v.LocalLightV, 0.f, 1.f, start_y, end_y);
		}
	}

	GetRenderInterface()->GetWorldScene()->SubmitFaces(GetWorldSpawn()->Faces);
	if (TempSky) GetRenderInterface()->GetSkyboxScene()->SubmitFaces(TempSky->Faces);
}

void KMapCompiler::BuildSpatialGrid()
{
	SYSLOG_GREEN(SECTION_HEADER);
	SYSLOG_GREEN("Building Collision Data");

	SYSLOG("Creating Collision Brushes...");
	
	Grid->BuildFromPending();
}

void KMapCompiler::WriteFile()
{	
	KMapFile kmap;

	if (!WorldSpawn) return;
	KString ingameName = WorldSpawn->GetProperty("ingame_name");
	if (ingameName.Trim().IsEmpty()) ingameName = MapFileName;
	kmap.MapName = ingameName;

	if (KRenderScene* scene = GetRenderInterface()->GetWorldScene())
	{
		kmap.WorldVertices = scene->WorldVertices;
		kmap.WorldIndices = scene->WorldIndices;
		for (auto& call : scene->OpaqueDrawCalls)
		{
			KMapFile::DrawCall c;
			c.IndexCount = call.IndexCount;
			c.MaterialName = call.Material->MaterialName;
			c.NameSize = c.MaterialName.Size();
			c.PendingMaterialProperties = call.PendingMaterialProperties;
			kmap.WorldDrawCalls.push_back(c);
		}
		for (auto& call : scene->TransparentDrawCalls)
		{
			KMapFile::DrawCall c;
			c.IndexCount = call.IndexCount;
			c.MaterialName = call.Material->MaterialName;
			c.NameSize = c.MaterialName.Size();
			c.PendingMaterialProperties = call.PendingMaterialProperties;
			kmap.WorldDrawCalls.push_back(c);
		}
		for (auto& call : scene->WaterDrawCalls)
		{
			KMapFile::DrawCall c;
			c.IndexCount = call.IndexCount;
			c.MaterialName = call.Material->MaterialName;
			c.NameSize = c.MaterialName.Size();
			c.PendingMaterialProperties = call.PendingMaterialProperties;
			kmap.WorldDrawCalls.push_back(c);
		}

		if (scene->GetTextureAtlas())
			kmap.WorldTextureAtlas = scene->GetTextureAtlas()->GetSurface();

		if (scene->GetLightmap())
			kmap.WorldLightmap = scene->GetLightmap()->GetSurface();
	}
	
	if (KRenderScene* scene = GetRenderInterface()->GetSkyboxScene())
	{
		kmap.SkyVertices = scene->WorldVertices;
		kmap.SkyIndices = scene->WorldIndices;
		for (auto& call : scene->OpaqueDrawCalls)
		{
			KMapFile::DrawCall c;
			c.IndexCount = call.IndexCount;
			c.MaterialName = call.Material->MaterialName;
			c.NameSize = c.MaterialName.Size();
			kmap.SkyDrawCalls.push_back(c);
		}

		if (scene->GetTextureAtlas())
			kmap.SkyTextureAtlas = scene->GetTextureAtlas()->GetSurface();

		if (scene->GetLightmap())
			kmap.SkyLightmap = scene->GetLightmap()->GetSurface();
	}

	for (UPtr<KMapEntity>& ent : ParsedEntities)
		kmap.Entities.push_back(ent.get());

	memcpy(kmap.FogData, MapSettings.FogColor, sizeof(f32) * 4);
	kmap.FogData[4] = MapSettings.FogStart;
	kmap.FogData[5] = MapSettings.FogEnd;

	kmap.WriteFile(MapFileName);
}

void KMapCompiler::LoadFaceSurface(const KString& tex)
{
	for (u8 i = 0; i < 2; i++)
	{
		if (!LoadedTextureSurfaces[i].contains(tex))
		{
			UPtr<KSurface2D> surface = std::make_unique<KSurface2D>();
			surface->LoadPNG(tex/*, ESurfacePadMethod::Repeat*/);
			if (surface) LoadedTextureSurfaces[i][tex] = std::move(surface);
		}
	}
}

void KMapCompiler::RemoveJunkFaces()
{
	SYSLOG_GREEN(SECTION_HEADER);

	for (UPtr<KMapEntity>& ent : ParsedEntities)
		ent->RemoveJunkFaces();

	return;
	/*

	SYSLOG_GREEN("Removing Junk Faces");
	FTimePoint start = KTime::Now();

	// before removing junk, store faces that are NOT junk

	for (UPtr<KMapBrush>& brush : WorldBrushes)
	  for (UPtr<KBrushFace>& face : brush->Faces)
		if (face->Surface & ESurfaceFlags::NOT_JUNK)
		  KMapCompiler::Get().NonJunkFaces.push_back(*(face.get()));

	RemoveInsideFaces();

	TVector<UPtr<KBrushFace>> faces;
	for (UPtr<KMapBrush>& brush : WorldBrushes)
	{
		for (UPtr<KBrushFace>& face : brush->Faces)
		{
			faces.push_back(std::move(face));
		}
		brush->Faces.clear();
	}

	KBspTree tree(faces, false);

	if (!MapSettings.bSkipFloodFill)
		tree.FloodFill();

	std::function<void(KBspNode*)> GetFaces = [&](KBspNode* node)
	{
		if (!node) return;
		if (node->bIsLeaf && (node->bFilled || MapSettings.bSkipFloodFill))
		{
			for (UPtr<KBrushFace>& f : node->LeafFaces)
				WorldFaces.push_back(std::move(f));
		}
		else
		{
			GetFaces(node->Front.get());
			GetFaces(node->Back.get());
		}
	};

	GetFaces(tree.GetHead());

	// return faces to their brushes and prepare to merge 
	for (UPtr<KBrushFace>& face : WorldFaces)
	{
		if (face->GetOwningBrush())
			face->GetOwningBrush()->Faces.push_back(std::move(face));
	}
	WorldFaces.clear();

	for (UPtr<KMapBrush>& brush : WorldBrushes)
	{
		// TODO this function leaves faces that, when ALL merged together, can be convex
		//	but merging any two will not
		brush->MergeFaces();

		for (UPtr<KBrushFace>& face : brush->Faces)
			WorldFaces.push_back(std::move(face));

		brush->Faces.clear();
	}

	// replace split non-junk faces with original
	for (i32 i = WorldFaces.size() - 1; i >= 0; i--)
	  if (WorldFaces[i]->Surface & ESurfaceFlags::NOT_JUNK)
		VectorRemoveAt(WorldFaces, i);

	for (KBrushFace& face : NonJunkFaces)
	{
		UPtr<KBrushFace> newF = std::make_unique<KBrushFace>();
		*(newF.get()) = face;
		WorldFaces.push_back(std::move(newF));
	}
	NonJunkFaces.clear();

	f64 time = KTime::Since(start);
	SYSLOG_GREEN("Junk face removal took " << std::setprecision(3) << time << " seconds.");*/
}

void KMapCompiler::AddTexture(class KTexture2D* tex)
{
	if (VectorContains(UsedTextures, tex)) return;
	UsedTextures.push_back(tex);
}

void KMapCompiler::AddLitFace(KBrushFace* face)
{
	LitFaces.push_back(face);
}

void KMapCompiler::ProcessWorldSpawn(KMapEntity* ent)
{
	KString res = ent->GetProperty("lightmap_resolution");
	if (!res.IsEmpty())
	{
		f64 num = res.ToNum<f64>();
		if (num > 0) MapSettings.LightmapResolution = num;
	}

	KString skipFill = ent->GetProperty("no_flood_fill");
	if (!skipFill.IsEmpty() && skipFill != "0")
		MapSettings.bSkipFloodFill = true;

	KString fog = ent->GetProperty("fog_color");
	TVector<f32> fogValues = fog.ToFloatArray();
	if (fogValues.size() == 4)
	{
		MapSettings.FogColor[0] = KSaturate(fogValues[0]);
		MapSettings.FogColor[1] = KSaturate(fogValues[1]);
		MapSettings.FogColor[2] = KSaturate(fogValues[2]);
		MapSettings.FogColor[3] = KSaturate(fogValues[3]);
	}

	ent->GetProperty("fog_start").ToFloatSafe(MapSettings.FogStart);
	ent->GetProperty("fog_end").ToFloatSafe(MapSettings.FogEnd);
}

void KMapCompiler::AddFaceToSmoothingGroup(i32 group, KBrushFace* face)
{
	SmoothingGroups[group].push_back(face);
}

void KMapCompiler::UpdateParseProgress()
{
	FileLine++;
	if ((FileLine % 100) == 0)
		ParseProgress.UpdateProgress(FileLine, FileLineCount);
}
