#if !_SERVER && !_COMPILER

#include "hud_load_screen.h"
#include "../interface/render_interface.h"
#include "../communication.h"

void KHudLoadScreen::Draw()
{
	KRenderInterface* iface = GetRenderInterface();
	if (!iface->IsLoadingMap() || iface->IsResettingMap()) return;

	f32 scaledY = GetScaledY();
	f32 scaledX = GetScaledX();
	f32 viewY = GetViewportY();
	f32 viewX = GetViewportX();

	const KString& mapName = iface->GetLoadingMapName();
	if (mapName == "mainmenu" || mapName == "emptymap") return;
	if (mapName != LoadingMapName)
	{
		LoadingMapName = mapName;
		LoadScreenHandle = iface->HUD_CreateTextLayout(mapName, EFontUsage::LoadScreen, 1);
	}

	f32 lastTime = iface->GetLastFrameTime();
	f32 progress = KRenderBridge::Get().MapLoadProgress;

	if (progress > LastProgress)
	{
		f32 add = (progress - LastProgress) * lastTime * 1;
		LastProgress = KLerp(LastProgress + add, progress, progress);
	}
	else if (progress < LastProgress)
		LastProgress = progress;
	
	KHudRectF rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = viewX;
	rect.bottom = viewY;
	iface->HUD_SetDrawColor(FColor32(.0, .0, .0, 1));
	iface->HUD_FillRect(rect);

	rect.left = viewX * .1;
	rect.right = viewX * .9;
	rect.top = viewY * .8;
	rect.bottom = viewY * .9;

	iface->HUD_SetDrawColor(FColor32(.2, .2, .2, 1));
	iface->HUD_FillRect(rect);

	rect.right = KLerp(viewX * .1, viewX * .9, LastProgress);
	iface->HUD_SetDrawColor(FColor32(1, LastProgress * LastProgress, .1, 1));
	iface->HUD_FillRect(rect);

	f32 w = iface->HUD_GetTextWidth(LoadScreenHandle);
	f32 h = iface->HUD_GetTextHeight(LoadScreenHandle);
	
	iface->HUD_SetDrawColor(FColor32(0, 0, 0, 1));
	KHudPointF p;
	p.x = (viewX / 2.f) - (w / 2.f);
	p.y = viewY * .85 - h/2;
	iface->HUD_DrawTextLayout(LoadScreenHandle, p);
}

void KHudLoadScreen::OnWindowResize()
{
	//LoadingMapName = "";
}

#endif