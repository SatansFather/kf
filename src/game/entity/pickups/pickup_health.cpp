#include "pickup_health.h"
#include "../properties/killable.h"
#include "engine/game/match.h"
#include "../deferred_drop.h"
#include "../character.h"

KEntity_Pickup_Health::KEntity_Pickup_Health()
{
	RespawnFrameCount = KTime::FramesFromTime(20);
}

bool KEntity_Pickup_Health::CanPickUp(KEntity* ent)
{
	if (KEntProp_Killable* p = dynamic_cast<KEntProp_Killable*>(ent))
	{
		if (MaxHealth > 0 && p->GetHealth() >= MaxHealth) return false;
		return p->CanPickUpHealth();
	}
	return false;
}

void KEntity_Pickup_Health::PickUp(KEntity* ent)
{
	KEntProp_Killable* k = dynamic_cast<KEntProp_Killable*>(ent);

	i32 start = k->GetHealth();
	i32 target = start + HealValue;

	if (MaxHealth == 0 && k->GetHealth() < 100 && HealValue >= 100)
		target += 100 - k->GetHealth();

	if (MaxHealth > 0 && target > MaxHealth)
		target = MaxHealth;


	const u32 overTarget = 400;
	const u32 maxTarget = 800;
	if (target > overTarget)
	{
		f32 over = target - overTarget;

		if (target > maxTarget) target = maxTarget;

		f32 remaining = HealValue;
		f32 accum = 0;
		const u32 step = 10;

		for (f32 h = start; h < target && remaining > 0; h += step)
		{
			if (h > overTarget)
			{
				accum += step * (1 - ((h - overTarget) / (maxTarget - overTarget)));
			}
			else
			{
				accum += step;
			}
			remaining -= step;
		}
		if (accum < 0) accum = 0;
		target = start + accum;


		if (KEntity_Character* guy = dynamic_cast<KEntity_Character*>(k->GetEntity()))
		{
			guy->DamageMultiplier += (over * (f32(HealValue) / 100.f)) / 20.f / 100.f;
		}
	}

	k->SetHealth(target);
}

void KEntity_Pickup_Health::OnRep_ReppedFlags()
{
	KEntity_PickupBase::OnRep_ReppedFlags();
	SetHealthType(ReppedFlags);
}

void KEntity_Pickup_Health::SetHealthType(u8 type)
{
	type &= ~EReppedPickupFlags::Spawned;
	type &= ~EReppedPickupFlags::Dropped;

	switch (type)
	{
		case EReppedPickupFlags::HP20:
		{
			ReppedFlags |= EReppedPickupFlags::HP20;
			MaxHealth = 0;
			HealValue = 20;
	#if !_SERVER
			Scale = .75;
			CrystalColor = FColor32(1, .4, .9, 1);
	#endif
			break;
		}
		case EReppedPickupFlags::HP50:
		{
			ReppedFlags |= EReppedPickupFlags::HP50;
#if !_SERVER
			CrystalColor = FColor32(.3, .4, 1, 1);
#endif
			break;
		}
		case EReppedPickupFlags::HP100:
		{
			ReppedFlags |= EReppedPickupFlags::HP100;
			MaxHealth = 0;
			HealValue = 100;
#if !_SERVER
			Scale = 1.25;
			CrystalColor = FColor32(.1, .8, .5, 1);
#endif
			break;
		}
		case EReppedPickupFlags::HP200:
		{
			ReppedFlags |= EReppedPickupFlags::HP200;
			MaxHealth = 0;
			HealValue = 200;
#if !_SERVER
			Scale = 1.5;
			CrystalColor = FColor32(1, .4, 0, 1);
#endif
			break;
		}
	}
}

void KEntity_Pickup_Health::SpawnDrop(u8 flagValue, const GVec3& position)
{
	if (IsNetAuthority())
	{
		KDeferredDropSpawn d;
		d.Position = position;
		d.HealthFlags = flagValue;
		d.ItemType = KDeferredDropSpawn::Type_Health;
		d.Finalize();
	}
}

KString KEntity_Pickup_Health::GetPickupMessage()
{
	return "Health +" + KString(HealValue);
}

KSoundID KEntity_Pickup_Health::GetRespawnSound() const
{
	if (HealValue == 200) return KSoundID::Respawn_HP_Big;
	if (HealValue == 100) return KSoundID::Respawn_HP_Med;
	return KSoundID::Respawn_HP_Small;
}

KSoundID KEntity_Pickup_Health::GetDespawnSound() const
{
	if (HealValue == 200) return KSoundID::Despawn_HP_Big;
	if (HealValue == 100) return KSoundID::Despawn_HP_Med;
	return KSoundID::Despawn_HP_Small;
}

#if !_SERVER
KBufferUpdateResult KEntity_Pickup_Health::UpdateBuffers(KStaticMesh<"healthframe", "healthframe">& frame, KHealthCrystal& crystal, KDynamicLight& light)
{	
	glm::mat4 mat(1);

	TotalTime += GameFrameDelta() / 2;


	bool spawned = IsSpawnedForViewedEntity();

	f32 changeFrames = KTime::FramesSince(SpawnStateChangeFrame);
	f32 changeAlpha = KSaturate(changeFrames / 30.f);
	if (changeAlpha < 1) changeAlpha = LerpFade(changeAlpha);

	f32 z = IsDropItem() ? 0 : sin(-(PI<f32>() / 4.f) + changeFrames * GameFrameDelta() * 2) * 4;

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

		frame.PrevModelMat = glm::translate(mat, LastFrameRenderPosition.AdjustZ(LastZ).ToGLM());
		frame.PrevModelMat = glm::scale(frame.PrevModelMat, glm::vec3(6 * Scale) * LastScaleAlpha);
		frame.PrevModelMat = glm::rotate(frame.PrevModelMat, (IsDropItem() ? GetFrameCreated() : LastRenderTime), glm::vec3(0, 1, 0));;
		frame.CurrentModelMat = glm::translate(mat, GetPosition().AdjustZ(z).ToGLM());
		frame.CurrentModelMat = glm::scale(frame.CurrentModelMat, glm::vec3(6 * Scale) * scaleAlpha);
		frame.CurrentModelMat = glm::rotate(frame.CurrentModelMat, (IsDropItem() ? GetFrameCreated() : TotalTime), glm::vec3(0, 1, 0));;
		frame.SetLastMoveRenderAlpha(GetLastMoveAlpha());

		crystal.PrevModelMat = frame.PrevModelMat;
		crystal.CurrentModelMat = frame.CurrentModelMat;
		crystal.Data = frame.Data;
		crystal.SetColor(CrystalColor.To8());

		LastScaleAlpha = scaleAlpha;
	}
	else
	{
		FlagBufferForSkip(frame);
		FlagBufferForSkip(crystal);
		LastScaleAlpha = 0;
	}

	light.SetPrevPosition(LastFrameRenderPosition.AdjustZ(-4));
	light.SetCurrentPosition(GetPosition().AdjustZ(-4));
	light.SetFalloff(20);
	light.SetColor(FColor8(0, 100, 255, 255));
	FlagBufferForSkip(light);

	f32 fromRadius = spawned ? 14 : 45;
	f32 toRadius = spawned ? 45 : 14;
	f32 radius = KLerp(fromRadius, toRadius, changeAlpha);

	light.SetPrevRadius(LastRadius);
	light.SetCurrentRadius(radius);

	LastRenderTime = TotalTime;
	LastZ = z;
	LastRadius = radius;

	return true;
}
#endif

