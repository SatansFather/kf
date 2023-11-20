#if !_COMPILER

#include "ent_creation.h"
#include "compiler/entity.h"
#include "match.h"
#include "game/entity/map/portal.h"
#include "game/entity/map/spawn_point.h"
#include "game/entity/map/map_pickup.h"
#include "game/entity/map/launcher.h"
#include "game/entity/map/projectile_launcher.h"
#include "game/entity/map/damage_volume.h"

#if !_SERVER
#include "game/entity/map/wall_torch.h"
#endif

#define ALLOC_ENT(type) UPtr<type> e = std::make_unique<type>(); e->EntityID = ent->GetCompiledID(); e->Angle = ent->GetAngle(); if (ent->GetOrigin() != 0) e->MapPosition = ent->GetOrigin().ToType<GFlt>();
#define ADD_AS_TARGET GetGameMatch()->AddTargetEntity(ent->GetProperty("targetname"), e.get());

UPtr<KLoadedMapEntity> KLoadedMapEntity::CreateFromFile(KMapEntity* ent)
{
	if (ent->GetName() == "portal_volume")
	{
		ALLOC_ENT(KMapEntity_Portal);
		e->TargetName = ent->GetProperty("target");
		KString reset = ent->GetProperty("reset_velocity");
		e->bResetVelocity = (reset == "0" || reset == "") ? false : true;
		KString silent = ent->GetProperty("silent");
		e->bSilent = (silent == "0" || silent == "") ? false : true;
		return e;
	}
	else if (ent->GetName() == "portal_exit")
	{
		ALLOC_ENT(KMapEntity_PortalExit);
		ADD_AS_TARGET;
		return e;
	}
	if (ent->GetName() == "launch_volume")
	{
		ALLOC_ENT(KMapEntity_Launcher);
		e->TargetName = ent->GetProperty("target");
		f64 arc = .5;
		ent->GetProperty("launch_arc").ToDoubleSafe(arc);
		e->LaunchArc = arc;
		return e;
	}
	if (ent->GetName() == "damage_volume")
	{
		ALLOC_ENT(KMapEntity_DamageVolume);
		return e;
	}
	else if (ent->GetName() == "launch_target")
	{
		ALLOC_ENT(KMapEntity_PortalExit);
		ADD_AS_TARGET;
		return e;
	}
	if (ent->GetName() == "projectile_launcher")
	{
		ALLOC_ENT(KMapEntity_ProjectileLauncher);
		e->TargetName = ent->GetProperty("target");
		f64 interval = 1;
		f64 delay = -1;
		ent->GetProperty("interval").ToDoubleSafe(interval);
		ent->GetProperty("initial_delay").ToDoubleSafe(delay);

		if (delay < 0) delay = interval;

		e->Interval = KTime::FramesFromTime(interval);
		e->InitialDelay = KTime::FramesFromTime(delay);

		e->CreateLauncher();

		return e;
	}
	else if (ent->GetName() == "projectile_launcher_target")
	{
		ALLOC_ENT(KMapEntity_PortalExit);
		ADD_AS_TARGET;
		return e;
	}
	else if (ent->GetName() == "deathmatch_spawn")
	{
		ALLOC_ENT(KMapEntity_DeathmatchSpawn);
		return e;
	}
	else if (ent->GetName().StartsWith("item"))
	{
		if (!IsNetAuthority()) return nullptr;
		ALLOC_ENT(KMapEntity_PickupItem);
		e->CreatePickup(ent->GetName());
		return e;
	}
	else if (ent->GetName() == "torch")
	{
#if !_SERVER
		ALLOC_ENT(KMapEntity_WallTorch);

		KString main = ent->GetProperty("main_color");
		if (!main.IsEmpty()) e->MainColor = main;
		KString off = ent->GetProperty("secondary_color");
		if (!off.IsEmpty()) e->OffColor = off;
		
		e->CreateTorch();
		return e;
#endif
	}

	return nullptr;
}

#endif