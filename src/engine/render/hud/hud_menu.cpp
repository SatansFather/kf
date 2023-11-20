#if !_SERVER && !_COMPILER

#include "hud_menu.h"
#include "engine/menus/option_menu.h"
#include "engine/render/interface/render_interface.h"
#include "../color.h"
#include "../interface/bitmap.h"
#include "../../input/input_processor.h"

KHudMenu::KHudMenu()
{
	KRenderInterface* iface = GetRenderInterface();

	for (KBasicMenu* menu : KBasicMenu::AllMenus)
	{
		HeaderLayouts[menu] = iface->HUD_CreateTextLayout(menu->Header, EFontUsage::MainLogo, 1, 1);
		AddMenuOptions(menu);
	}

	//EnabledLayout = iface->HUD_CreateTextLayout("Enabled", EFontUsage::MenuButtonSmall, 1, 1);
	//DisabledLayout = iface->HUD_CreateTextLayout("Disabled", EFontUsage::MenuButtonSmall, 1, 1);
	SliderLayout = iface->HUD_CreateTextLayout("----------------", EFontUsage::MenuButtonSmall, 1, 1);
	SliderBarLayout = iface->HUD_CreateTextLayout("I", EFontUsage::MenuButtonSmall, 1, 1);
}

void KHudMenu::Draw()
{
	// WARNING: hell

	KBasicMenu* menu = KBasicMenu::GetTopMenu();
	KRenderInterface* iface = GetRenderInterface();
	if (!menu) return;
	
	bool optionMenu = dynamic_cast<KOptionMenu*>(menu);

	if (menu == GetServerBrowser() || menu == GetMapsMenu())
	{
		menu->OptionsLock.lock();
		AddMenuOptions(menu);
	}

	u32 headerLayout = HeaderLayouts[menu];
	TVector<u32>& optionLayouts = OptionLayouts[menu];
	u32 selected = menu->SelectedOption;

	f32 cursorX = -1;
	f32 cursorY = -1;
	if (menu->ShouldUseMouseSelection())
	{
		menu->GetCursorPosition(cursorX, cursorY);
		selected = -1;
	}

	const f32 viewX = GetViewportX();
	const f32 viewY = GetViewportY();
	const f32 scaleY = GetScaledY();
	const f32 scaleX = GetScaledX();
	const f32 xMid = viewX / 2.f;
	const bool useMouse = menu->ShouldUseMouseSelection();
	const bool leftMouse = KInputProcessor::LeftMouseDown();

	KHudRectF background;
	background.left = 0;
	background.top = 0;
	background.right = viewX;
	background.bottom = viewY;
	iface->HUD_SetDrawColor(FColor8(20, 20, 20, 100));
	//iface->HUD_FillRect(background);
	
	{
		u32 w = iface->splatter->GetWidth();
		u32 h = iface->splatter->GetHeight();
		f32 x = viewX / 2;
		f32 y = viewY / 2;
	
		iface->HUD_DrawBitmap(iface->splatter.get(), x - viewY/2, y - viewY/2 - scaleY * 100, viewY, viewY);
	}

	KHudPointF point;

	{
		point.y = viewY * .15f;
		f32 h = iface->HUD_GetTextHeight(headerLayout);
		f32 w = iface->HUD_GetTextWidth(headerLayout);
		point.x = xMid - (w / 2.f);
		DrawTextShadowed(headerLayout, point, 4, FColor8(255, 20, 50, 255), FColor8(40, 40, 0, 255));
	}

	point.y = viewY * .23f;

	for (u32 i = 0; i < optionLayouts.size(); i++)
	{
		KMenuOption* option = menu->Options[i].get();
		if (!option->bActive) continue;
		
		f32 h = iface->HUD_GetTextHeight(optionLayouts[i]);
		f32 w = iface->HUD_GetTextWidth(optionLayouts[i]);
		point.x = xMid - (w / 2.f);
				
		KHudRectF select;
		select.left = viewX * .42;
		select.right = viewX * .58;
		select.top = point.y;
		select.bottom = point.y + h;

		bool customOption = false;
		bool noHighlight = false;

		KMenuOption_CheckBox* box		= nullptr;
		KMenuOption_RadioButton* button = nullptr;
		KMenuOption_Slider* slider		= nullptr;

		box = dynamic_cast<KMenuOption_CheckBox*>(option);
		if (!box) slider = dynamic_cast<KMenuOption_Slider*>(option);
		if (!slider) button = dynamic_cast<KMenuOption_RadioButton*>(option);

		f32 optionX = xMid + 20 * scaleY;

		if (box || button || slider) customOption = true;
		//if (button || slider) noHighlight = true;
		
		if (box)
		{
			f32 textWidth = 
				KMax(iface->HUD_GetTextWidth(box->DisabledLayout),
				iface->HUD_GetTextWidth(box->EnabledLayout));

			select.left = viewX / 2;
			select.right = select.left + textWidth + 40 * scaleY;
		}
		else if (slider)
		{
			select.left = optionX;// + iface->HUD_GetTextWidth(SliderLayout) + (20 * scaleY);
			select.right = select.left + iface->HUD_GetTextWidth(SliderLayout);
		}

		if (cursorX > select.left && cursorX < select.right &&
			cursorY > select.top && cursorY < select.bottom
			&& option->CanBeSelected())
			selected = i;

		FColor8 textColor = FColor8(255, 100, 100, 255);
		FColor8 highlightColor = FColor8(255, 200, 200, 255);
		FColor8 shadowColor = FColor8(40, 40, 0, 255);

		if (!option->CanBeSelected())
		{
			textColor = FColor8(100, 100, 100, 255);
			highlightColor = FColor8(100, 100, 100, 255);
		}

		bool shouldHighlight = (i == selected && !noHighlight);
			
		if (customOption) point.x = xMid - w - 20 * scaleY;

		if (box)
		{
			u8 value = *box->Value;

			DrawTextShadowed(optionLayouts[i], point, 2, !useMouse && shouldHighlight ? highlightColor : textColor, shadowColor);

			point.x = optionX;
			DrawTextShadowed(value ? box->EnabledLayout : box->DisabledLayout, point, 2, 
				useMouse && shouldHighlight ? highlightColor : textColor, shadowColor);
		}
		else if (slider)
		{
			f32 value = -1;

			if (useMouse && selected == i && leftMouse)
				value = MapRange(cursorX, select.left, select.right, slider->SliderMin, slider->SliderMax);

			slider->Mutex.lock();

			if (value != -1) 
			{
				slider->Mutex.unlock();
				slider->SetValue(value, true);
				slider->Mutex.lock();
			}
			value = *slider->Value;

			bool pendingUpdate = slider->bPendingValueLayoutUpdate;
			slider->bPendingValueLayoutUpdate = false;
			slider->Mutex.unlock();
				
			DrawTextShadowed(optionLayouts[i], point, 2, !useMouse && shouldHighlight ? highlightColor : textColor, shadowColor);
			point.x = optionX;
			DrawTextShadowed(SliderLayout, point, 2, textColor, shadowColor);

			f32 maxW = iface->HUD_GetTextWidth(SliderLayout);
			f32 barW = iface->HUD_GetTextWidth(SliderBarLayout);
			point.x = MapRange(value, slider->SliderMin, slider->SliderMax, optionX, optionX + maxW - barW);

			DrawTextShadowed(SliderBarLayout, point, 2, highlightColor, shadowColor);

			if (slider->LastRenderedValue != value)
			{
				KString strVal;
				if (slider->bForceInt)
					strVal = KString(i32(value));
				else
					strVal = KString(value, 2);
					
				slider->ValueLayout = iface->HUD_CreateTextLayout(strVal, 
					EFontUsage::MenuButtonSmall, 1, 1, slider->ValueLayout);
				slider->LastRenderedValue = value;
			}

			point.x = optionX + maxW + (20 * scaleY);
			iface->HUD_SetDrawColor(textColor);
			DrawTextShadowed(slider->ValueLayout, point, 2, textColor, shadowColor);
		}
		else
			DrawTextShadowed(optionLayouts[i], point, 2, shouldHighlight ? highlightColor : textColor, FColor8(40, 40, 0, 255));

		point.y += h + (optionMenu ? scaleY * 5 : 0);
	}

	// if active cursor hovers over nothing, dont allow a selection
	if (useMouse) menu->SelectedOption = selected;

	if (menu == GetServerBrowser() || menu == GetMapsMenu())
		menu->OptionsLock.unlock();
}

void KHudMenu::AddMenuOptions(class KBasicMenu* menu)
{
	KRenderInterface* iface = GetRenderInterface();
	OptionLayouts[menu].clear();
	for (UPtr<KMenuOption>& option : menu->Options)
	{
		OptionLayouts[menu].push_back(iface->HUD_CreateTextLayout(
			option->Text, option->FontUsage, 1, 1));

		if (KMenuOption_CheckBox* box = dynamic_cast<KMenuOption_CheckBox*>(option.get()))
		{
			box->EnabledLayout = iface->HUD_CreateTextLayout(box->Enabled, EFontUsage::MenuButtonSmall, 1, 1);
			box->DisabledLayout = iface->HUD_CreateTextLayout(box->Disabled, EFontUsage::MenuButtonSmall, 1, 1);
		}
	}
}

#endif

