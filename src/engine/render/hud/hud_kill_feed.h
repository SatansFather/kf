#pragma once

#if !_SERVER && !_COMPILER

#include "hud_widget.h"
#include "../../game/kill_feed_message.h"

class KHudKillFeed : public KHudWidget
{
	friend class KRenderInterface;

	TVector<u32> PreVictimHandles;
	TVector<u32> PostVictimHandles;

	const u32 MaxOnScreenCount = 5;
	TVector<KKillFeedMessage> KillFeedMessages;

	// struct lets value default to 0
	struct LayoutHandle { u32 Handle = 0; };
	TMap<std::string, LayoutHandle> PlayerNameLayouts;

public:

	KHudKillFeed();
	void Draw() override;
};

#endif