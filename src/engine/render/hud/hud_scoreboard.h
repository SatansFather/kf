#pragma once

#if !_SERVER && !_COMPILER

#include "kfglobal.h"
#include "hud_widget.h"
#include "game/player_score_info.h"

class KHudScoreboard : public KHudWidget
{
	friend class KRenderInterface;

	u32 NameLayout = 0;
	u32 ScoreLayout = 0;
	u32 FragsLayout = 0;
	u32 DeathsLayout = 0;
	u32 DamageLayout = 0;
	u32 TimeLayout = 0;
	u32 PingLayout = 0;
	u32 PeriodLayout = 0;

	TMap<std::string, u32> PlayerNameLayouts;
	TMap<i32, u32> NumericLayouts;

	bool bShowing = false;

	TVector<KPlayerScoreInfo> PlayerInfo;

	void Draw() override;
public:
	KHudScoreboard();
	~KHudScoreboard();
};

#endif