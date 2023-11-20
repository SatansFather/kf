#if !_SERVER && !_COMPILER

#include "hud_text_console.h"
#include "../../console/text_console.h"
#include "../interface/render_interface.h"
#include "../color.h"

void KHudTextConsole::DeleteRemovedLayouts(bool lock /*= true*/)
{
	if (lock) Console->MessageMutex.lock();
	for (u32 i : Console->PendingLayoutRemoval)
		GetRenderInterface()->HUD_DeleteTextLayout(i);
	Console->PendingLayoutRemoval.clear();
	if (lock) Console->MessageMutex.unlock();
}

void KHudTextConsole::Draw()
{
	KRenderInterface* r = GetRenderInterface();

	f32 scaledY = GetScaledY();
	f32 viewY = GetViewportY();
	f32 viewX = GetViewportX();

	KHudRectF rect;
	GetBackgroundBounds(rect);
	f32 msgTop = rect.top;
	// draw main background
	r->HUD_SetDrawColor(BackgroundColor);
	r->HUD_FillRect(rect);

	const f32 fontSize = BaseFontSize * scaledY;
	const f32 lineTop = rect.bottom;
	const f32 linePad = 3 * scaledY;
	const f32 lineHeight = fontSize + (linePad * 4);
	const f32 lineBottom = lineTop + lineHeight;


	f32 msgBottom = lineTop - linePad;
	f32 leftOffset = scaledY * 4;

	// draw submission text and cursor
	if (Console->IsShowing())
	{
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
		
		TVector<KString> autoSuggestions;
		KTimePoint lastUpdate;

		bool drawCursor = false;
		{
			std::lock_guard<std::mutex> lock(Console->TextMutex);

			f64 flash = KTime::Since(Console->LastCursorFlashTime);
			if (i32(flash * 2) % 2 == 0) drawCursor = true;

			if (Console->bPendingTextUpdate)
			{
				KString pos = Console->CurrentString.GetSubstring(0, Console->CursorPosition);

				if (Console->SubmissionLayoutHandle > 0)
					r->HUD_DeleteTextLayout(Console->SubmissionLayoutHandle);
				if (Console->CursorPosHandle > 0)
					r->HUD_DeleteTextLayout(Console->CursorPosHandle);

				Console->SubmissionLayoutHandle = r->HUD_CreateTextLayout(
					Console->CurrentString, FontUsage, 999, 0, Console->SubmissionLayoutHandle);

				Console->CursorPosHandle = r->HUD_CreateTextLayout(
					pos, FontUsage, 999, 0, Console->CursorPosHandle);
			}

			GetAutoSuggestions(autoSuggestions, lastUpdate);
		}

		r->HUD_DrawTextLayout(Console->SubmissionLayoutHandle,
			{ rect.left + leftOffset, lineTop + linePad });

		if (drawCursor && Console->IsInputFocus())
		{
			f32 cursorX = rect.left + leftOffset + r->HUD_GetTextWidth(Console->CursorPosHandle);
			r->HUD_DrawLine(cursorX, lineTop + linePad, cursorX, lineBottom - linePad);
		}

		bool updateHandles = false;
		if (KTime::Between(lastUpdate, LastAutoCompleteUpdateTime) != 0)
		{
			LastAutoCompleteUpdateTime = lastUpdate;
			AutoCompleteHandles.resize(autoSuggestions.size());
			updateHandles = true;
		}

		if (autoSuggestions.size() > 0)
		{
			K_ASSERT(AutoCompleteHandles.size() == autoSuggestions.size(), "auto complete handles size didnt match suggestions size");
			if (updateHandles)
			{
				for (u32 i = 0; i < autoSuggestions.size(); i++)
				{
					const KString& str = autoSuggestions[i];
					AutoCompleteHandles[i] = r->HUD_CreateTextLayout(str, EFontUsage::Console, 1000, 1000);
				}
			}

			KHudPointF p;
			p.x = rect.left;
			p.y = rect.bottom;
			u32 height = r->HUD_GetTextHeight(AutoCompleteHandles[0]);

			KHudRectF back;
			back.top = rect.bottom;
			back.bottom = back.top + height * AutoCompleteHandles.size();
			back.left = rect.left;
			back.right = viewX * .3;
			r->HUD_SetDrawColor(BackgroundColor);
			r->HUD_FillRect(back);
			r->HUD_SetDrawColor(FColor8(255, 255, 255, 255));
			for (u32 h : AutoCompleteHandles)
			{
				r->HUD_DrawTextLayout(h, p);
				p.y += height;
			}
		}
	}

	// draw visible entries
	{
		std::lock_guard<std::mutex> lock(Console->MessageMutex);
		DeleteRemovedLayouts(false);
		KTextMessage* msg = Console->MessageHead.get();

		u32 currentMsgLines = 0;

		// find starting message based on scroll
		for (i32 i = 0; i < Console->ScrollCount && msg; i++)
		{
			if (currentMsgLines == 0)
				currentMsgLines = r->HUD_GetTextLineCount(msg->LayoutHandle);

			if (currentMsgLines > 0)
				currentMsgLines--;

			if (currentMsgLines == 0)
				msg = msg->Next.get();
		}

		while (msg && msgBottom > msgTop)
		{
			if (msg->LayoutHandle == 0)
			  msg->LayoutHandle = r->HUD_CreateTextLayout(
				msg->Message, FontUsage, 1, 1);

			f32 h = r->HUD_GetTextHeight(msg->LayoutHandle);
			f32 alpha = GetMessageOpacity(msg);
			if (bDrawShadowedText)
			{
				DrawTextShadowed(msg->LayoutHandle, { rect.left + leftOffset, msgBottom - h }, 1.5 * scaledY,
					FColor32(msg->r, msg->g, msg->b, alpha).To8(), FColor32(0, 0, 0, alpha).To8());
			}
			else
			{
				r->HUD_SetDrawColor(FColor32(msg->r, msg->g, msg->b, 1));
				r->HUD_DrawTextLayout(msg->LayoutHandle, { rect.left + leftOffset, msgBottom - h });
			}

			msg = msg->Next.get();
			if (msg) msgBottom -= h;// * r->HUD_GetTextLineCount(msg->LayoutHandle);
		}
	}
}

#endif