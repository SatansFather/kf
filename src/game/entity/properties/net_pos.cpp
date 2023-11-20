#include "net_pos.h"
#include "../entity.h"
#include "movable.h"
#include "engine/net/state.h"
#include "collidable.h"
#include "engine/collision/trace.h"
#include "engine/net/player.h"

void KEntProp_NetPosition::GetTransient_ReppedNetPos(KNetVec3& val)
{
	val = GetEntity()->GetPosition();
}

void KEntProp_NetPosition::SetTransient_ReppedNetPos(KNetVec3& val)
{
	KEntity* ent = GetEntity();

	NetPosA = ent->GetPosition();
	NetPosB = val.ToVec3();

	KEntProp_Movable* mov = ent->As<KEntProp_Movable>();
	
	if (mov) mov->FramesSinceMove = 0;

	if (val.Data & 0x8000)
	{
		if (mov)
			mov->MovableFlags &= KEntProp_Movable::MF_Teleported;

		NetPosA = NetPosB;
	}
}

GVec3 KEntProp_NetPosition::GetNetPosition()
{
	KEntity* ent = GetEntity();
	f32 alpha = GetNetState()->GetInterpAlpha();

	GVec3 pos = GVec3::Lerp(NetPosA, NetPosB, alpha);
	if (alpha == 1) NetPosA = NetPosB;

	if (!pos.ContainsNaN())
	{
		KEntProp_CollidableBBox* box = ent->As<KEntProp_CollidableBBox>();
		KEntProp_Movable* mov = ent->As<KEntProp_Movable>();
		if (box && mov && box->IsCollisionEnabled() && mov->MovementIsEnabled() && 
			(box->OverlapsChannel(ECollisionMask::Weapon) || box->BlocksChannel(ECollisionMask::Weapon)))
		{
			// need to trace against our own projectiles for proper prediction
			GVec3 traceStart = ent->GetPosition();
			GHitResult hit;

			if (KMoveState_Walking* w = ent->As<KMoveState_Walking>())
				box->CrouchDistance = mov->GetCrouchDepth() * w->CrouchDropDistance;

			box->InitHitResult(hit);
			hit.SearchCollision = ECollisionMask::Weapon;
			TraceBox(GLineSegment(traceStart, pos), box->GetCollisionBoundsHalfExtent(), hit);

			if (hit.bHit & GHitResult::BoxEnt)
			{
				KEntProp_CollidableBBox* hitBox = (KEntProp_CollidableBBox*)hit.Object;
				KEntity* hitEnt = hitBox->GetEntity();

				if (hitEnt->GetOwningPlayer() == GetLocalNetPlayer())
				{
					if (KEntProp_Movable* mov = hitEnt->As<KEntProp_Movable>())
					{
						GHitResult entHit;
						hitBox->InitHitResult(entHit);
						entHit.Point = hitEnt->GetPosition();
						entHit.Normal = -hit.Normal;
						entHit.Object = box;
						mov->OnMoveBlocked(mov->Velocity, entHit); // this is our projectile
					}
				}
			}
		}
		return pos;
	}

	return NetPosB;
}
