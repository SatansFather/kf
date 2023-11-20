#include "character.h"
#include "engine/collision/hit_result.h"
#include "engine/game/local_player.h"

// DELETE
#include "../../engine/collision/broadphase/bvh_grid.h"
#include "../../compiler/compiler.h"
#include "engine/system/terminal/terminal.h"
#include "../../engine/input/view.h"
#include "character_player.h"
#include "../../engine/input/listener_game.h"
#include "../../engine/input/binding.h"
#include "projectile.h"
#include "../../engine/utility/random.h"
#include "../../engine/game_instance.h"
#include "../../engine/net/net_interface.h"
#include "../../engine/audio/audio.h"
#include "graphics/flash.h"
#include "projectiles/proj_shotgun.h"
#include "projectiles/proj_atom.h"
#include "../cheat_manager.h"
#include "graphics/gibs.h"
#include "../../engine/net/player.h"
#include "spectator.h"
#include "death_cam.h"
#include "../../engine/replay/replay_read.h"

#if !_SERVER && !_COMPILER
KString CCOM_Tele(const KString& val)
{
	GVec3 target(0, 0, 0);
	TVector<GFlt> vals = val.ToNumArray<GFlt>();
	if (vals.size() == 3)
	  for (u8 i = 0; i < 3; i++)
		target[i] = vals[i];

	
	if (GetLocalPlayer()->ControlledEntity.IsValid())
	{
		GetLocalPlayer()->ControlledEntity.Get()->SetPosition(target);
		//GetLocalPlayer()->ControlledCharacter->MoveState.Velocity = GVec3(0, 0, 0);
	}

	return "";
}

KString CCOM_Position(const KString& val)
{
	if (GetLocalPlayer()->ControlledEntity.IsValid())
		return GetLocalPlayer()->ControlledEntity.Get()->GetPosition().ToString();

	return "";
}

KString CCOM_Direction(const KString& val)
{
	if (GetLocalPlayer()->ControlledEntity.IsValid())
	{
		KEntity_Character_Player* guy = GetLocalPlayer()->ControlledEntity.As<KEntity_Character_Player>();
		if (val.IsEmpty())
		{
			
			/*GVec3 py = GVec3::FromPitchYaw(guy->GetPitch(), guy->GetYaw());
			GVec3 forward = py;
			GVec3 up ({ 0, 0, 1 });
			GVec3 right = forward.Cross(up);
			LOG("--------");
			LOG(GVec3(forward.y, forward.z, forward.x).ToString());
			LOG(GVec3(right.y, right.z, right.x).ToString());

			forward = GVec3(py.y, py.z, py.x);
			up = GVec3(0, 1, 0);
			right = forward.Cross(up);
			LOG(forward.ToString());
			LOG(right.ToString());*/

			GVec3 dir = GVec3::FromPitchYaw(guy->GetPitch(), guy->GetYaw());
			return KString(dir.ToString() 
						+ " - Pitch: " 
						+ KString(glm::degrees(guy->GetPitch()), 2) 
						+ " - Yaw: "
						+ KString(glm::degrees(guy->GetYaw()), 2)); 
		}
		else
		{
			GVec3 dir(0, 0, 0);
			TVector<GFlt> vals = val.ToNumArray<GFlt>();
			if (vals.size() == 3)
			  for (u8 i = 0; i < 3; i++)
				dir[i] = vals[i];

			KInputView::SetAnglesFromVector(dir);
		}
	}

	return "";
}

#endif

void KEntity_Character::SetTransient_ReplayPosition(KNetVec3& val)
{
	//SetPosition(val.ToVec3());
}

void KEntity_Character::GetTransient_ReplayPosition(KNetVec3& val)
{
	//val = GetPosition();
}

void KEntity_Character::UpdateInputVector()
{
	if (IsControlledEntity())
		SetInputFromPlayer();
	else if (IsNetServer() && IsPlayerControlled())
		SetInputFromClient();
	else if (!IsNetClient())
		SetInputVector(GetAiInputvector());
}

GVec3 KEntity_Character::GetAiInputvector()
{
	//return GVec3(1, 0, 0);
	return 0;
}

void KEntity_Character::InitNetObject()
{
	if (OwningPlayerIndex != NULL_PLAYER)
	{
		if (OwningPlayerIndex == GetLocalNetPlayer()->OwningPlayerIndex)
		{
			GetLocalPlayer()->PossessEntity(this);

			// temporary
			TDataPool<KEntity_Spectator>::GetPool()->DestroyAll();

			if (GetFrameCreated() == KTime::FrameCount())
			{
				SetPosition(PositionForOwner);
				SetYawInt(SpawnYaw);
				SetPitch(0);
			}
		}

		if (KNetPlayer* p = GetNetInterface()->GetPlayerFromIndex(OwningPlayerIndex))
			p->ControlledEntity = this;
	}
}

void KEntity_Character::OnNetUpdate()
{
	if (IsControlledEntity())
	{
		SetReplayingMoves(true);
		PushFramesRemaining = PushFramesRemaining;
		CheckServerMove(GetLocalNetPlayer()->LastStateFrameUnpacked, VelocityForOwner,
			PositionForOwner, 0, 0);
		SetReplayingMoves(false);
	}
}

void KEntity_Character::PreCreateSnapshot()
{
	VelocityForOwner = Velocity;
	PositionForOwner = GetPosition();
}

void KEntity_Character::Tick()
{
	//Move(delta);
	GFlt delta = GameFrameDelta();

	//if (!GetReplayReader()->IsPlaying())
	{
		UpdateInputVector();
		if (IsNetClient() && !IsControlledEntity())
		{
			SetPosition(GetNetPosition());
			UpdateOccupiedCells();
		}
		else if (!PerformMovement(GetInputVectorForMove())) 
			return; // dead
	}

	GVec3 vel = Velocity;

	if (!IsNetAuthority() && IsSpectatedEntity())
	{
		// fake velocity
		if (KEntity_Spectator* spec = GetControlledEntity()->As<KEntity_Spectator>())
		{
			vel = spec->GetEntVelocity();
			if (IsWalking()) Velocity.z = 0;
		}
	}

	CameraAdjustZ -= GetLastMoveStepDistance();

	GFlt camheight = 20 + CameraAdjustZ;

	if (CameraAdjustZ != 0)
	{
		CameraAdjustZ *= KLerp(.92, .7, vel.Length() / 550);
		if (abs(CameraAdjustZ) < .25) CameraAdjustZ = 0;
	}
	// TODO check if this is followed character
	if (IsViewedEntity() || IsNetServer())
	{
		FacingDirection = GVec3
		(
			cos(Pitch) * cos(Yaw),
			cos(Pitch) * sin(Yaw),
			sin(Pitch)
		);

		GFlt speed = vel.Length();
		bool walking = IsWalking() && speed > 100;
		f32 sideBob = 0;
		if (!walking) 
		{
			LastStopTime = KTime::FrameNow();

			if (HeadBob <= .01) HeadBob = 0;
			else HeadBob *= .95;
		}
		else
		{
			const f32 bobRate = 15;
			f32 time = KTime::FrameNow() - LastStopTime;
			HeadBob = speed < 50 ? 0 : cos(time * bobRate);
			sideBob = speed < 50 ? 0 : cos(time * bobRate/2);
			f32 freq = (bobRate / PI<f32>()) / 2.0;
			freq = 1.0 / freq;
			f32 wave = std::round(time / freq) * freq;
			if (LastStepWave != wave && !IsCrouching() && !IsReplayingMoves())
			{
				LastStepWave = wave;
				KSoundProperties props;
				props.MaxDistance = 1024;
				props.Volume = .5;
				props.bLowHoldFrames = true;
				if (KNetPlayer* player = GetOwningPlayer())
				{
					props.ReplicationMethod = SkipIndex;
					props.PlayerIndex = player->OwningPlayerIndex;
				}
				KSoundID sounds[] = 
				{
					KSoundID::Player_Step_1,
					KSoundID::Player_Step_2,
					KSoundID::Player_Step_3
				};
				KAudio::PlaySoundAttached(sounds[KTime::FrameCount() % 3], this, props);
			}

			if (time < .15) HeadBob *= time / .15;
#if !_SERVER
			HeadBob *= KSaturate(GetUserConfig()->Game.BobScale) * 4;
			HeadBob *= KSaturate(speed / GFlt(550));
#endif
		}
		
#if !_SERVER
		if (IsViewedEntity())
		{
			if (!IsFalling())
			{
				f32 landTime = KClamp(LandZ / 16, .5, 1);
				f32 landAlpha = KSaturate(KTime::FramesSince(GetLastLandFrame()) / (landTime / GameFrameDelta()));
				if (landAlpha < 1)
				{
					landAlpha = sin(PI<f32>() * pow(1 - landAlpha, 3));
					GetLocalPlayer()->CameraLandOffset = KClamp(KLerp(0, LandZ, landAlpha), 0, 16);
				}
				else
				{
					LandZ = 0;
				}
			}
			else
			{
				GetLocalPlayer()->CameraLandOffset *= .85;
				if (GetLocalPlayer()->CameraLandOffset < .05)
					GetLocalPlayer()->CameraLandOffset = 0;
			}

			GetLocalPlayer()->CameraPosition = GetPosition().AdjustZ(camheight + HeadBob - GetLocalPlayer()->CameraLandOffset);		
			GetLocalPlayer()->CameraPosition.z -= GetCrouchDepth() * As<KMoveState_Walking>()->CrouchDropDistance;

			// side bob
			//GVec3 lookDir = GVec3::FromPitchYaw(0, Yaw);
			//GVec3 rightDir = lookDir.Cross(GVec3(0, 0, 1)).GetNormalized();
			//GetLocalPlayer()->CameraPosition += rightDir * sideBob;
		
			GFlt v = vel.ToType<GFlt>() | (FacingDirection.SetZ(0) ^ GVec3(0, 0, 1));
			GetLocalPlayer()->CameraRoll = glm::radians(KClamp(v / GFlt(550), -1, 1)) * KSaturate(GetUserConfig()->Game.RollScale) * 3;
		
			if (TeleportedThisFrame())
				GetLocalPlayer()->bTeleportedThisFrame = true;

			GetLocalPlayer()->WeaponRenderInfo.SetFallSpeed(vel.z);
			GetLocalPlayer()->WeaponRenderInfo.SetRunSpeedAlpha(IsWalking() ? KClamp(vel.Length() / 550, 0, 1) : 0);
			GetLocalPlayer()->WeaponRenderInfo.SetCrouchAlpha(GetCrouchDepth());
			GetLocalPlayer()->WeaponRenderInfo.SetPitch(GetPitch());
			GetLocalPlayer()->WeaponRenderInfo.SetYaw(GetYaw());
		}

		//GetLocalPlayer()->WeaponRenderInfo.FallSpeed = IsFalling() ? Velocity.z : 0;
		//GetLocalPlayer()->WeaponRenderInfo.RunSpeedPercentage = KClamp(Velocity.Length() / 550, 0, 1);
#endif
	}
	CharacterTick();
}

void KEntity_Character::OnEnteredWater(const GHitResult& hit)
{
	//KAudio::PlaySound3D(nullptr, GetPosition(), .25);
}

bool KEntity_Character::IsCollisionEnabled()
{
	return KEntProp_CollidableBBox::IsCollisionEnabled() && 
		!(IsControlledEntity() && KCheatManager::CheatIsActive(EPersistentCheat::Ghost));
}

void KEntity_Character::OnKilled(class KNetPlayer* player /*= nullptr*/)
{
	if (player)	
		KillingPlayerIndex = player->OwningPlayerIndex;

#if !_SERVER// && !_DEBUG
	for (u32 i = 0; i < 16; i++)
	{
		KEntity_Gib_MeatChunk* chunk = TDataPool<KEntity_Gib_MeatChunk>::GetPool()->CreateNew().Get();
		GVec3 pos = GetPosition();
		GVec3 bounds = GetCollisionBoundsHalfExtent() - 5;
		pos.x += RandFloat(-bounds.x, bounds.x);
		pos.y += RandFloat(-bounds.y, bounds.y);
		pos.z += RandFloat(-bounds.z, bounds.z);
		chunk->SetPosition(pos);
		chunk->PrevMatrix = chunk->BuildMatrix(pos);
		GVec3 diff = pos - GetPosition();
		if (diff.z < 3) diff.z = 3;
		chunk->Push(diff.GetNormalized() * RandRange(250, 500) + GetPendingImpulse());
	}
#endif

	KSoundID deathSounds[] =
	{
		KSoundID::Player_Death_1, KSoundID::Player_Death_2
	};

	u32 index = (KTime::FrameCount() + Poolable_GetID()) % 2;

	if (IsViewedEntity())
		KAudio::PlaySound(deathSounds[index]);
	else
	{
		KSoundProperties props;
		props.MaxDistance = 1536;
		KAudio::PlaySound3D(deathSounds[index], GetPosition(), props);
	}

	//auto brain = TDataPool<KEntity_Pickup_Powerup_Brain>::GetPool()->CreateNew();
	//brain->SetPosition(GetPosition().AdjustZ(.1));
	//brain->Push(GVec3(D_RandRange(-200, 200), D_RandRange(-200, 200), D_RandRange(200, 350)));
	//brain->SetDropItem(true);

#if !_SERVER
	if (IsViewedEntity())
	{
		auto pool = TDataPool<KEntity_DeathCamera>::GetPool();
		pool->DestroyAll();
		auto cam = TDataPool<KEntity_DeathCamera>::GetPool()->CreateNew().Get();

		GVec3 extent = GetCollisionBoundsHalfExtent();
		GFlt viewZ = 20;
		GFlt camDimension = KMin(extent.x, extent.z) * .8;
		cam->SetCollisionBoundsHalfExtent(camDimension);

		// try to place this at camera height, but dont leave our own bounds
		GVec3 pos = GetPosition();
		GFlt maxZ = extent.z - camDimension;

		if (maxZ < viewZ)
		{
			// camera view needs to be same as this
			cam->OffsetZ = viewZ - maxZ;
			pos.z += maxZ;
		}
		else
		{
			pos.z += viewZ;
		}


		GVec3 vel = Velocity + GetPendingImpulse();
		if (vel.z < 100) vel.z = KClamp(vel.z + 100, vel.z, 100);

		GFlt p, y;
		(-vel).ToPitchYaw(p, y);
		SetYaw(y);
		SetPitch(0);

		cam->SetPosition(pos);

		cam->AddPendingImpulse(vel);

		KAudio::PlaySound(KSoundID::Score_Death);
	}
#endif

	KEntProp_Killable::OnKilled();
}

GVec3 KEntity_Character::GetGibPushVelocity(const GVec3& otherVel)
{
	return Velocity * 1.5;
}

void Push(GVec3 impulse, u32 pushFrames = 5)
{
	
}
