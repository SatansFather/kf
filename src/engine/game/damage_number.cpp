#include "damage_number.h"
#include "../utility/random.h"
#include "match.h"
#include "../game_instance.h"
#include "../audio/audio.h"

static u32 LastSoundFrame = 0;

KDamageNumber::KDamageNumber(const FVec3& pos, i32 damage, u32 frameLifetime)
	: Position(pos), Damage(damage), FramesRemaining(frameLifetime)
{
#if !_SERVER
	RandomSeed = Random();
	RenderTimeCreated = KGameInstance::Get().GetTotalRenderTime();
	Lifespan = FramesRemaining * (1.0 / GameFrameDelta());

	GetGameMatch()->LiveDamageNumbers.push_back(*this);

	if (KTime::FrameCount() != LastSoundFrame)
	{
		const u32 pitches = 12;
		const u32 maxDamage = 200;

		i32 pitch = damage / (maxDamage / pitches);
		if (pitch < 0) return;
		if (pitch > pitches - 1) pitch = pitches - 1;

		KAudio::PlaySound(KSoundID::Damage_Tick_0 + pitch, 2);
		LastSoundFrame = KTime::FrameCount();
	}
#endif
}
