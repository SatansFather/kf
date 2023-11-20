#include "entity.h"
#include "engine/system/time.h"
#include "properties/renderable.h"
#include "properties/movable.h"
#include "engine/net/net_interface.h"
#include "properties/collidable.h"
#include "engine/net/snapshottable.h"
#include "engine/net/player.h"
#include "engine/net/state.h"
#include "properties/powerup_inventory.h"

KEntity::KEntity()
{
	EntityFrameCreated = KTime::FrameCount();
}

const GVec3& KEntity::GetLastFrameNetPosition() const
{
	return GetEntityFrameAge() < 1 ? Position : LastFrameNetPosition;
}

u32 KEntity::GetEntityFrameAge() const
{
	return KTime::FrameCount() - GetFrameCreated();
}

GFlt KEntity::GetEntityAge() const
{
	return GameFrameDelta() * GetEntityFrameAge();
}

void KEntity::SetPosition(const GVec3& pos)
{
	Position = pos.ContainsNaN() ? 0 : pos;
	OnPositionUpdated();
}

bool KEntity::CreatedThisFrame()
{
	return EntityFrameCreated == KTime::FrameCount();
}

void KEntity::DestroyEntity()
{
	// dont let clients destroy server created objects
	if (KSnapshottable* snap = As<KSnapshottable>())
	  if (snap->bCreatedFromServer) return;

	// update render now since this will be gone during render update
	RenderUpdate(true);

	if (KEntProp_CollidableBBox* col = As<KEntProp_CollidableBBox>())
		col->DisableCollision();

	OnEntityDestroyed();
	Poolable_Destroy();
}

void KEntity::RenderUpdate(bool destroying /*= false*/)
{
#if !_SERVER
	u32 count = KTime::FrameCount();
	// this cast should always succeed since only classes that 
	// inherit from this are added to the renderable pool map
	KEntProp_Movable* mover = As<KEntProp_Movable>();
	KRenderablePropBase* render = As<KRenderablePropBase>();
	if (render)
	{
		if (count != EntityFrameCreated)
		{			
			if (destroying)
			{
				// force an update the next frame
				// this frame needs a forced update because this is being added out of order
				// next frame needs an update because this will be gone
				render->SetNextForceUpdateFrame(count + 1);

				// TODO update doesnt always need to be forced on destroy frame
			}
			else if (render->RenderableLifespan >= 0)
			{
				render->RenderableLifespan -= GameFrameDelta();
				if (render->RenderableLifespan <= 0)
				{
					//render->MarkInfoChanged(); // force an update
					//render->UpdateRenderableBuffers(true);
					DestroyEntity();
					return;
				}
			}
			render->UpdateRenderableBuffers(count <= EntityFrameCreated + 2 || destroying);
		}
		else
		{
			render->InitializeRenderable(); // does not update any buffers
		}
		if (!mover) render->LastFrameRenderPosition = GetPosition();
	}
	if (mover)
	{
		if (mover->FramesSinceMove == mover->FrameTickInterval - 1 || count == EntityFrameCreated)
		{
			if (render) render->LastFrameRenderPosition = GetPosition();
			mover->LastFrameVelocity = mover->Velocity;
		}

		// TODO this probably shouldnt be here but its the safest since it happens after game tick is finished
		mover->CreateSavedMove();
	}
#endif
}

void KEntity::Poolable_PreDestroy()
{
	if (KSnapshottable* snap = As<KSnapshottable>())
	{
		snap->CreateDestroyData();
	}
}

void KEntity::AddPositionToHistory(u32 frame)
{
	if (KEntProp_CollidableBBox* box = As<KEntProp_CollidableBBox>())
	{
		KCellMap& cellMap = GetNetState()->PositionSnapshots[frame % MAX_SNAPSHOTS];

		if (KEntProp_Movable* m = As<KEntProp_Movable>())
		{
			// get bounds of entire move this frame
			GBoundingBox bounds = box->GetAdjustedBounds();
			if (!m->TeleportedThisFrame())
			{
				GBoundingBox prevBounds = bounds;
				GVec3 posDiff = GetPosition() - GetLastFrameNetPosition();
				prevBounds.Min -= posDiff;
				prevBounds.Max -= posDiff;
				bounds.Update(prevBounds);
			}
			
			cellMap.AddCompEntityFromBounds(this, bounds);
		}
		else
		{
			cellMap.AddCompEntityFromBounds(this, box->GetAdjustedBounds());
		}
		LastFrameNetPosition = Position;
	}
}

class KEntity_Powerup* KEntity::GetCarriedPowerup(u8 id)
{
	if (KEntProp_PowerupInventory* inv = As<KEntProp_PowerupInventory>())
	  if (inv->HasPowerup(id))
	    return inv->CarriedPowerups[id].Get();

	return nullptr;
}

KNetPlayer* KEntity::GetOwningPlayer()
{
	KNetInterface* iface = GetNetInterface();
	if (!iface) return nullptr;

	KSnapshottable* snap = As<KSnapshottable>();
	if (!snap) return nullptr;

	if (snap->OwningPlayerIndex == -1) return nullptr;
	return iface->GetPlayerFromIndex(snap->OwningPlayerIndex);
}
