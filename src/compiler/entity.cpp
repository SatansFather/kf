#include "entity.h"
#include "engine/utility/kstring.h"
#include "brush/brush.h"
#include "engine/utility/k_assert.h"
#include "compiler.h"
#include "engine/system/time.h"
#include "engine/system/terminal/terminal.h"
#include "surface_flags.h"
#include "engine/system/terminal/terminal_progress.h"
#include "engine/utility/thread_pool.h"
#include "bsp/bsp_tree.h"
#include "bsp/bsp_node.h"
#include "light/face_attribs.h"
#include "game/collision_mask.h"

static TVector<KString> CollisionBrushEntities
{
	"worldspawn",
	"func_group",
	"water_volume",
	"collision_block",
	"collision_pass",
	"launch_volume",
	"portal_volume",
	"damage_volume",
};

static TVector<KString> RenderableBrushEntities
{
	"worldspawn",
	"func_group",
	"skybox",
	"collision_pass",
	"water_volume",
	"portal_volume",
};

static TMap<KString, u32> EntityCollisionChannels
{
	{ "worldspawn",			ECollisionMask::WorldStatic },
	{ "func_group",			ECollisionMask::WorldStatic },
	{ "collision_block",	ECollisionMask::WorldStatic },
	{ "collision_pass",		ECollisionMask::WorldStatic },
	{ "water_volume",		ECollisionMask::Water },
	{ "launch_volume",		ECollisionMask::Launcher },
	{ "damage_volume",		ECollisionMask::Damage },
	{ "portal_volume",		ECollisionMask::Portal },
};

static TVector<KString> NonCompiledEntities
{
	"worldspawn",
	"point_light",
	"spot_light",
	"para_light",
	"collision_pass",
	"collision_block",
};

KMapEntity::KMapEntity() {}
KMapEntity::~KMapEntity() {}

KString KMapEntity::GetProperty(KString key) const
{
	if (!Properties.contains(key)) return KString("");
	return Properties.at(key);
}

class KString KMapEntity::GetName() const
{
	K_ASSERT(Classname != "", "entity without classname");
	return GetProperty("classname");
}

void KMapEntity::CreateBrushFaces(bool loadingMap /*= false*/)
{
	for (auto& brush : Brushes)
		brush->CreateFaces(loadingMap);
}

bool KMapEntity::HasCollisionBrushes() const
{
	if (Classname.Get() == "collision_pass")
	{
		// pass entities can pass some or all
		// if it will pass all, it can be ignored

		u8 player = 1, monster = 1, weapon = 1, corpse = 1, item = 1, light = 0, precip = 0;

		GetProperty("pass_player").ToU8Safe(player);
		if (player == 0) return true;

		GetProperty("pass_monster").ToU8Safe(monster);
		if (monster == 0) return true;

		GetProperty("pass_weapon").ToU8Safe(weapon);
		if (weapon == 0) return true;

		GetProperty("pass_corpse").ToU8Safe(corpse);
		if (corpse == 0) return true;

		GetProperty("pass_item").ToU8Safe(item);
		if (item == 0) return true;

		GetProperty("pass_light").ToU8Safe(light);
		if (light == 0) return true;

		GetProperty("pass_precip").ToU8Safe(precip);
		if (precip == 0) return true;

		return false;
	}

	return VectorContains(CollisionBrushEntities, Classname);
}

bool KMapEntity::HasRenderableBrushes() const
{
	if (Classname.Get() == "portal_volume")
		return GetProperty("render") != "0";

	return VectorContains(RenderableBrushEntities, Classname);
}

DVec3 KMapEntity::GetOrigin()
{
	return DVec3(OriginX, OriginY, OriginZ);
}

void KMapEntity::SetOrigin(const DVec3& o)
{
	OriginX = o.x;
	OriginY = o.y;
	OriginZ = o.z;
}

u32 KMapEntity::GetCollisionChannels() const
{
	if (EntityCollisionChannels.contains(GetName()))
		return EntityCollisionChannels[GetName()];

	return 0;
}

u32 KMapEntity::GetCollisionPass() const
{
	u32 pass = 0;
	u32 player = 1, monster = 1, weapon = 1, corpse = 1, item = 1, light = 0, precip = 0;
	if (GetName() == "collision_pass")
	{
		GetProperty("pass_player").ToU32Safe(player);
		GetProperty("pass_monster").ToU32Safe(monster);
		GetProperty("pass_weapon").ToU32Safe(weapon);
		GetProperty("pass_corpse").ToU32Safe(corpse);
		GetProperty("pass_item").ToU32Safe(item);
		GetProperty("pass_light").ToU32Safe(light);
		GetProperty("pass_precip").ToU32Safe(precip);
		if (player)  pass |= ECollisionMask::PlayerCharacter;
		if (monster) pass |= ECollisionMask::MonsterCharacter;
		if (weapon)  pass |= ECollisionMask::Weapon;
		if (corpse)  pass |= ECollisionMask::Gib;
		if (item)    pass |= ECollisionMask::Pickup;
		if (light)	 pass |= ECollisionMask::Light;
		if (precip)  pass |= ECollisionMask::Precipitation;
	}
	else if (GetName() == "collision_block")
	{
		GetProperty("block_player").ToU32Safe(player);
		GetProperty("block_monster").ToU32Safe(monster);
		GetProperty("block_weapon").ToU32Safe(weapon);
		GetProperty("block_corpse").ToU32Safe(corpse);
		GetProperty("block_item").ToU32Safe(item);
		GetProperty("block_light").ToU32Safe(light);
		GetProperty("block_precip").ToU32Safe(precip);
		if (!player)  pass |= ECollisionMask::PlayerCharacter;
		if (!monster) pass |= ECollisionMask::MonsterCharacter;
		if (!weapon)  pass |= ECollisionMask::Weapon;
		if (!corpse)  pass |= ECollisionMask::Gib;
		if (!item)    pass |= ECollisionMask::Pickup;
		if (!light)   pass |= ECollisionMask::Light;
		if (!precip)  pass |= ECollisionMask::Precipitation;
	}
	return pass;
}

void KMapEntity::Copy(KMapEntity& ent)
{
	ent.Properties = Properties;
	ent.Bounds = Bounds;
	ent.OriginX = OriginX;
	ent.OriginY = OriginY;
	ent.OriginZ = OriginZ;
	ent.Angle = Angle;
	ent.Classname = Classname;
	ent.CompiledID = CompiledID;
	ent.Brushes = std::move(Brushes);
	ent.Faces = std::move(Faces);
}

#if _COMPILER

void KMapEntity::MoveBrushesToVector(TVector<UPtr<KMapBrush>>& vector)
{
	for (auto& brush : Brushes)
		vector.push_back(std::move(brush));

	Brushes.clear();
}

bool KMapEntity::ShouldBeCompiled() const
{
	return !VectorContains(NonCompiledEntities, Classname);
}

bool KMapEntity::ShouldCompileBrushes() const
{
	return false;
}

void KMapEntity::FinalizeParse()
{
	// set origin and angle if we have them

	if (Properties.contains("origin"))
	{
		KString origin = Properties["origin"];
		TVector<KString> vals;
		origin.SplitByChar(vals, ' ');

		K_ASSERT(vals.size() == 3, "entity origin did not contain 3 components");

		OriginX = vals[0].ToNum<f64>();
		OriginY = vals[1].ToNum<f64>();
		OriginZ = vals[2].ToNum<f64>();
	}

	if (Properties.contains("angle"))
	{
		Angle = Properties["angle"].ToNum<f64>();
	}

	if (Properties.contains("classname"))
	{
		Classname = Properties["classname"];
	}
	else
	{
		K_ASSERT(false, "cannot create entity without classname");
		return;
	}

	if (Classname.Get() == "face_attribs")
	{
		if (KFaceAttribs* att = KFaceAttribs::CreateNewAttribs(this))
			att->PropertyMap = Properties;
	}
}

void KMapEntity::ParseProperty(const KString& line)
{
	KString key, val, buffer;
	bool parsing = false;

	for (const char& c : line)
	{
		if (c == '\"')
		{
			parsing = !parsing;

			if (!parsing)
			{
				if (key.IsEmpty()) key = buffer;
				else val = buffer;

				buffer.Clear();
			}

			continue;
		}

		if (parsing)
		{
			buffer += c;
		}
	}

	K_ASSERT(!key.IsEmpty(), "attempted to parse line with empty key");
	K_ASSERT(!val.IsEmpty(), "attempted to parse line with empty value");

	// set the origin and angle if we have it
	Properties[key] = val;
}

void KMapEntity::ParseEntity(std::ifstream& file)
{
	// continue to increment the file cursor
	std::string l;
	while (std::getline(file, l))
	{
		KMapCompiler::Get().UpdateParseProgress();

		// start with quote means parse a property
		// start with open brace means parse a brush

		KString line(l);
		line.TrimInPlace();

		if (line.StartsWith("//") || line.IsEmpty()) continue;

		if (line.StartsWith("\""))
		{
			ParseProperty(line);
			continue;
		}
		else if (line.StartsWith("{"))
		{
			Brushes.push_back(std::make_unique<KMapBrush>());
			Brushes[Brushes.size() - 1]->OwningEntity = this;
			Brushes[Brushes.size() - 1]->ParseBrush(file);
			continue;
		}
		else if (line.StartsWith("}"))
		{
			break;
		}
	}
	// initializes some of the data that was parsed from the file
	FinalizeParse();
}


void KMapEntity::RemoveJunkFaces()
{
	if (!HasRenderableBrushes()) return;

	SYSLOG_GREEN("Removing Junk Faces From \"" << Classname << "\"");
	KTimePoint start = KTime::Now();

	// copy non junk faces so we can restore them later
	TVector<KBrushFace> nonJunk;
	for (UPtr<KMapBrush>& brush : Brushes)
	  for (UPtr<KBrushFace>& face : brush->Faces)
		if (face->Surface & ESurfaceFlags::NOT_JUNK)
		  nonJunk.push_back(*(face.get()));
	
	// remove geometry from the inside of other brushes
	RemoveInsideFaces();

	// remove faces that can never be seen from the playable area
	RemoveUnreachableFaces();

	// merge brush faces that share a plane if the result can be convex
	MergeBrushFaces();

	// remove non junk faces that have been split
	for (i32 i = Faces.size() - 1; i >= 0; i--)
	  if (Faces[i]->Surface & ESurfaceFlags::NOT_JUNK)
		VectorRemoveAt(Faces, i);

	// restore original non junk faces
	for (KBrushFace& face : nonJunk)
	{
		UPtr<KBrushFace> newF = std::make_unique<KBrushFace>();
		*(newF.get()) = face;
		Faces.push_back(std::move(newF));
	}

	f64 time = KTime::Since(start);
	SYSLOG_GREEN("Junk face removal from \" " << Classname << "\" took " << std::setprecision(3) << time << " seconds.");
}

void KMapEntity::RemoveInsideFaces()
{
	KTerminalProgressBar bar("Splitting faces...");
	u32 counts = 0;
	std::mutex mm;

	const auto iterBrushes = [&](u32 start, u32 end) -> void
	{
		for (u32 i = start; i < end; i++)
		{
			Brushes[i]->RemoveFacesFromOtherBrushes(Brushes);
			mm.lock();
			counts++;
			bar.UpdateProgress(counts, Brushes.size());
			mm.unlock();
		}
	};

	KThreadPool::Iterate(iterBrushes, KApplication::GetCoreCount(), Brushes.size());

	bar.Finish();

	for (UPtr<KMapBrush>& brush : Brushes)
	{
		brush->Faces.clear();
		brush->Faces = std::move(brush->PendingNewFaces);
	}
}

void KMapEntity::RemoveUnreachableFaces()
{
	if (KMapCompiler::Get().MapSettings.bSkipFloodFill) return;

	if (Classname == "skybox") return;

	// create a bsp tree so we can perform a flood fill
	TVector<UPtr<KBrushFace>> faces;
	for (UPtr<KMapBrush>& brush : Brushes)
	{
		if (brush->IsPenetrable()) continue;

		for (UPtr<KBrushFace>& face : brush->Faces)
			faces.push_back(std::move(face));

		brush->Faces.clear();
	}

	//KBspTree tree(faces, false);
	KMapCompiler::Get().BspTree = new KBspTree(faces, false);
	KMapCompiler::Get().BspTree->FloodFill();

	// return faces to brushes
	std::function<void(KBspNode*)> GetFaces = [&](KBspNode* node)
	{
		if (!node) return;
		if (node->IsFilledLeaf())
		{
			for (UPtr<KBrushFace>& f : node->GetLeafFaces())
			{
				if (f->GetOwningBrush())
					f->GetOwningBrush()->Faces.push_back(std::move(f));
			}
		}
		else
		{
			GetFaces(node->GetFront());
			GetFaces(node->GetBack());
		}
	};

	GetFaces(KMapCompiler::Get().BspTree->GetHead());
}

void KMapEntity::MergeBrushFaces()
{
	for (UPtr<KMapBrush>& brush : Brushes)
	{
		brush->MergeFaces();

		// let the entity own the brush faces, no longer need them as part of a brush
		for (UPtr<KBrushFace>& face : brush->Faces)
			Faces.push_back(std::move(face));

		brush->Faces.clear();
	}
}

#endif