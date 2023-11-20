#if !_SERVER

#include "pickup_message.h"
#include "engine/game/match.h"

void KPickupMessage::Create(const KString& text)
{
	if (KGameMatch* match = GetGameMatch())
		match->PendingPickupMessage.Text = text;
}

#endif