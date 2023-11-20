#include "character_player.h"
#include "engine/game/local_player.h"

// DELETE
#include "spectator.h"
#include "../../engine/input/listener_game.h"
#include "weapons/wep_shotgun.h"
#include "../../engine/net/player.h"
#include "../../engine/net/net_interface.h"
#include "../../engine/audio/audio.h"
#include "graphics/water_splash.h"
#include "../../engine/game/frag_message.h"
#include "../../engine/game/kill_feed_message.h"
#include "../../engine/net/state.h"
#include "../../engine/collision/trace.h"
#include "pickups/pickup_health.h"
#include "powerups/powerup_invis.h"
#include "../on_screen_message.h"
#include "deferred_drop.h"

void KEntity_Character_Player::GetTransient_DeathVelocity(KNetVec3& val)
{
	val = GetPendingImpulse();
}

void KEntity_Character_Player::SetTransient_DeathVelocity(KNetVec3& val)
{
	PendingImpulse = val.ToVec3();
}

KEntity_Character_Player::KEntity_Character_Player()
{
	WalkingProperties.MoveSpeed = 550;
	WalkingProperties.Deceleration = .875;//.8535;
	WalkingProperties.Acceleration = 4800;
	//GetWalkingMovement()->WalkingProperties.Acceleration = 3500;
	AirAcceleration = 1100;
	SetMovementState(EMoveState::Walking);

	FlyingProperties.MoveSpeed = 700;
	FlyingProperties.Deceleration = .92;
	FlyingProperties.Acceleration = 6000;
	
	SwimmingProperties.MoveSpeed = 550;
	SwimmingProperties.Deceleration = .95;
	SwimmingProperties.Acceleration = 2500;

	SetCollisionBoundsHalfExtent(GVec3(14, 14, 24));
	CollisionChannels = ECollisionMask::PlayerCharacter;

	CollisionBlocks = ECollisionMask::WorldStatic
	                | ECollisionMask::MonsterCharacter
	                | ECollisionMask::PlayerCharacter;

	CollisionOverlaps = ECollisionMask::Launcher
	                  | ECollisionMask::Weapon
	                  | ECollisionMask::Water
	                  | ECollisionMask::Portal
	                  | ECollisionMask::Gib
	                  | ECollisionMask::Damage
	                  | ECollisionMask::Pickup;

	//MaxStepHeight = 18;
	MaxStepHeight = 24;
	CrouchDropDistance = 20;
	SetCanBeHealed(true);
	SetCanPickUpHealth(true);
	SetHealth(100);
	AssignIgnoreID();
}

void KEntity_Character_Player::OnNetDestroy()
{
#if !_SERVER
	KNetPlayer* killer = nullptr;
	KNetPlayer* owner = nullptr;
	if (KillingPlayerIndex != NULL_PLAYER && KillingPlayerIndex != OwningPlayerIndex)
		killer = GetNetInterface()->GetPlayerFromIndex(KillingPlayerIndex);

	if (OwningPlayerIndex != NULL_PLAYER)
		owner = GetNetInterface()->GetPlayerFromIndex(OwningPlayerIndex);

	if (killer && killer == GetLocalNetPlayer() && owner)
	{
		// show frag message
		KFragMessage f(owner->PlayerName);
	}
	else if (owner == GetLocalNetPlayer() && killer)
	{
		// show death message
		KDeathMessage f(killer->PlayerName, 0);
	}

	if (owner)
	{
		if (killer) 
		{
			KKillFeedMessage m(killer->PlayerName, owner->PlayerName, 0);
		}
		else		
		{
			KKillFeedMessage m("", owner->PlayerName, 1);
		}
	}

	OnKilled(killer);
#endif
}

void KEntity_Character_Player::OnKilled(class KNetPlayer* player /*= nullptr*/)
{
	if (player && player != GetOwningPlayer())
	{
		if (player->IsViewedPlayer())
			KAudio::PlaySound(KSoundID::Score_Frag, 2);

		if (KEntity* ent = player->ControlledEntity.Get())
		{
			if (KEntity_Character_Player* guy = ent->As<KEntity_Character_Player>())
			{
				if (IsNetAuthority())
					guy->KillStreak++;

				if (guy->KillStreak % 5 == 0 && guy->KillStreak > 0)
					KOnScreenMessage::Create(player->PlayerName + " kill streak " + KString(guy->KillStreak), 0, KSoundID::Spree_Announce);
			}
		}
	}
	
	if (KNetPlayer* p = GetOwningPlayer())
	{
		if (KillStreak >= 5)
		{
			if (!player || player == p)
			{
				KOnScreenMessage::Create(p->PlayerName + " ended their own spree (" + KString(KillStreak) + ")", 1, KSoundID::Spree_Ended);
			}
			else if (player && player != p)
			{
				KOnScreenMessage::Create(p->PlayerName + "'s killing spree (" + KString(KillStreak) +  ") was ended by " + player->PlayerName, 1, KSoundID::Spree_Ended);
			}
		}
	}


	if (IsNetAuthority())
	{
		u32 count = KillStreak / 3 + 1;
		for (u32 i = 0; i < count && i < 10; i++)
			KEntity_Pickup_Health::SpawnDrop(EReppedPickupFlags::HP20, GetPosition());

		if (KillStreak >= 10)
		{
			KDeferredDropSpawn d;
			d.Position = GetPosition();
			d.ItemType = KDeferredDropSpawn::Type_Powerup;
			d.FrameDuration = KTime::FramesFromTime(20);
			d.PowerupIndex = EPowerupID::Invis;
			d.Finalize();
		}
	}

	KEntity_Character::OnKilled(player);
}

void KEntity_Character_Player::OnEnteredWater(const GHitResult& hit)
{
	return;
#if !_SERVER
	GFlt velLen = Velocity.Length();
	if (velLen > 200)
	KEntity_WaterSplash::Create(hit.Point - hit.Normal, hit.Normal, FColor32(.72, .52, .68, .35).To8(), velLen / 40, velLen / 40);
#endif
}

void KEntity_Character_Player::CharacterTick()
{
	if (GetFrameCreated() == KTime::FrameCount() - 1)
		KAudio::PlaySoundAttached(KSoundID::Player_Spawn, this);

	i16 scroll = 0;
	u8 wep = 0;
	u8 state = 0;

#if !_SERVER
	if (IsControlledEntity())
	{
		KGameInputListener* list = GetGameInput();
		scroll = list->GetScroll();
		wep = list->GetWeapon();
		state = list->GetState();
	}
	else if (IsNetServer())
#endif
	{
		state = InputState.Keys;
		wep = InputState.WeaponIndex;
	}
	
	UpdateWeaponState(state, wep, scroll);
}

#if !_SERVER

void KEntity_Character_Player::InitializeRenderable()
{
	PrevMatrix = glm::mat4(1);
	PrevMatrix = glm::translate(PrevMatrix, GetPosition().ToGLM());
	PrevMatrix = glm::scale(PrevMatrix, glm::vec3(1, 1, 1));
	PrevMatrix = glm::rotate(PrevMatrix, (f32)GetYaw() - 1.5708f, glm::vec3(0, 1, 0));
}

KBufferUpdateResult KEntity_Character_Player::UpdateBuffers(KStaticMesh<"player", "player_0">& entry, KDynamicLight& light)
{
	if (HasPowerup(EPowerupID::Invis))
		if (dynamic_cast<KEntity_Powerup_Invis*>(CarriedPowerups[EPowerupID::Invis].Get())->IsInvisible())
			return false;	

	if (IsViewedEntity()) 
	{
		FlagBufferForSkip(entry);
	}
	else
	{
		GFlt crouch = GetCrouchDepth() * CrouchDropDistance;
	
		glm::mat4 mat(1);
		mat = glm::translate(mat, GetPosition().AdjustZ(-crouch/2).ToGLM());
		mat = glm::scale(mat, glm::vec3(1, (48.f - crouch) / 48.f, 1));
		mat = glm::rotate(mat, (f32)GetYaw() - 1.5708f, glm::vec3(0, 1, 0));

		entry.CurrentModelMat = mat;
		entry.PrevModelMat = TeleportedThisFrame() ? mat : PrevMatrix;
		PrevMatrix = mat;
		entry.SetLastMoveRenderAlpha(1);

		if (IsNetClient())
			MovableFlags &= ~MF_Teleported;
	}

	light.SetPrevPosition(LastFrameRenderPosition.AdjustZ(-24));
	light.SetCurrentPosition(GetPosition().AdjustZ(-24));
	light.SetPrevRadius(24);
	light.SetCurrentRadius(24);
	light.SetColor(FColor8(120, 120, 120, 0));
	light.SetLightAll();
	light.SetNegative();
	light.SetFalloff(2);

	//entry.PrevPos = LastFramePosition.ToGLM4();
	//entry.CurrentPos = GetPosition().ToGLM4();
	//entry.CurrentPos.w = TeleportedThisFrame();
	//entry.HalfExtent = glm::vec4(14, 24, 14, 1);

	return KBufferUpdateResult(/*LastFramePosition != GetPosition()*/true, true);
}
#endif