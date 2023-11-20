#include "killable.h"
#include "../entity.h"
#include "../../cheat_manager.h"
#include "engine/game/local_player.h"
#include "engine/game/match.h"
#include "engine/audio/audio.h"
#include "engine/net/player.h"
#include "engine/net/net_interface.h"
#include "../powerups/powerup_invis.h"

void KEntProp_Killable::OnKilled(class KNetPlayer* player /*= nullptr*/)
{
	if (KEntity* ent = GetEntity())
		ent->DestroyEntity();
}

bool KEntProp_Killable::SetHealth(i32 health, bool triggerEvent, KNetPlayer* player)
{
	if (Health == health) return !IsDead();

	if (!HasMatchAuthority())
	{
		Health = health;
		return !IsDead();
	}

	bool startDead = IsDead();
	bool damaged = health < Health;
	bool healed = health > Health;
	i32 diff = Health - health;
	
	Health = health;

	if (damaged)	
	{
		DamageTaken += abs(diff);
		if (triggerEvent) OnDamaged(IsDead(), player);
	}
	else if (healed)
	{
		HealingTaken += abs(diff);
		if (triggerEvent) OnHealed();
	}

	if (IsDead() && !startDead)	
	{
		if (player && IsNetServer())
		{
			if (KNetPlayer* myPlayer = GetEntity()->GetOwningPlayer())
			{
				if (myPlayer == player)
				{
					player->Score--;
					player->Deaths++;
				}
				else
				{
					myPlayer->Deaths++;
					player->Frags++;
					player->Score++;
				}
				GetNetInterface()->FlagPendingScoreUpdate();
			}
		}

		OnKilled(player);
	}
	else if (!IsDead() && startDead)
	{
		LifeCount++;
		OnResurrected(player);
	}

	return !IsDead();
}

void KEntProp_Killable::TakeDamage(i32 damage, KNetPlayer* player)
{
	if (damage == 0) return;
	if (!HasMatchAuthority()) return;

	if (damage < 0) 
	{
		TakeHeal(-damage, player);
		return;
	}

	KEntity* ent = GetEntity();
	KNetPlayer* owningPlayer = ent->GetOwningPlayer();
	KNetPlayer* localPlayer = GetLocalNetPlayer();

	if (player)
	  if (KEntity* e = player->ControlledEntity.Get())
		if (e != ent)
		  if (KEntity_Character* guy = e->As<KEntity_Character>())
	        damage = std::round(damage * guy->DamageMultiplier);

	if (CanBeDamaged())
	{
		if (KEntity_Powerup* powerup = ent->GetCarriedPowerup(EPowerupID::Invis))
		{
			dynamic_cast<KEntity_Powerup_Invis*>(powerup)->Break();
		}

		if (player == localPlayer)
		// this condition is true offline (nullptr == nullptr)
		{
			if (ent != GetLocalPlayer()->ControlledEntity.Get())
				KDamageNumber d(ent->GetPosition().AdjustZ(32).ToType<f32>(), damage, 45);
		}
		else if (player && player != owningPlayer)
		{
			if (KSnapshottable* snap = ent->As<KSnapshottable>())
				player->OutDamageNumbers.push_back( { KTime::FrameCount(), snap->NetID, damage } );
		}

		if (player)
		{
			LastDamagingPlayerIndex = player->OwningPlayerIndex;
			if (player != owningPlayer)
			{
				player->Damage += damage;
				GetNetInterface()->FlagPendingScoreUpdate();
			}
		}

		if (SetHealth(Health - damage, true, player))
		{
			if (KTime::FrameCount() != LastPainSoundFrame)
			{
				KSoundID painSounds[] =
				{
					KSoundID::Player_Pain_1, 
					KSoundID::Player_Pain_2,
					KSoundID::Player_Pain_3,
					KSoundID::Player_Pain_4
				};

				u32 index = KTime::FrameCount();
				index += ent->Poolable_GetIndex();
				index %= 4;
				KSoundProperties props;
				props.ReplicationMethod = EReplicatedSound::SendAll;
				props.Volume = 1;
				props.MaxDistance = 1536;
				KAudio::PlaySoundAttached(painSounds[index], ent, props);
				LastPainSoundFrame = KTime::FrameCount();
			}
		}
		else if (player)
		{
#if !_SERVER
			// dead guy
			if (owningPlayer == localPlayer && player != owningPlayer)
			{
				// someone killed me
				KDeathMessage f(player->PlayerName, 0);
			}
			else if (player == localPlayer && owningPlayer)
			{
				// i killed a guy
				KFragMessage  f(owningPlayer->PlayerName);
			}
#endif
		}
	}
}

void KEntProp_Killable::TakeHeal(i32 heal, KNetPlayer* player)
{
	if (heal == 0) return;
	if (!HasMatchAuthority()) return;

	if (heal < 0)
	{
		TakeDamage(-heal, player);
		return;
	}

	if (CanBeHealed())
		SetHealth(Health + heal, true);
}

i32 KEntProp_Killable::GetHealth() const
{
	return Health;
}

bool KEntProp_Killable::CanBeDamaged()
{
	if (KCheatManager::CheatIsActive(EPersistentCheat::God))
	{
		KLocalPlayer* p = GetLocalPlayer();
		if (KEntity* ent = p->ControlledEntity.Get())
		  if (ent == GetEntity())
			return false;
	}
	return KillableFlags & Damageable;
}

void KEntProp_Killable::SetCanBeDamaged(bool v)
{
	if (v)	KillableFlags |= Damageable;
	else	KillableFlags &= ~Damageable;
}

void KEntProp_Killable::SetCanBeHealed(bool v)
{
	if (v)	KillableFlags |= Healable;
	else	KillableFlags &= ~Healable;
}

void KEntProp_Killable::SetCanBeResurrected(bool v)
{
	if (v)	KillableFlags |= Resurrectable;
	else	KillableFlags &= ~Resurrectable;
}

void KEntProp_Killable::SetCanPickUpHealth(bool v)
{
	if (v)	KillableFlags |= PickUpHP;
	else	KillableFlags &= ~PickUpHP;
}
