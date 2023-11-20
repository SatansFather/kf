#if !_SERVER

#include "../game_instance.h"
#include "basic_menu.h"
#include "option_menu.h"
#include "../console/engine_console.h"
#include "../render/interface/render_interface.h"
#include "../net/player.h"

void KGameInstance::InitMainMenu()
{
	KBasicMenu* menu = GetMainMenu();
	UPtr<KMenuOption> option;

	menu->Header = "Karnage Freak";

	option = std::make_unique<KMenuOption>();
	option->Text = "Server Browser";
	const auto browser = []() -> void { GetServerBrowser()->OpenMenu(); };
	option->RunFunction = browser;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Maps";
	const auto maps = []() -> void { GetMapsMenu()->OpenMenu(); };
	option->RunFunction = maps;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Options";
	const auto options = []() -> void { GetOptionsMenu()->OpenMenu(); };
	option->RunFunction = options;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Quit";
	const auto quit = []() -> void { KGameInstance::Get().ExitGame(); };
	option->RunFunction = quit;
	menu->Options.push_back(std::move(option));
}

void KGameInstance::InitIngameMenu()
{
	KBasicMenu* menu = GetIngameMenu();
	UPtr<KMenuOption> option;

	menu->Header = "Karnage Freak";

	option = std::make_unique<KMenuOption>();
	option->Text = "Resume";
	const auto resume = [menu]() -> void { menu->CloseMenu(); };
	option->RunFunction = resume;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Server Browser";
	const auto browser = []() -> void { GetServerBrowser()->OpenMenu(); };
	option->RunFunction = browser;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Options";
	const auto options = []() -> void { GetOptionsMenu()->OpenMenu(); };
	option->RunFunction = options;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Save";
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Load";
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Main Menu";
	const auto mainmenu = []() -> void 
	{
		GetEngineConsole()->bPrintBack = false;
		GetEngineConsole()->ProcessSubmission("map mainmenu");
		GetEngineConsole()->bPrintBack = true;
		KBasicMenu::CloseAllMenus();
	};
	option->RunFunction = mainmenu;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Quit";
	const auto quit = []() -> void { KGameInstance::Get().ExitGame(); };
	option->RunFunction = quit;
	menu->Options.push_back(std::move(option));
}

void KGameInstance::InitMapsMenu()
{
#if !_COMPILER
	KBasicMenu* menu = GetMapsMenu();
	menu->Options.clear();
	menu->Header = "Maps";
	for (const auto& kv : LocalMaps)
	{
		if (kv.second.MapFileName.Get() == "emptymap" || kv.second.MapFileName.Get() == "mainmenu")
			continue;

		UPtr<KMenuOption> option = std::make_unique<KMenuOption>();
		option->Text = kv.second.MapIngameName;
		const auto map = [&]() -> void 
		{
			GetEngineConsole()->bPrintBack = false;
			GetEngineConsole()->ProcessSubmission("map " + kv.second.MapFileName);
			GetEngineConsole()->bPrintBack = true;
			KBasicMenu::CloseAllMenus();
		};
		option->RunFunction = map;
		menu->Options.push_back(std::move(option));
	}
	menu->AddBackButton();
#endif
}

void KGameInstance::InitServerBrowser()
{
	KBasicMenu* menu = GetServerBrowser();
	UPtr<KMenuOption> option;

	menu->Header = "Server Browser";

	menu->AddBackButton();
}

void KGameInstance::InitJoinMenu()
{
	KBasicMenu* menu = GetJoinMenu();
	UPtr<KMenuOption> option;

	option = std::make_unique<KMenuOption>();
	option->Text = "Join Game";
	const auto join = [&]() -> void
	{
		if (KNetPlayer* p = GetLocalNetPlayer())
		{
			if (p->ViewedPlayerIndex != p->OwningPlayerIndex)
			{
				GetLocalPlayer()->bPendingSpawnRequest = true;
				p->ViewedPlayerIndex = p->OwningPlayerIndex;
			}			
		}
		KBasicMenu::CloseAllMenus();
	};
	option->RunFunction = join;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Menu Menu";
	const auto mainmenu = []() -> void 
	{
		GetEngineConsole()->bPrintBack = false;
		GetEngineConsole()->ProcessSubmission("map mainmenu");
		GetEngineConsole()->bPrintBack = true;
		KBasicMenu::CloseAllMenus();
	};
	option->RunFunction = mainmenu;
	menu->Options.push_back(std::move(option));
}

void KGameInstance::InitOptionsMenu()
{
	KBasicMenu* menu = GetOptionsMenu();
	UPtr<KMenuOption> option;
	
	menu->Header = "Options";
	
	option = std::make_unique<KMenuOption>();
	option->Text = "Search...";
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Video";
	const auto video = []() -> void { GetOptionMenu(EOptionType::Video)->OpenMenu(); };
	option->RunFunction = video;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Audio";
	const auto audio = []() -> void { GetOptionMenu(EOptionType::Audio)->OpenMenu(); };
	option->RunFunction = audio;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Game";
	const auto game = []() -> void { GetOptionMenu(EOptionType::Game)->OpenMenu(); };
	option->RunFunction = game;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Mouse";
	const auto mouse = []() -> void { GetOptionMenu(EOptionType::Mouse)->OpenMenu(); };
	option->RunFunction = mouse;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Control";
	const auto control = []() -> void { GetOptionMenu(EOptionType::Control)->OpenMenu(); };
	option->RunFunction = control;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Interface";
	const auto iface = []() -> void { GetOptionMenu(EOptionType::Interface)->OpenMenu(); };
	option->RunFunction = iface;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Misc";
	const auto misc = []() -> void { GetOptionMenu(EOptionType::Misc)->OpenMenu(); };
	option->RunFunction = misc;
	menu->Options.push_back(std::move(option));

	option = std::make_unique<KMenuOption>();
	option->Text = "Back";
	const auto back = [menu]() -> void { menu->CloseMenu(); };
	option->RunFunction = back;
	menu->Options.push_back(std::move(option));
}

void KGameInstance::InitVideoOptionsMenu()
{
	KOptionMenu* menu = GetOptionMenu(EOptionType::Video);
	menu->Header = "Video Options";

	menu->AddSubcategory("Display");
	menu->AddSliderOption("Frame Rate", &GetDrawConfig()->MaxFramerate, 30, 500, 1, "maxfps");
	menu->AddSliderOption("Render Scale", &GetDrawConfig()->RenderScale, .01, 1, .05, "r_scale");
	menu->AddBooleanOption("Downscale Filtering", &GetDrawConfig()->bFilterRenderScale, "r_scaleFilter");
	menu->AddBooleanOption("Scale Metric", &GetDrawConfig()->RenderScaleMetric, "r_scaleMetric", "Area", "Dimensions");
	menu->AddSubcategory("Field of View");
	menu->AddSliderOption("Vertical Fov", &GetDrawConfig()->VFov, 1, 175, 1, "fov");
	menu->AddSliderOption("Horizontal Fov", &GetDrawConfig()->HFov, 1, 175, 1, "hfov");
	//menu->AddSubcategory("Rate");
	//menu->AddBooleanOption("Interpolate", &GetDrawConfig()->bInterpolate, "r_interpolate");
	menu->AddBackButton();
}

void KGameInstance::InitAudioOptionsMenu()
{
	KOptionMenu* menu = GetOptionMenu(EOptionType::Audio);
	menu->Header = "Audio Options";

	menu->AddSliderOption("Master", &GetUserConfig()->Audio.MasterVolume, 0, 1, .05, "volume");

	menu->AddBackButton();
}

void KGameInstance::InitGameOptionsMenu()
{
	KOptionMenu* menu = GetOptionMenu(EOptionType::Game);
	menu->Header = "Game Options";

	menu->AddBooleanOption("Enable Gore", &GetUserConfig()->Game.bGore, "", "Fuck Yeah", "Yeah Dude");
	menu->AddBooleanOption("Pause In Console", &GetUserConfig()->Game.bConsolePause, "consolepause");

	menu->AddSliderOption("Bob Scale", &GetUserConfig()->Game.BobScale, 0, 1, .1, "bobscale");
	menu->AddSliderOption("Roll Scale", &GetUserConfig()->Game.RollScale, 0, 1, .1, "rollscale");
	menu->AddSliderOption("Shake Scale", &GetUserConfig()->Game.ShakeScale, 0, 1, .1, "shakescale");
	menu->AddSliderOption("Concussion Scale", &GetUserConfig()->Game.ConcussionScale, 0, 1, .1, "concussionscale");
	menu->AddBooleanOption("Weapon Fire Flash", &GetUserConfig()->Game.bWeaponFlash, "weaponflash", "Enabled", "Disable");

	menu->AddBackButton();
}

void KGameInstance::InitMouseOptionsMenu()
{
	KOptionMenu* menu = GetOptionMenu(EOptionType::Mouse);
	menu->Header = "Mouse Options";

	auto measure = menu->AddBooleanOption("Measurement", &GetUserConfig()->Mouse.Horizontal.bUseDistance, "m_measurement", "Distance", "Sensitivity");
	
	auto sense = menu->AddSliderOption("Sensitivity", &GetUserConfig()->Mouse.Horizontal.Sensitivity, 0, 10, .01, "sensitivity");
	sense->RequiredCheckBox = measure;
	sense->bRequiredCheckBoxSetting = 0;

	menu->AddBooleanOption("Invert Y", &GetUserConfig()->Mouse.bInvertY, "invertY");
	menu->AddBooleanOption("Invert X", &GetUserConfig()->Mouse.bInvertX, "invertX");

	auto unit = menu->AddBooleanOption("Distance Unit", &GetUserConfig()->Mouse.Horizontal.bUseInches, "m_unit", "Inches", "Centimeters");
	auto spin = menu->AddSliderOption("Units Per Spin", &GetUserConfig()->Mouse.Horizontal.UnitsPerSpin, 0, 50, 1, "m_unitsPerSpin");
	auto deg = menu->AddSliderOption("Spin Degrees", &GetUserConfig()->Mouse.Horizontal.SpinDegrees, 0, 360, 45, "m_degrees");
	auto dpi = menu->AddSliderOption("Mouse DPI", &GetUserConfig()->Mouse.Horizontal.MouseDPI, 0, 10000, 50, "m_dpi", "", true);
	dpi->bForceInt = true;

	unit->RequiredCheckBox = measure;
	spin->RequiredCheckBox = measure;
	deg->RequiredCheckBox = measure;
	dpi->RequiredCheckBox = measure;

	unit->bRequiredCheckBoxSetting = 1;
	spin->bRequiredCheckBoxSetting = 1;
	deg->bRequiredCheckBoxSetting = 1;
	dpi->bRequiredCheckBoxSetting = 1;
	

	menu->AddBackButton();
}

void KGameInstance::InitControlOptionsMenu()
{
	KOptionMenu* menu = GetOptionMenu(EOptionType::Control);
	menu->Header = "Control Options";

	menu->AddBackButton();
}

void KGameInstance::InitInterfaceOptionsMenu()
{
	KOptionMenu* menu = GetOptionMenu(EOptionType::Interface);
	menu->Header = "Interface Options";

	menu->AddBackButton();
}

void KGameInstance::InitMiscOptionsMenu()
{
	KOptionMenu* menu = GetOptionMenu(EOptionType::Misc);
	menu->Header = "Misc Options";

	menu->AddBackButton();
}

#endif