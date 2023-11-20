#include "spec_point.h"
#include "engine/game/match.h"
#include "entity/spectator.h"
#include "engine/game/local_player.h"
#include "../engine/input/listener_game.h"

#if !_SERVER

KString CCOM_SpecPoint(const KString& val)
{
	KGameMatch* m = GetGameMatch();
	KEntity_Spectator* spec = dynamic_cast<KEntity_Spectator*>(GetControlledEntity());

	if (m && spec)
	{
		KGameInputListener* l = GetGameInput();
		GVec3 rot(l->GetPitch(), l->GetYaw(), 0);
		m->AddSpecPoint(spec->GetPosition(), rot);
	}

	return "";
}

KString CCOM_SpecPlay(const KString& val)
{
	if (KEntity_Spectator* spec = dynamic_cast<KEntity_Spectator*>(GetControlledEntity()))
		spec->CycleSpecPoints();

	return "";
}

#endif