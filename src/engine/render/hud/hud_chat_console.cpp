#if !_SERVER && !_COMPILER

#include "hud_chat_console.h"
#include "../../console/chat_console.h"
#include "../interface/render_interface.h"

KHudChatConsole::KHudChatConsole()
{
	Console = GetChatConsole();
	BaseFontSize = 16;
	FontUsage = EFontUsage::Chat;
	BackgroundColor = FColor8(0, 0, 0, 0);
	bDrawShadowedText = true;
}

void KHudChatConsole::GetBackgroundBounds(KHudRectF& rect)
{
	f32 viewX = GetViewportX();
	f32 viewY = GetViewportY();
	f32 scaleY = GetScaledY();

	rect.left = viewX * .1;
	rect.top = viewY * .65;
	rect.bottom = viewY * .78;

	f32 aspect = viewX / viewY;
	f32 maxAspect = 16.f / 9.f;
	f32 aspectScale = KSaturate(aspect / maxAspect);

	rect.right = rect.left + 600 * scaleY * aspectScale;
}

f32 KHudChatConsole::GetMessageOpacity(class KTextMessage* msg)
{
	if (Console->IsShowing() || GetRenderInterface()->IsShowingScoreboard()) return 1;
	f32 time = KTime::Since(msg->TimeCreated);
	if (time < 5) return 1;
	return 1 - KSaturate(time - 5);
}

#endif