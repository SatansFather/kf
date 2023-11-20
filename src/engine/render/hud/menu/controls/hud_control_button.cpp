#if !_SERVER && !_COMPILER

#include "hud_control_button.h"
#include "engine/render/interface/render_interface.h"

void KHudControlButton::Draw()
{
	
}

void KHudControlButton::SetText(const KString& text)
{
	ButtonText = text;
	KRenderInterface* iface = GetRenderInterface();
	TextLayoutHandle = iface->HUD_CreateTextLayout(ButtonText, FontUsage, 1, 1);
}

void KHudControlButton::SetClickBoundsToText()
{
	if (TextLayoutHandle == 0) return;


}

#endif