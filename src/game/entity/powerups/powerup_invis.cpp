#include "powerup_invis.h"
#include "../properties/movable.h"
#include "engine/game/local_player.h"
#include "engine/net/state.h"
#include "../../../engine/audio/audio.h"

void KEntity_Powerup_Invis::OnRep_bIsInvisible()
{
	SetInvis(bIsInvisible);
}

KEntity_Powerup_Invis::KEntity_Powerup_Invis()
{
	PowerupID = EPowerupID::Invis;
}

void KEntity_Powerup_Invis::SetInvis(bool invis)
{
	if ((IsNetAuthority() && bIsInvisible != invis) || (bIsInvisible != bLastInvis))
	{
		if (KEntity* ent = CarryingEntity.Get())
		{
			KSoundProperties props;
			props.bAttachedPlayAfterDeath = true;
			KAudio::PlaySoundAttached(invis ? KSoundID::Invis_On : KSoundID::Invis_Off, ent, props);
		}
		bIsInvisible = invis;
		bLastInvis = invis;
	}
}

void KEntity_Powerup_Invis::Tick()
{
	if (IsNetAuthority())
		SetInvis(LastBreakFrame == MAX_U32 || KTime::FramesSince(LastBreakFrame) > 60);

	KEntity_Powerup::Tick();
}

void KEntity_Powerup_Invis::Break()
{
	LastBreakFrame = KTime::FrameCount();
}

#if !_SERVER
KBufferUpdateResult KEntity_Powerup_Invis::UpdateBuffers(KDynamicLight& entry)
{
	if (CarryingEntity.IsValid())
	{
		KEntity* guy = CarryingEntity.Get();
		bool isMine = guy == GetViewedEntity();
		if (isMine != IsInvisible())
		{
			FlagBufferForSkip(entry);
			return false;
		}

	//	SetPosition(guy->GetPosition());
		entry.SetPrevPosition(LastFramePosition);
		entry.SetCurrentPosition(GetPosition());

		entry.SetFalloff(1);
		entry.SetColor(FColor8(255, 255, 255, 0));

		f32 prevAge = KSaturate((GetEntityAge() - GameFrameDelta()) * 2);
		f32 age = KSaturate(GetEntityAge() * 2);
		
		entry.SetPrevRadius(prevAge * 192);
		entry.SetCurrentRadius(age * 192);
		entry.SetLightAll();
		entry.SetNegative();

		LastFramePosition = GetPosition();

		return true;
	}

	return false;
}
#endif