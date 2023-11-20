#include "pickup.h"
#include "engine/game/local_player.h"
#include "engine/collision/trace.h"
#include "properties/collidable.h"
#include "../../engine/utility/random.h"
#include "../../engine/audio/audio.h"
#include "../../engine/net/state.h"
#include "../pickup_message.h"

KEntity_PickupBase::KEntity_PickupBase()
{
	SetCollisionBoundsHalfExtent(GVec3(24, 24, 16));
	PhysicsProperties.Deceleration = .9;
	PhysicsFluidProperties.Deceleration = .93;
	SetMovementState(EMoveState::Physics);
	CollisionChannels = ECollisionMask::Pickup;
	CollisionBlocks = ECollisionMask::WorldStatic;
	CollisionOverlaps = ECollisionMask::Water | ECollisionMask::Portal | ECollisionMask::Launcher;
}

void KEntity_PickupBase::OnRep_ReppedFlags()
{
	SetGlobalSpawnState(ReppedFlags & EReppedPickupFlags::Spawned);
	SetDropItem(ReppedFlags & EReppedPickupFlags::Dropped);
}

void KEntity_PickupBase::PreCreateSnapshot()
{
	if (IsDropItem())
	{
		ReppedFlags |= EReppedPickupFlags::Spawned;
		ReppedFlags |= EReppedPickupFlags::Dropped;
	}
	else
	{
		ReppedFlags &= ~EReppedPickupFlags::Dropped;
		if (IsSpawned()) ReppedFlags |= EReppedPickupFlags::Spawned;
		else             ReppedFlags &= ~EReppedPickupFlags::Spawned;
	}
}

void KEntity_PickupBase::InitNetObject()
{
	DisableCollision();
}

bool KEntity_PickupBase::IsSpawned(KEntity* ent) const
{
	if (ShouldSpawnPerEntity() && ent)
		return !RemainingEntityRespawnFrames.contains(KHashedPoolable(ent).Hash);

	return Flags & PF_IsSpawnedGlobal;
}

void KEntity_PickupBase::SetStartSpawned(bool start)
{
	if (start)	Flags |= PF_StartSpawned;
	else		Flags &= ~PF_StartSpawned;
}

void KEntity_PickupBase::SetDropItem(bool drop)
{
	if (drop) Flags |= PF_IsDropItem;
	else	  Flags &= ~PF_IsDropItem;
}

bool KEntity_PickupBase::OnOverlap(KEntity* ent)
{
	if (CanPickUp(ent) && IsSpawned(ent) && IsNetAuthority())
	{
		PickUp(ent);
		Despawn(ent);
		return true;
	}
	return false;
}	

void KEntity_PickupBase::Tick()
{
	if (IsNetClient())
	{
		SetPosition(GetNetPosition());
		return;
	}

	if (IsDropItem() && IsNetAuthority())
	{
		if (Flags & PF_DestroyNextTick || GetEntityAge() >= 30)
		{
			DestroyEntity();
			return;
		}

		// drop items start small and then increase to the biggest size they can get to
		// up to the intended size
		// prevents spawning inside geometry
		TryIncreaseSize();
	}

	if (GetFrameCreated() == KTime::FrameCount() - 1 && IsNetAuthority())
	{
		UpdateOccupiedCells();
		SetMovementEnabled(IsDropItem());
	}

	if (MovementIsEnabled() && IsNetAuthority())
	{
		GVec3 input = 0;
		if (false && PhysicsFloorNormal != 0)
		{
			GFlt xPush = D_RandRange(100, 150);
			xPush *= (D_Random() % 2) ? -1 : 1;

			GFlt yPush = D_RandRange(100, 150);
			yPush *= (D_Random() % 2) ? -1 : 1;

			GFlt zPush = D_RandRange(250, 300);

			input = GVec3(xPush, yPush, zPush);
			Push(input);
		}

		PerformMovement(0);
		if ((PhysicsFloorNormal.z > 0 && Velocity.SetZ(0).LengthSq() < .5) ||
			(IsInWater() && Velocity.LengthSq() < 5))
			SetMovementEnabled(false);
	}

	if (RemainingGlobalRespawnFrames > 0)
		RemainingGlobalRespawnFrames--;

	if (RemainingGlobalRespawnFrames == 0)
	{
		// respawn
		if (! (Flags & PF_IsSpawnedGlobal) )
		{
			if (GetFrameCreated() + 1 < KTime::FrameCount())
				KAudio::PlaySound3D(GetRespawnSound(), GetPosition());

			Flags |= PF_IsSpawnedGlobal;
			SpawnStateChangeFrame = KTime::FrameCount();

			// check overlaps
			const auto checkPickup = [](KEntity*& ent, KCheckPickupLocalCapture& locals) -> bool
				{ return locals.Pickup->OnOverlap(ent); };

			KCheckPickupLocalCapture cap;
			cap.Pickup = this;

			KFunctionArg<KEntity*, KCheckPickupLocalCapture, bool> func(checkPickup, cap);

			TObjRef<KEntity_PickupBase> ref = this;

			PerformOnOverlapping(GetPosition(), GetCollisionBoundsHalfExtent(), &func);
			
			// drop items that spawn inside an entity that takes it will result in this being destroyed
			if (!ref.IsValid())
				return;

			// re enable collision if we werent picked up again
			if (RemainingGlobalRespawnFrames == 0)
				EnableCollision(); 
			else
				Flags &= ~PF_IsSpawnedGlobal;
		}
	}
	else
		Flags &= ~PF_IsSpawnedGlobal;

	TVector<std::array<u32, 4>> deleteKeys;

	for (auto& kv : RemainingEntityRespawnFrames)
	{
		if (!KHashedPoolable(kv.first).IsValid())
		{
			deleteKeys.push_back(kv.first);
			continue;
		}

		if (kv.second > 0)
			kv.second--;

		if (kv.second == 0)
		{
			deleteKeys.push_back(kv.first);
			continue;
		}
	}

	for (auto& key : deleteKeys)
		RemainingEntityRespawnFrames.erase(key);
}

bool KEntity_PickupBase::IsSpawnedForEntity(KEntity* ent) const
{
	return IsSpawned(ent);	
}

bool KEntity_PickupBase::IsSpawnedForViewedEntity() const
{
	KEntity* ent = GetLocalPlayer()->ControlledEntity.Get();
	return IsSpawned(ent);
}

void KEntity_PickupBase::SetGlobalSpawnState(bool spawned)
{
	if (spawned)
	{
		if (!IsSpawned() && SpawnStateChangeFrame != 0)
			KAudio::PlaySound3D(GetRespawnSound(), GetPosition());

		if (!(Flags & PF_IsSpawnedGlobal))
			SpawnStateChangeFrame = KTime::FrameCount();

		Flags |= PF_IsSpawnedGlobal;
	}
	else
	{
		
		KSnapshottable* snap = GetNetState()->GetReplicatedObject(LastPickupEntID);
		KEntity* ent = nullptr;
		if (snap) ent = dynamic_cast<KEntity*>(snap);

		Despawn(ent);

		if (IsNetClient())
			Flags &= ~PF_IsSpawnedGlobal;
	}
}

void KEntity_PickupBase::Despawn(KEntity* ent)
{
#if !_SERVER
	if (IsSpawned())
	{
		if (ent && ent == GetViewedEntity())
		{
			KAudio::PlaySoundAttached(GetDespawnSound(), ent);
			KPickupMessage::Create(GetPickupMessage());
		}
		else
			KAudio::PlaySound3D(GetDespawnSound(), GetPosition());
	}
#endif

	if (IsNetClient()) return;

	DisableCollision();

	if (KSnapshottable* snap = ent->As<KSnapshottable>())
		LastPickupEntID = snap->GetNetID();
	else
		LastPickupEntID = 0;

	if (IsDropItem())
	{
		DestroyEntity();
		return;
	}

	if (ShouldSpawnPerEntity())
	{
		if (!ent) return;
		if (!IsSpawned(ent)) return;

		RemainingEntityRespawnFrames[KHashedPoolable(ent).Hash] = 
			RespawnFrameCount;
	}
	else
	{
		if (!IsSpawned() && RemainingGlobalRespawnFrames > 0) return;
		RemainingGlobalRespawnFrames = RespawnFrameCount;
		SpawnStateChangeFrame = KTime::FrameCount();
	}
}

void KEntity_PickupBase::Poolable_OnInstantiated()
{
	if (!ShouldStartSpawned())
		Despawn(nullptr);
}

void KEntity_PickupBase::FillLightProperties(struct KDynamicLight& light)
{

}

void KEntity_PickupBase::TryIncreaseSize()
{
	// dont do this if we arent even able to move into a better spot
	if (!MovementIsEnabled()) return;

	const GVec3 maxSize(24, 24, 16);
	const GVec3 currentSize = GetCollisionBoundsHalfExtent();

	if (currentSize != maxSize)
	{
		GHitResult hit;
		InitHitResult(hit, true, false);
		
		GFlt currentScale = currentSize.x / maxSize.x;
		GFlt nextScale = currentScale;
		while (nextScale < 1 && !hit.bHit)
		{
			currentScale = nextScale;
			nextScale = KSaturate(currentScale + 1.0 / 24.0);
			const GVec3 testExtent = maxSize * nextScale;
			TraceBox(GLineSegment(GetPosition(), GetPosition()), testExtent, hit);
		}

		SetCollisionBoundsHalfExtent(maxSize * currentScale);
	}
}

void KEntity_PickupBase::ExplodePush(const GVec3& dir, f32 strength)
{
	if (IsDropItem())
	{
		SetMovementEnabled(true);
		Push(dir * strength * 2);
	}
}

void KEntity_PickupBase::InitDropItem()
{
	if (!IsNetAuthority()) return;

	SetDropItem(true);
	GFlt xPush = D_RandRange(150, 300);
	xPush *= (D_Random() % 2) ? -1 : 1;
	GFlt yPush = D_RandRange(150, 300);
	yPush *= (D_Random() % 2) ? -1 : 1;
	GFlt zPush = D_RandRange(250, 400);
	Push(GVec3(xPush, yPush, zPush));

	SetCollisionBoundsHalfExtent(.1);
	TryIncreaseSize();
	EnableCollision();
}

void KEntity_PickupBase::DestroyNextTick()
{
	Flags |= PF_DestroyNextTick;
}

void KEntity_PickupBase::AddPositionToHistory(u32 frame)
{
	if (IsDropItem())
		KEntity::AddPositionToHistory(frame);
}
