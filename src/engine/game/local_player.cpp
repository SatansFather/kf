
#include "local_player.h"
#include "engine/game_instance.h"
#include "engine/utility/kstring.h"
#include "game/entity/properties/controllable.h"
#include "../input/listener_game.h"
#include "match.h"
#include "game/entity/death_cam.h"
#include "../../game/entity/spectator.h"
#include "../net/net_interface.h"
#include "../input/view.h"
#include "../collision/trace.h"
#include "../net/player.h"
#include "game/entity/properties/collidable.h"
#include "frag_message.h"
#include "../collision/broadphase/trace_args.h"

#if !_SERVER

void KLocalPlayer::LoadConfig(const KString& profile)
{
	Config.Load(profile);
	LOG("Loaded config file: " + profile + ".cfg");
}

void KLocalPlayer::SaveConfig(const class KString& profile, bool lock /*= false*/)
{
	Config.Save(profile, lock);
	LOG("Saved config file: " + profile + ".cfg");
}

#endif

void KLocalPlayer::PossessEntity(const TObjRef<KEntity>& ent)
{
	if (ent.IsValid() && ControlledEntity != ent)
	{
#if !_SERVER
		if (GetGameMatch())
		{
			GetGameMatch()->CurrentDeathMessage.bShouldShow = false;
			GetGameMatch()->CurrentDeathMessage.bUpdated = true;
		}
#endif

		WeaponRenderInfo.Reset();
		CharacterInfo.SetNewCharacter(true);

		ControlledEntity = ent;

#if !_SERVER
		TDataPool<KEntity_DeathCamera>::GetPool()->DestroyAll();
#endif
		if (KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(ControlledEntity.Get()))
		{
			CameraRoll = 0;
			CameraPosition = ControlledEntity.Get()->GetPosition();
			cont->OnPossessed();
			bTeleportedThisFrame = true;
		}
	}
}

void KLocalPlayer::PossessSpectator()
{
	KEntity_Spectator* spec = TDataPool<KEntity_Spectator>::GetPool()->CreateNew().Get();
	if (spec) PossessEntity(spec);
}

bool KLocalPlayer::HasPendingSpawnRequest()
{
	bool pending = bPendingSpawnRequest;
	bPendingSpawnRequest = false;
	return pending;
}

void KLocalPlayer::ResetForNewMatch()
{
	
}

void KLocalPlayer::Tick()
{
	if (CameraCorrectionOffset != 0)
	{
		f32 dist = CameraCorrectionOffset.Length();

		f32 ups = 256 * (dist / 12.f);
		if (ups < 256) ups = 256;

		GVec3 max = (CameraCorrectionOffset.GetNormalized() * ups);

		f64 alpha = (dist / ups) - GameFrameDelta();

		if (alpha <= 0) CameraCorrectionOffset = 0;
		else CameraCorrectionOffset = GVec3::Lerp(0, max, alpha);
	}

	if (bPendingControlReset)
	{
		u8 state = GetGameInput()->GetState();
		if (state & u8(EInputKeyState::Firing))
		{
			GetGameMatch()->SpawnPlayerCharacter();	
			if (IsNetClient()) 
			{
				KNetPlayer* p = GetLocalNetPlayer();
				if (!p->IsSpectating())
					bPendingSpawnRequest = true;
			}
			bPendingControlReset = false;
		}
	}

#if !_SERVER
	// show names of other players
	TargetPlayerName = "";
	//if (KNetInterface* iface = GetNetInterface())
	//if (false)
	{
		if (GetViewedEntity())
		{
			f32 p, y;
			KInputView::GetViewForRender(p, y);
			GVec3 dir = GVec3::FromPitchYaw(p, y);

			GHitResult hit;
			hit.SearchCollision = ECollisionMask::PlayerCharacter | ECollisionMask::WorldStatic;
		
			const auto notMe = [](KTraceHitParams& hits, int& locals) -> bool
			{
				if (hits.Test->HitCollision == ECollisionMask::WorldStatic) return true;
				KEntity* ent = ((KEntProp_CollidableBBox*)(hits.Test->Object))->GetEntity();
				return ent != GetViewedEntity();
			};

			int nullint = 0;
			KFunctionArg<KTraceHitParams, int, bool> func(notMe, nullint);
KTimePoint start = KTime::Now();
			//TraceLine(GLineSegment(CameraPosition, CameraPosition + dir * 10000), hit, &func);
			TraceLine(GLineSegment(CameraPosition, CameraPosition + dir * 128), hit, &func);
//CHATLOG(KTime::Since(start));

			if (hit.HitCollision == ECollisionMask::PlayerCharacter)
			{
				if (KEntity_Character_Player* guy = ((KEntProp_CollidableBBox*)hit.Object)->GetEntity()->As<KEntity_Character_Player>())
				  if (KNetPlayer* p = GetNetInterface()->GetPlayerFromIndex(guy->OwningPlayerIndex))
				    TargetPlayerName = p->PlayerName;
			}
		}
	}
#endif

	if (KNetPlayer* p = GetLocalNetPlayer())
		p->Tick();
}

#if !_SERVER
void KLocalPlayer::PushCameraRotation(const FVec3& amount, f32 duration, f32 startAlpha /*= 0*/)
{
	CameraPush.PushRotation(amount, duration, startAlpha);
}

void KLocalPlayer::PushCameraFov(f32 amount, f32 duration, f32 startAlpha /*= 0*/)
{
	CameraPush.PushFov(amount, duration, startAlpha);
}

void KLocalPlayer::ShakeCamera(f32 strength, f32 duration, f32 shakeRate)
{
	CameraPush.PushShake(strength, duration, shakeRate);
}

void KLocalPlayer::AddConcussion(f32 concussion)
{
	CameraPush.AddConcussion(concussion);
}
#endif

KLocalPlayer* GetLocalPlayer()
{
	return KGameInstance::Get().GetLocalPlayer();
}

KEntity* GetControlledEntity()
{
	return GetLocalPlayer()->ControlledEntity.Get();
}

KEntity* GetSpectatedEntity()
{
	KEntity* control = GetControlledEntity();
	if (!control) return nullptr;

	if (KEntity_Spectator* spec = control->As<KEntity_Spectator>())
		return spec->GetViewedEntity();

	return nullptr;
}

KEntity* GetViewedEntity()
{
	KEntity* ent = GetSpectatedEntity();
	if (!ent) ent = GetControlledEntity();
	return ent;
}
