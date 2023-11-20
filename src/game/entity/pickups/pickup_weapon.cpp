#include "engine/game/match.h"
#include "../deferred_drop.h"
#include "pickup_weapon.h"
#include "../properties/weapon_inventory.h"

KEntity_Pickup_Weapon::KEntity_Pickup_Weapon()
{
	RespawnFrameCount = KTime::FramesFromTime(5);
}

bool KEntity_Pickup_Weapon::CanPickUp(KEntity* ent)
{
	return dynamic_cast<KEntProp_WeaponInventory*>(ent);
}

void KEntity_Pickup_Weapon::PickUp(KEntity* ent)
{
	KEntProp_WeaponInventory* inv = dynamic_cast<KEntProp_WeaponInventory*>(ent);
	inv->AddWeaponByID(GetWeaponID(), GetAmmoCount());
}

void KEntity_Pickup_Weapon::SpawnDrop(u8 id, const GVec3& position)
{
	if (IsNetAuthority())
	{
		KDeferredDropSpawn d;
		d.Position = position;
		d.WeaponIndex = id;
		d.ItemType = KDeferredDropSpawn::Type_Weapon;
		d.Finalize();
	}
}

#if !_SERVER
bool KEntity_Pickup_Weapon::UpdateRenderBuffer(KStaticMeshBase* entry, const GVec3& lastRenderPos)
{
	bool skip = false;

	glm::mat4 mat(1);

	TotalTime += GameFrameDelta() / 2;

	bool spawned = IsSpawnedForViewedEntity();

	f32 changeFrames = KTime::FramesSince(SpawnStateChangeFrame);
	f32 changeAlpha = KSaturate(changeFrames / 30.f);
	if (changeAlpha < 1) changeAlpha = LerpFade(changeAlpha);

	f32 z = IsDropItem() ? 0 : 8 + sin(-(PI<f32>() / 4.f) + changeFrames * GameFrameDelta() * 2) * 4;

	if (IsDropItem()) changeAlpha = 1;

	if (spawned)
	{
		f32 rotAlpha = KSaturate(changeFrames / 70);
		if (rotAlpha < 1) rotAlpha = LerpFade(rotAlpha);

		f32 scaleAlpha = KSaturate(changeFrames / 50);
		if (scaleAlpha < 1) scaleAlpha = LerpFade(scaleAlpha);

		if (IsDropItem())
		{
			scaleAlpha = 1;
			rotAlpha = 1;
		}

		TotalTime += (1 - rotAlpha) * GameFrameDelta() * 22;

		entry->PrevModelMat = glm::translate(mat, lastRenderPos.AdjustZ(LastZ).ToGLM());
		entry->PrevModelMat = glm::scale(entry->PrevModelMat, glm::vec3(2) * LastScaleAlpha);
		entry->PrevModelMat = glm::rotate(entry->PrevModelMat, (IsDropItem() ? GetFrameCreated() : LastRenderTime * 2), glm::vec3(0, 1, 0));;
		entry->CurrentModelMat = glm::translate(mat, GetPosition().AdjustZ(z).ToGLM());
		entry->CurrentModelMat = glm::scale(entry->CurrentModelMat, glm::vec3(2) * scaleAlpha);
		entry->CurrentModelMat = glm::rotate(entry->CurrentModelMat, (IsDropItem() ? GetFrameCreated() : TotalTime * 2), glm::vec3(0, 1, 0));;
		entry->SetLastMoveRenderAlpha(GetLastMoveAlpha());

		LastScaleAlpha = scaleAlpha;
	}
	else
	{
		skip = true;
		LastScaleAlpha = 0;
	}

	LastRenderTime = TotalTime;
	LastZ = z;

	return skip;
}

KBufferUpdateResult KEntity_Pickup_Weapon_Rocket::UpdateBuffers(KStaticMesh<"gunplaceholder", "rockettex">& entry)
{
	if (UpdateRenderBuffer(&entry, LastFrameRenderPosition))
		FlagBufferForSkip(entry);
	return true;
}


KBufferUpdateResult KEntity_Pickup_Weapon_Cannon::UpdateBuffers(KStaticMesh<"gunplaceholder", "cannontex">& entry)
{
	if (UpdateRenderBuffer(&entry, LastFrameRenderPosition))
		FlagBufferForSkip(entry);
	return true;
}


KBufferUpdateResult KEntity_Pickup_Weapon_Shotgun::UpdateBuffers(KStaticMesh<"gunplaceholder", "shotguntex">& entry)
{
	if (UpdateRenderBuffer(&entry, LastFrameRenderPosition))
		FlagBufferForSkip(entry);
	return true;
}


KBufferUpdateResult KEntity_Pickup_Weapon_Blast::UpdateBuffers(KStaticMesh<"gunplaceholder", "blastertex">& entry)
{
	if (UpdateRenderBuffer(&entry, LastFrameRenderPosition))
		FlagBufferForSkip(entry);
	return true;
}

#endif