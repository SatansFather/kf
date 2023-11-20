#pragma once

#if !_SERVER

#include "basic_menu.h"

enum class EOptionType
{
	Video,
	Audio,
	Game,
	Mouse,
	Control,
	Interface,
	Misc
};

class KOptionMenu : public KBasicMenu
{
public:
	class KMenuOption_CheckBox* AddBooleanOption(const KString& name, u8* value, const KString& consoleCommand, const KString& enabled = "Enabled", const KString& disabled = "Disabled");
	class KMenuOption_Slider* AddSliderOption(const KString& name, f32* value, f32 min, f32 max, f32 step, const KString& consoleCommand, const KString& tooltip = "", bool useInt = false);
	class KMenuOption* AddSubcategory(const KString& name);
};

KOptionMenu* GetOptionMenu(EOptionType type);

#endif