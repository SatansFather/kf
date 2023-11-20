#if !_SERVER

#include "option_menu.h"
#include "../console/engine_console.h"
#include "menu_option.h"

KOptionMenu* GetOptionMenu(EOptionType type)
{
	switch (type)
	{
		case EOptionType::Video:
		{
			static KOptionMenu video;
			return &video;
		}
		case EOptionType::Audio:
		{
			static KOptionMenu audio;
			return &audio;
		}
		case EOptionType::Game:
		{
			static KOptionMenu game;
			return &game;
		}
		case EOptionType::Mouse:
		{
			static KOptionMenu mouse;
			return &mouse;
		}
		case EOptionType::Control:
		{
			static KOptionMenu control;
			return &control;
		}
		case EOptionType::Interface:
		{
			static KOptionMenu iface;
			return &iface;
		}
		case EOptionType::Misc:
		{
			static KOptionMenu misc;
			return &misc;
		}
	}
	return nullptr;
}

KMenuOption_CheckBox* KOptionMenu::AddBooleanOption(const KString& name, u8* value, const KString& consoleCommand, const KString& enabled, const KString& disabled)
{
	auto option = std::make_unique<KMenuOption_CheckBox>();
	option->FontUsage = EFontUsage::MenuButtonSmall;
	option->Text = name;
	option->ConsoleCommand = consoleCommand;
	option->Enabled = enabled;
	option->Disabled = disabled;
	KMenuOption_CheckBox* op = dynamic_cast<KMenuOption_CheckBox*>(option.get());
	op->Value = value;
	const auto func = [op, consoleCommand]() -> void
	{
		ExecuteConsoleCommand(consoleCommand + " " + KString(*op->Value));
	};
	option->RunFunction = func;
	KMenuOption_CheckBox* ret = option.get();
	Options.push_back(std::move(option));
	return ret;
}

KMenuOption_Slider* KOptionMenu::AddSliderOption(const KString& name, f32* value, f32 min, f32 max, f32 step, const KString& consoleCommand, const KString& tooltip /*= ""*/, bool useInt /*= false*/)
{	
	auto option = std::make_unique<KMenuOption_Slider>();
	option->FontUsage = EFontUsage::MenuButtonSmall;
	option->Text = name;
	KMenuOption_Slider* op = dynamic_cast<KMenuOption_Slider*>(option.get());
	op->SliderMin = min;
	op->SliderMax = max;
	op->Step = step;
	op->Value = value;
	const auto func = [op, consoleCommand]() -> void
	{
		ExecuteConsoleCommand(consoleCommand + " " + KString(*op->Value));
	};
	option->RunFunction = func;
	KMenuOption_Slider* ret = option.get();
	Options.push_back(std::move(option));
	return ret;
}

KMenuOption* KOptionMenu::AddSubcategory(const KString& name)
{
	auto option = std::make_unique<KMenuOption>();
	option->FontUsage = EFontUsage::MenuButtonLarge;
	option->bSelectable = false;
	option->Text = name;
	KMenuOption* ret = option.get();
	Options.push_back(std::move(option));
	return ret;
}

#endif