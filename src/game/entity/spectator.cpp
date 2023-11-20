#include "spectator.h"
#include "engine/game/local_player.h"
#include "engine/input/view.h"
#include "engine/input/listener_game.h"


// DELETE
#include "../../engine/net/net_interface.h"
#include "../../engine/net/player.h"
#include "../../engine/game/match.h"
#include "../spec_point.h"

KEntity_Spectator::KEntity_Spectator()
{
	GetFlyingMovement()->FlyingProperties.MoveSpeed = 700;
	GetFlyingMovement()->FlyingProperties.Deceleration = .85;
	GetFlyingMovement()->FlyingProperties.Acceleration = 8000;
	SetMovementState(EMoveState::Flying);

	MaxStepHeight = 16;

	SetCollisionBoundsHalfExtent(8);
	CollisionChannels = ECollisionMask::PlayerCharacter;
	CollisionBlocks = ECollisionMask::WorldStatic;
	CollisionOverlaps = ECollisionMask::Portal;
}

void KEntity_Spectator::Tick()
{
#if !_SERVER

	if (bCyclingSpecPoints)
	{
		if (KGameMatch* m = GetGameMatch())
		{
			if (SpecPointIndex < m->SpecPoints.size())
			{
				KSpecPoint* start = m->SpecPoints[SpecPointIndex].get();
				KSpecPoint* target = nullptr;
				if (SpecPointIndex + 1 < m->SpecPoints.size())
					target = m->SpecPoints[SpecPointIndex + 1].get();

				if (!target)
				{
					SetPosition(start->Position);
					SetPitchInt(start->Rotation.x);
					SetYawInt(start->Rotation.y);
					GetLocalPlayer()->CameraPosition = GetPosition();
				}
				else
				{
					GVec3 sp = start->Position;
					GVec3 tp = target->Position;
					GVec3 sr = start->Rotation;
					GVec3 tr = target->Rotation;

					SetPosition(GVec3::Lerp(sp, tp, SpecPointAlpha));
					SetPitchInt(KLerp(sr.x, tr.x, LerpFade(SpecPointAlpha)));
					SetYawInt(KLerp(sr.y, tr.y, LerpFade(SpecPointAlpha)));
					GetLocalPlayer()->CameraPosition = GetPosition();

					SpecPointAlpha += GameFrameDelta();
					while (SpecPointAlpha >= 1)
					{
						SpecPointIndex++;
						SpecPointAlpha -= 1;
					}
				}
			}
		}
		return;
	}

	// assign viewed entity
	u8 state = GetGameInput()->GetState();

	if ((state & EInputKeyState::Firing) && !(LastInputState & EInputKeyState::Firing))
	{
		if (KNetInterface* iface = GetNetInterface())
		{
			u8 startIndex = ViewedPlayerIndex;
			for (u32 i = 1; i <= 255; i++)
			{
				u8 index = startIndex + i;
				if (index == GetLocalNetPlayer()->OwningPlayerIndex)
					continue;

				if (KNetPlayer* p = iface->GetPlayerFromIndex(index))
				{
					ViewedPlayerIndex = index;
					break;
				}
				// TODO should i be index instead?
				if (i == NULL_PLAYER) ViewedPlayerIndex = NULL_PLAYER;
			}
		}
	}
	LastInputState = state;

	if (KEntity* ent = GetViewedEntity())
	{
		if (KEntProp_Controllable* cont = ent->As<KEntProp_Controllable>())
			KInputView::InterpolatePitchYaw(cont->GetPitch(), cont->GetYaw(), GameFrameDelta() * 2);

		GVec3 pos = ent->GetPosition();
		EntVelocity = (pos - LastFramePosition) / GameFrameDelta();
		LastFramePosition = pos;
	}
	else
	{
		EntVelocity = 0;
		LastFramePosition = 0;

		SetInputFromPlayer();
		PerformMovement(GetInputVectorForMove());
		GetLocalPlayer()->CameraPosition = GetPosition();
		GetLocalPlayer()->CameraRoll = 0;
		if (TeleportedThisFrame())
			GetLocalPlayer()->bTeleportedThisFrame = true;
	}
#endif
}

KEntity* KEntity_Spectator::GetViewedEntity()
{
	if (ViewedPlayerIndex != NULL_PLAYER)
	{
		if (KNetInterface* iface = GetNetInterface())
		{
			if (KNetPlayer* p = iface->GetPlayerFromIndex(ViewedPlayerIndex))
			{
				return p->ControlledEntity.Get();
			}
			else
			{
				ViewedPlayerIndex = NULL_PLAYER;
			}
		}
	}

	return nullptr;
}

void KEntity_Spectator::CycleSpecPoints()
{
	bCyclingSpecPoints = true;
	SpecPointAlpha = 0;
	SpecPointIndex = 0;
}
