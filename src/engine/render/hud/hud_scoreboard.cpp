#if !_SERVER && !_COMPILER

#include "hud_scoreboard.h"
#include "engine/render/interface/render_interface.h"
#include "engine/render/color.h"

KHudScoreboard::KHudScoreboard()
{
	KRenderInterface* iface = GetRenderInterface();
	NameLayout = iface->HUD_CreateTextLayout("Player", EFontUsage::Scoreboard, 1000, 1000);
	ScoreLayout = iface->HUD_CreateTextLayout("Score", EFontUsage::Scoreboard, 1000, 1000);
	FragsLayout = iface->HUD_CreateTextLayout("Frags", EFontUsage::Scoreboard, 1000, 1000);
	DeathsLayout = iface->HUD_CreateTextLayout("Deaths", EFontUsage::Scoreboard, 1000, 1000);
	DamageLayout = iface->HUD_CreateTextLayout("Damage", EFontUsage::Scoreboard, 1000, 1000);
	TimeLayout = iface->HUD_CreateTextLayout("Time", EFontUsage::Scoreboard, 1000, 1000);
	PingLayout = iface->HUD_CreateTextLayout("Ping", EFontUsage::Scoreboard, 1000, 1000);
	PeriodLayout = iface->HUD_CreateTextLayout(".", EFontUsage::Scoreboard, 1000, 1000);
}

KHudScoreboard::~KHudScoreboard()
{

}

void KHudScoreboard::Draw()
{
	if (!bShowing) return;

	KRenderInterface* iface = GetRenderInterface();

	const f32 viewX = GetViewportX();
	const f32 viewY = GetViewportY();
	const f32 scaleY = GetScaledY();

	const f32 wFromCenter = .25 * ((16.f / 9.f) / (viewX / viewY));
	const f32 screenTop = viewY * .1;
	const f32 rectHeight = iface->HUD_GetTextHeight(NameLayout) + scaleY * 3;

	KHudRectF r;
	r.left = viewX * (.5 - wFromCenter);
	r.right = viewX * (.5 + wFromCenter);
	r.top = screenTop;
	r.bottom = r.top + rectHeight;

	// header
	iface->HUD_SetDrawColor(FColor8(20, 20, 20, 200));
	iface->HUD_FillRect(r);

	bool lightRect = true;
	for (const KPlayerScoreInfo& info : PlayerInfo)
	{
		r.top = r.bottom;
		r.bottom = r.top + rectHeight;
		u8 rectColor = lightRect ? 60 : 20;
		iface->HUD_SetDrawColor(FColor8(rectColor, rectColor, rectColor, 100));
		iface->HUD_FillRect(r);
		lightRect = !lightRect;
	}

	const f32 barWidth = r.right - r.left;
	const f32 namePercentage = .4;

	// all layouts minus name
	u32 layouts[] =
	{
		ScoreLayout,
		FragsLayout,
		DeathsLayout,
		DamageLayout,
		//TimeLayout,
		PingLayout
	};

	const u32 layoutCount = sizeof(layouts) / sizeof(u32);
	const f32 colPercentage = (1.f - namePercentage) / layoutCount;

	KHudPointF p;
	p.x = r.left + (namePercentage / 2 * barWidth) - iface->HUD_GetTextWidth(NameLayout) / 2;
	p.y = screenTop + scaleY;
	DrawTextShadowed(NameLayout, p, scaleY, FColor8(200, 200, 200, 255), FColor8(40, 40, 40, 255));

	// draw player names
	p.x = r.left + barWidth * .01;
	u32 place = 1;
	for (const KPlayerScoreInfo& info : PlayerInfo)
	{
		p.y += rectHeight;

		u32 nameLayout = 0;
		u32 placeLayout = 0;
		if (PlayerNameLayouts.contains(info.PlayerName.Get()))
		{
			nameLayout = PlayerNameLayouts[info.PlayerName.Get()];
		}
		else
		{
			nameLayout = iface->HUD_CreateTextLayout(info.PlayerName, EFontUsage::Scoreboard, 1000, 1000);
			PlayerNameLayouts[info.PlayerName.Get()] = nameLayout;
		}
		if (NumericLayouts.contains(place))
		{
			placeLayout = NumericLayouts[place];
		}
		else
		{
			placeLayout = iface->HUD_CreateTextLayout(place, EFontUsage::Scoreboard, 1000, 1000);
			NumericLayouts[place] = placeLayout;
		}

		FColor8 color = info.bIsMine ? FColor8(200, 200, 0, 255) : FColor8(200, 200, 200, 255);
		DrawTextShadowed(placeLayout, p, scaleY, color, FColor8(40, 40, 40, 255));
		f32 preX = p.x;
		p.x += iface->HUD_GetTextWidth(placeLayout);
		DrawTextShadowed(PeriodLayout, p, scaleY, color, FColor8(40, 40, 40, 255));
		p.x = preX + barWidth * .04;
		DrawTextShadowed(nameLayout, p, scaleY, color, FColor8(40, 40, 40, 255));
		p.x = preX;
		place++;
	}

	for (u32 i = 0; i < layoutCount; i++)
	{
		p.y = screenTop + scaleY;
		const f32 colWidth = barWidth * colPercentage;

		f32 px = (r.left + namePercentage * barWidth) + (colWidth * (i + 1) - (colWidth / 2));
		p.x = px - iface->HUD_GetTextWidth(layouts[i]) / 2;
				
		DrawTextShadowed(layouts[i], p, scaleY, FColor8(200, 200, 200, 255), FColor8(40, 40, 40, 255));

		for (const KPlayerScoreInfo& info : PlayerInfo)
		{
			p.y += rectHeight;

			i32 stat = info.GetStatByIndex(i);
			u32 layout = 0;
			if (NumericLayouts.contains(stat))
			{
				layout = NumericLayouts[stat];
			}
			else
			{
				layout = iface->HUD_CreateTextLayout(KString(stat), EFontUsage::Scoreboard, 1000, 1000);
				NumericLayouts[stat] = layout;
			}
			
			p.x = px - iface->HUD_GetTextWidth(layout) / 2;
			FColor8 color = info.bIsMine ? FColor8(200, 200, 0, 255) : FColor8(200, 200, 200, 255);
			DrawTextShadowed(layout, p, scaleY, color, FColor8(40, 40, 40, 255));
		}
	}
}

#endif