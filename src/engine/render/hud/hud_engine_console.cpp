#if !_SERVER && !_COMPILER

#include "hud_engine_console.h"
#include "../../console/engine_console.h"
#include "../interface/render_interface.h"

#if 0
void KHudEngineConsole::DeleteRemovedLayouts()
{
	KEngineConsole* c = GetEngineConsole();
	c->MessageMutex.lock();
	for (u32 i : c->PendingLayoutRemoval)
		GetRenderInterface()->HUD_DeleteTextLayout(i);
	c->PendingLayoutRemoval.clear();
	c->MessageMutex.unlock();
}

void KHudEngineConsole::Draw()
{
	KRenderInterface* r = GetRenderInterface();
	KEngineConsole* c = GetEngineConsole();
	if (!c->IsShowing()) return;
	
	f32 scaledY = GetScaledY();
	f32 viewY = GetViewportY();
	f32 viewX = GetViewportX();

	KHudRectF rect;
	rect.left = 0;
	rect.top = 0;
	rect.bottom = viewY / 2;
	rect.right = viewX;

	// draw main background
	r->HUD_SetDrawColor(FColor8(0, 0, 0, 200));
	r->HUD_FillRect(rect);

	const f32 fontSize = 20 * scaledY;
	const f32 lineTop = viewY / 2;
	const f32 linePad =  3 * scaledY;
	const f32 lineHeight = fontSize + (linePad * 4);
	const f32 lineBottom = lineTop + lineHeight;
	
	// draw text entry rect
	rect.top = lineTop;
	rect.bottom = lineBottom;
	r->HUD_SetDrawColor(FColor8(20, 20, 20, 200));
	r->HUD_FillRect(rect);

	// outline text entry
	r->HUD_SetDrawColor(FColor8(255, 255, 255, 200));
	rect.left += scaledY / 2;
	rect.right -= scaledY / 2;
	r->HUD_DrawRect(rect, scaledY);

	f32 msgBottom = lineTop - linePad;
	f32 leftOffset = scaledY * 4;

	// draw submission text and cursor
	bool drawCursor = false;
	{
		std::lock_guard<std::mutex> lock(c->TextMutex);

		f64 flash = KTime::Since(c->LastCursorFlashTime);
		if (i32(flash * 2) % 2 == 0) drawCursor = true;
		
		if (c->bPendingTextUpdate)
		{
			KString pos = c->CurrentString.GetSubstring(0, c->CursorPosition);

			if (c->SubmissionLayoutHandle > 0)
				r->HUD_DeleteTextLayout(c->SubmissionLayoutHandle);
			if (c->CursorPosHandle > 0)
				r->HUD_DeleteTextLayout(c->CursorPosHandle);

			c->SubmissionLayoutHandle = r->HUD_CreateTextLayout(
				c->CurrentString, EFontUsage::Console, 999, 0, c->SubmissionLayoutHandle);

			c->CursorPosHandle = r->HUD_CreateTextLayout(
				pos, EFontUsage::Console, 999, 0, c->CursorPosHandle);
		}
	}

	r->HUD_DrawTextLayout(c->SubmissionLayoutHandle, 
		{ leftOffset, lineTop + linePad } );

	if (drawCursor)
	{
		f32 cursorX = leftOffset + r->HUD_GetTextWidth(c->CursorPosHandle);
		r->HUD_DrawLine(cursorX, lineTop + linePad, cursorX, lineBottom - linePad);
	}

	// draw visible entries
	{	
		std::lock_guard<std::mutex> lock(c->MessageMutex);
		KConsoleMessage* msg = c->MessageHead.get();
		
		u32 currentMsgLines = 0;

		// find starting message based on scroll
		for (i32 i = 0; i < c->ScrollCount && msg; i++)
		{
			if (currentMsgLines == 0) 
				currentMsgLines = r->HUD_GetTextLineCount(msg->LayoutHandle);

			if (currentMsgLines > 0) 
				currentMsgLines--;

			if (currentMsgLines == 0) 
				msg = msg->Next.get();
		}
		
		while (msg && msgBottom > 0)
		{
			if (msg->LayoutHandle == 0) 
			  msg->LayoutHandle = r->HUD_CreateTextLayout(
				msg->Message, EFontUsage::Console, 1, 1);

			f32 h = r->HUD_GetTextHeight(msg->LayoutHandle);

			r->HUD_SetDrawColor(FColor32(msg->r, msg->g, msg->b, 1));
			r->HUD_DrawTextLayout(msg->LayoutHandle, { leftOffset, msgBottom - h });	
			
			msg = msg->Next.get();
			if (msg) msgBottom -= h;// * r->HUD_GetTextLineCount(msg->LayoutHandle);
		}
	}
}
#endif

KHudEngineConsole::KHudEngineConsole()
{
	Console = GetEngineConsole();
	FontUsage = EFontUsage::Console;
	BackgroundColor = FColor8(0, 0, 0, 200);
}

void KHudEngineConsole::Draw()
{
	if (Console->IsShowing()) KHudTextConsole::Draw();
}

void KHudEngineConsole::GetBackgroundBounds(KHudRectF& rect)
{
	rect.left = 0;
	rect.top = 0;
	rect.bottom = GetViewportY() / 2;
	rect.right = GetViewportX();
}

f32 KHudEngineConsole::GetMessageOpacity(class KTextMessage* msg)
{
	return 1;
}

void KHudEngineConsole::GetAutoSuggestions(TVector<KString>& vec, KTimePoint& time)
{
	KEngineConsole* c = dynamic_cast<KEngineConsole*>(Console);
	if (c)
	{
		if (c->AutoSuggestions) vec = *c->AutoSuggestions;
		time = c->LastAutoCompleteUpdateTime;
	}
}

#endif