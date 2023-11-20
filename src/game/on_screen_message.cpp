#include "on_screen_message.h"
#include "engine/game/match.h"
#include "engine/audio/audio.h"

void KOnScreenMessage::Create(const KString& text, u32 slot, KSoundID sound, f32 duration)
{
#if !_SERVER
	KOnScreenMessage msg;
	msg.Text = text;
	msg.Slot = slot;
	msg.Duration = duration;
	GetGameMatch()->PendingOnScreenMessages.push_back(msg);

	if (sound.Index != KSoundID::Null_Sound)
		KAudio::PlaySound(sound);

#endif
}
