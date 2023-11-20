#if !_SERVER

#include "config.h"
#include "engine/global/paths.h"
#include "local_player.h"
#include <fstream>
#include "engine/render/interface/render_interface.h"
#include "../game_instance.h"
#include "../audio/audio.h"
#include "../net/net_interface.h"
#include "../input/input_processor.h"
#include "../input/key_names.h"
#include "../net/data/address.h"

#if !_COMPILER

#define UPDATE_CVAR(type, var) GetUserConfig()->UpdateValueFromConsole<type>(&(var), val);
#define UPDATE_CVAR_MINMAX(type, var, min, max) GetUserConfig()->UpdateValueFromConsole<type>(&(var), val, min, max);

f32 Fov_V_to_H(f32 v, f32 ratio = 0)
{
	v = glm::radians(v);
	if (ratio == 0)
	{
		f32 x = GetViewportX();
		f32 y = GetViewportY();
		ratio = x / y;
	}

	f32 h = atan(tan(v / 2.f) * (ratio)) * 2;
	return glm::degrees(h);
}

f32 Fov_H_to_V(f32 h)
{
	h = glm::radians(h);
	f32 x = GetViewportX();
	f32 y = GetViewportY();
	f32 v = atan(tan(h / 2.f) * (y / x)) * 2;
	return glm::degrees(v);
}

KString CCOM_RenderScale(const KString & val)
{
	KString ret = UPDATE_CVAR_MINMAX(f32, GetDrawConfig()->RenderScale, 0, 32);
	GetRenderInterface()->bPendingRescale = true;
	GetRenderInterface()->UpdateRenderScaleSampler(GetDrawConfig()->RenderScale >= 1 ? 0 : -1);
	return "Render Scale: " + KString(GetDrawConfig()->RenderScale, 2) + " - Scaled to " + KString(GetRenderResX()).ToInt() + "x" + KString(GetRenderResY()).ToInt();
}

KString CCOM_RenderScaleFilter(const KString& val)
{
	KString str = UPDATE_CVAR(u8, GetDrawConfig()->bFilterRenderScale);
	GetRenderInterface()->UpdateRenderScaleSampler();
	return str;
}

KString CCOM_RenderScaleMetric(const KString& val)
{
	KString ret = UPDATE_CVAR_MINMAX(u8, GetDrawConfig()->RenderScaleMetric, 0, 1);
	GetRenderInterface()->bPendingRescale = true;
	return ret;
}

KString CCOM_Interpolate(const KString& val)
{
	return UPDATE_CVAR(u8, GetDrawConfig()->bInterpolate);
}

KString CCOM_Fov(const KString& val)
{
	KString ret = UPDATE_CVAR_MINMAX(f32, GetDrawConfig()->VFov, 1.f, 179.f);

	LockConfig();
	f32 h = Fov_V_to_H(GetDrawConfig()->VFov);
	UnlockConfig();
	GetUserConfig()->UpdateValueFromConsole<f32>(&(GetDrawConfig()->HFov), h);

	return ret;
}

KString CCOM_FovAdd(const KString& val)
{
	LockConfig();
	f32 fov = GetDrawConfig()->VFov + 1;
	UnlockConfig();
	KString sfov = KString(fov);
	return CCOM_Fov(sfov);
}

KString CCOM_FovSub(const KString& val)
{
	LockConfig();
	f32 fov = GetDrawConfig()->VFov - 1;
	UnlockConfig();
	KString sfov = KString(fov);
	return CCOM_Fov(sfov);
}

KString CCOM_HFov(const KString& val)
{
	KString ret = UPDATE_CVAR_MINMAX(f32, GetDrawConfig()->HFov, 1.f, 179.f);

	LockConfig();
	f32 v = Fov_H_to_V(GetDrawConfig()->HFov);
	UnlockConfig();
	GetUserConfig()->UpdateValueFromConsole<f32>(&(GetDrawConfig()->VFov), v);

	return ret;
}

KString CCOM_MaxFPS(const KString& val)
{
	return UPDATE_CVAR_MINMAX(f32, GetDrawConfig()->MaxFramerate, 0, -1);
}

KString CCOM_Brightness(const KString& val)
{
	return UPDATE_CVAR_MINMAX(f32, GetDrawConfig()->Brightness, 0, 2);
}

KString CCOM_Contrast(const KString& val)
{
	return UPDATE_CVAR_MINMAX(f32, GetDrawConfig()->Contrast, 0, 2);
}

KString CCOM_Saturation(const KString& val)
{
	return UPDATE_CVAR_MINMAX(f32, GetDrawConfig()->Saturation, 0, 1000);
}

KString CCOM_Flip(const KString& val)
{
	KString v = val;
	if (v.Get() != "1") v = "0";
	return UPDATE_CVAR_MINMAX(f32, GetDrawConfig()->Flip, 0, 1);
}

KString CCOM_Res(const KString& val)
{
	KRenderInterface* r = GetRenderInterface();

	LockConfig();
	KString res = KString(GetDrawConfig()->GetResX()) + KString("x") + KString(GetDrawConfig()->GetResY());
	UnlockConfig();

	TVector<KString> out;
	val.SplitByChar(out, 'x');

	if (out.size() == 2)
	{
		u32 x = 0;
		if (!out[0].ToU32Safe(x)) return res;
		u32 y = 0;
		if (!out[1].ToU32Safe(y)) return res;

		r->ResizeResolution(x, y);

		LockConfig();
		GetUserConfig()->GetDrawConfig()->SetResolution(x, y);
		GetUserConfig()->Save("config");
		UnlockConfig();

		return "";
	}
	else if (val == KString("reset"))
	{
		LockConfig();
		u32 x = GetDrawConfig()->GetResX();
		u32 y = GetDrawConfig()->GetResY();
		r->ResizeResolution(x, y);
		UnlockConfig();
		return "";
	}

	KString current = KString(GetRenderResX()) + "x" + KString(GetRenderResY());

	return res + KString((current.Get() != res.Get()) ? KString(" -- Current Window: " + current) : KString(""));
}

KString CCOM_MouseSense(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Mouse.Horizontal.Sensitivity);
}

KString CCOM_MouseSenseY(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Mouse.Vertical.Sensitivity);
}

KString CCOM_InvertY(const KString& val)
{	
	return UPDATE_CVAR(u8, GetUserConfig()->Mouse.bInvertY);
}

KString CCOM_InvertX(const KString& val)
{
	return UPDATE_CVAR(u8, GetUserConfig()->Mouse.bInvertX);
}

KString CCOM_M_Measurement(const KString& val)
{
	return UPDATE_CVAR(u8, GetUserConfig()->Mouse.Horizontal.bUseDistance);
}
KString CCOM_M_Unit(const KString& val)
{
	return UPDATE_CVAR(u8, GetUserConfig()->Mouse.Horizontal.bUseInches);
}

KString CCOM_M_UnitsPerSpin(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Mouse.Horizontal.UnitsPerSpin);
}

KString CCOM_M_DPI(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Mouse.Horizontal.MouseDPI);
}

KString CCOM_M_Degrees(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Mouse.Horizontal.SpinDegrees);
}

KString CCOM_ConsolePause(const KString& val)
{
	KString ret = UPDATE_CVAR(u8, GetUserConfig()->Game.bConsolePause);
	LockConfig();
	KGameInstance::Get().bConsolePause = GetUserConfig()->Game.bConsolePause;
	UnlockConfig();
	return ret;
}

KString CCOM_WeaponFlash(const KString& val)
{
	KString ret = UPDATE_CVAR(u8, GetUserConfig()->Game.bWeaponFlash);
	LockConfig();
	KGameInstance::Get().bWeaponFlash = GetUserConfig()->Game.bWeaponFlash;
	UnlockConfig();
	return ret;
}

KString CCOM_BobScale(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Game.BobScale);
}

KString CCOM_RollScale(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Game.RollScale);
}

KString CCOM_ShakeScale(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Game.ShakeScale);
}

KString CCOM_ConcussionScale(const KString& val)
{
	return UPDATE_CVAR(f32, GetUserConfig()->Game.ConcussionScale);
}

KString CCOM_MasterVolume(const KString& val)
{
	KString ret = UPDATE_CVAR_MINMAX(f32, GetUserConfig()->Audio.MasterVolume, 0, 1);
	KGameInstance::Get().MasterVolume = GetUserConfig()->Audio.MasterVolume;
	if (KGameInstance::Get().IsPaused())
		KAudio::UpdateAudio();
		
	return ret;
}

KString CCOM_Name(const KString& val)
{
	KUserConfig* cfg = GetUserConfig();

	KString v = val;
	v.TrimInPlace();
	if (v.Size() > 32) v = val.GetSubstring(0, 32);

	LockConfig();
	KString ret = "";
	if (v.IsEmpty()) ret = cfg->Net.PlayerName;

	if (v.Get() != cfg->Net.PlayerName.Get())
	{
		cfg->Net.PlayerName = v;

		if (KNetInterface* iface = KGameInstance::Get().GetNetInterface())
		{
			iface->bPendingNameChange = true;
			iface->FlagPendingScoreUpdate();
		}
	}
	cfg->Save(cfg->GetActiveConfig());
	UnlockConfig();

	return ret;
}

KString CCOM_NetFramePad(const KString& val)
{
	return UPDATE_CVAR_MINMAX(u32, GetUserConfig()->Net.FramePad, 0, 10);
}

KString CCOM_NetSnapshotBuffer(const KString& val)
{
	return UPDATE_CVAR_MINMAX(u32, GetUserConfig()->Net.SnapshotBuffer, 0, 10);
}

#endif

void KUserConfig::Save(const KString& profile, bool lock /*= false*/)
{
	if (lock)
	{
		std::lock_guard<std::mutex> lock(ConfigMutex);
		Save(profile, false);
		return;
	}

	std::ofstream file(CFG_DIR + (profile == KString("") ? "config" : profile.Get()) + ".cfg");
	
	file << "[Karnage Freak Config]\n";

	file << "\n[Video]\n";
	file << "MaxFPS " << KString(Video.MaxFramerate) << "\n";
	file << "ResX " << KString((u16)std::round(Video.ResX)) << "\n";
	file << "ResY " << KString((u16)std::round(Video.ResY)) << "\n";
	file << "RenderScale " << KString(Video.RenderScale) << "\n";
	file << "DownscaleFilter " << KString(Video.bFilterRenderScale) << "\n";
	file << "ScaleMetric " << KString(Video.RenderScaleMetric) << "\n";
	file << "Fullscreen " << KString(Video.WindowState == EWindowState::Fullscreen) << "\n";
	file << "VerticalFOV " << KString(Video.VFov) << "\n";
	file << "Interpolate " << KString(Video.bInterpolate) << "\n";
	file << "Brightness " << KString(Video.Brightness) << "\n";
	file << "Contrast " << KString(Video.Contrast) << "\n";
	file << "Saturation " << KString(Video.Saturation) << "\n";
	file << "Flip " << KString(u32(Video.Flip)) << "\n";

	file << "\n[Game]\n";
	file << "BobScale " << KString(Game.BobScale) << "\n";
	file << "RollScale " << KString(Game.RollScale) << "\n";
	file << "ShakeScale " << KString(Game.ShakeScale) << "\n";
	file << "ConcussionScale " << KString(Game.ConcussionScale) << "\n";
	file << "WeaponFlash " << KString(Game.bWeaponFlash) << "\n";
	file << "Gore " << KString(Game.bGore) << "\n";
	file << "ConsolePause " << KString(Game.bConsolePause) << "\n";
	//file << "ReplaySavePerFrame " << STR(bDemoSaveEachFrame) << "\n";
	
	file << "\n[Control]\n";
	file << "InvertY " << KString(Mouse.bInvertY) << "\n";
	file << "InvertX " << KString(Mouse.bInvertY) << "\n";
	file << "SeparateMouseY " << KString(Mouse.bSeparateMouseAxes) << "\n";
	file << "Sensitivity " << KString(Mouse.Horizontal.Sensitivity) << "\n";
	file << "UseDistance " << KString(Mouse.Horizontal.bUseDistance) << "\n";
	file << "UseInches " << KString(Mouse.Horizontal.bUseInches) << "\n";
	file << "SpinDegrees " << KString(Mouse.Horizontal.SpinDegrees) << "\n";
	file << "UnitsPerSpin " << KString(Mouse.Horizontal.UnitsPerSpin) << "\n";
	file << "MouseDPI " << KString(Mouse.Horizontal.MouseDPI) << "\n";
	file << "Y_Sensitivity " << KString(Mouse.Vertical.Sensitivity) << "\n";
	file << "Y_UseDistance " << KString(Mouse.Vertical.bUseDistance) << "\n";
	file << "Y_UseInches " << KString(Mouse.Vertical.bUseInches) << "\n";
	file << "Y_SpinDegrees " << KString(Mouse.Vertical.SpinDegrees) << "\n";
	file << "Y_UnitsPerSpin " << KString(Mouse.Vertical.UnitsPerSpin) << "\n";
	file << "Y_MouseDPI " << KString(Mouse.Vertical.MouseDPI) << "\n";

	file << "\n[Audio]\n";
	file << "MasterVolume " << KString(Audio.MasterVolume) << "\n";

	file << "\n[Net]\n";
	file << "PlayerName " << Net.PlayerName.Trim() << "\n";
	file << "LastConnectedIP " << Net.LastConnectedIP.Trim() << "\n";
	file << "FramePad " << KString(Net.FramePad) << "\n";
	file << "SnapshotBuffer " << KString(Net.SnapshotBuffer) << "\n";

	if (Net.FavoriteServers.size() > 0)
	{
		file << "\n";
		for (const auto& kv : Net.FavoriteServers)
		{
			if (kv.first.Trim() != "local");
				file << "@" << kv.second << " " << kv.first << "\n";
		}
	}

	file << "\n[Keybinds]\n";
	for (const auto& kv : Keybinds.GameBinds)
	{
		// game binds are action - key

		KString action = KKeyNames::CorrectActionCase(kv.first);

		file << "+" << action << " { ";
		for (i32 i = 0; i < kv.second.size(); i++)
		{
			file << kv.second[i];
			if (i < kv.second.size() - 1)
				file << ", ";
		}
		file << " }\n";
	}

	file << "\n";
	for (const auto& kv : Keybinds.CustomBinds)
	{
		// custom binds are key - action
		file << "\"" << kv.first << "\" \"";
		file << kv.second;
		file << "\"\n";
	}
}

void KUserConfig::Load(const KString& profile)
{	
	std::lock_guard<std::mutex> lock(ConfigMutex);

	KInputProcessor::InitDefaultGameBinds(Keybinds.GameBinds);
	Keybinds.CustomBinds.clear();
	Net.FavoriteServers.clear();

	std::ifstream file(CFG_DIR + (profile == KString("") ? "config" : profile.Get()) + ".cfg");

	// if no config exists, just write default monitor settings
	MonitorSpecs specs = GetMonitorSpecs();
	if (!file.is_open())
	{
		Video.MaxFramerate = specs.Refresh;
		Video.ResX = specs.X;
		Video.ResY = specs.Y;
		KInputProcessor::UpdateGameBindsFromConfig(this);
		Save(profile);
		return;
	}

	KUserConfig defaults;

	std::string l;
	while (std::getline(file, l))
	{
		KString line(l);
		TVector<KString> entry;
		line.SplitByChar(entry, ' ', 1);

		if (entry.size() == 2)
		{
			KString key = entry[0].ToLower().Trim();
			KString value = entry[1].ToLower().Trim();
			
			if (key == "maxfps")
			{
				u16 val = specs.Refresh;
				value.ToU16Safe(val);
				Video.MaxFramerate = val;
			}
			if (key == "interpolate")
			{
				u8 val = defaults.Video.bInterpolate;
				value.ToU8Safe(val);
				Video.bInterpolate = val;
			}
			if (key == "fullscreen")
			{
				u8 val = defaults.Video.WindowState == EWindowState::Fullscreen ? 1 : 0;
				value.ToU8Safe(val);
				Video.WindowState = val ? EWindowState::Fullscreen : EWindowState::Windowed;
			}
			else if (key == "verticalfov")
			{
				f32 val = defaults.Video.VFov;
				value.ToFloatSafe(val);
				Video.VFov = val;
			}
			else if (key == "brightness")
			{
				f32 val = defaults.Video.Brightness;
				value.ToFloatSafe(val);
				Video.Brightness = val;
			}
			else if (key == "contrast")
			{
				f32 val = defaults.Video.Contrast;
				value.ToFloatSafe(val);
				Video.Contrast = val;
			}
			else if (key == "saturation")
			{
				f32 val = defaults.Video.Saturation;
				value.ToFloatSafe(val);
				Video.Saturation = val;
			}
			else if (key == "flip")
			{
				u32 val = defaults.Video.Flip;
				value.ToU32Safe(val);
				Video.Flip = val;
			}
			else if (key == "resx")
			{
				f32 val = defaults.Video.ResX;
				value.ToFloatSafe(val);
				if (val < 1) val = 1;
				Video.ResX = val;
			}
			else if (key == "resy")
			{
				f32 val = defaults.Video.ResY;
				value.ToFloatSafe(val);
				if (val < 1) val = 1;
				Video.ResY = val;
			}
			else if (key == "renderscale")
			{
				f32 val = defaults.Video.RenderScale;
				value.ToFloatSafe(val);
				Video.RenderScale = val;
			}
			else if (key == "downscalefilter")
			{
				u8 val = defaults.Video.bFilterRenderScale;
				value.ToU8Safe(val);
				Video.bFilterRenderScale = val;
			}
			else if (key == "scalemetric")
			{
				u8 val = defaults.Video.RenderScaleMetric;
				value.ToU8Safe(val);
				Video.RenderScaleMetric = val;
			}
			else if (key == "inverty")
			{
				u8 val = defaults.Mouse.bInvertY;
				value.ToU8Safe(val);
				Mouse.bInvertY = val;
			}
			else if (key == "invertx")
			{
				u8 val = defaults.Mouse.bInvertX;
				value.ToU8Safe(val);
				Mouse.bInvertX = val;
			}
			else if (key == "separatemousey")
			{
				u8 val = defaults.Mouse.bSeparateMouseAxes;
				value.ToU8Safe(val);
				Mouse.bSeparateMouseAxes = val;
			}
			else if (key == "sensitivity")
			{
				f32 val = defaults.Mouse.Horizontal.Sensitivity;
				value.ToFloatSafe(val);
				Mouse.Horizontal.Sensitivity = val;
			}
			else if (key == "usedistance")
			{
				u8 val = defaults.Mouse.Horizontal.bUseDistance;
				value.ToU8Safe(val);
				Mouse.Horizontal.bUseDistance = val;
			}
			else if (key == "useinches")
			{
				u8 val = defaults.Mouse.Horizontal.bUseInches;
				value.ToU8Safe(val);
				Mouse.Horizontal.bUseInches = val;
			}
			else if (key == "spindegrees")
			{
				f32 val = defaults.Mouse.Horizontal.SpinDegrees;
				value.ToFloatSafe(val);
				Mouse.Horizontal.SpinDegrees = val;
			}
			else if (key == "unitsperspin")
			{
				f32 val = defaults.Mouse.Horizontal.UnitsPerSpin;
				value.ToFloatSafe(val);
				Mouse.Horizontal.UnitsPerSpin = val;
			}
			else if (key == "mousedpi")
			{
				f32 val = defaults.Mouse.Horizontal.MouseDPI;
				value.ToFloatSafe(val);
				Mouse.Horizontal.MouseDPI = val;
			}
			else if (key == "y_sensitivity")
			{
				f32 val = defaults.Mouse.Vertical.Sensitivity;
				value.ToFloatSafe(val);
				Mouse.Vertical.	Sensitivity = val;
			}
			else if (key == "y_usedistance")
			{
				u8 val = defaults.Mouse.Vertical.bUseDistance;
				value.ToU8Safe(val);
				Mouse.Vertical.bUseDistance = val;
			}
			else if (key == "y_useinches")
			{
				u8 val = defaults.Mouse.Vertical.bUseInches;
				value.ToU8Safe(val);
				Mouse.Vertical.bUseInches = val;
			}
			else if (key == "y_spindegrees")
			{
				f32 val = defaults.Mouse.Vertical.SpinDegrees;
				value.ToFloatSafe(val);
				Mouse.Vertical.SpinDegrees = val;
			}
			else if (key == "y_unitsperspin")
			{
				f32 val = defaults.Mouse.Vertical.UnitsPerSpin;
				value.ToFloatSafe(val);
				Mouse.Vertical.UnitsPerSpin = val;
			}
			else if (key == "y_mousedpi")
			{
				f32 val = defaults.Mouse.Vertical.MouseDPI;
				value.ToFloatSafe(val);
				Mouse.Vertical.MouseDPI = val;
			}
			else if (key == "bobscale")
			{
				f32 val = defaults.Game.BobScale;
				value.ToFloatSafe(val);
				Game.BobScale = val;
			}
			else if (key == "shakescale")
			{
				f32 val = defaults.Game.ShakeScale;
				value.ToFloatSafe(val);
				Game.ShakeScale = val;
			}
			else if (key == "concussionscale")
			{
				f32 val = defaults.Game.ConcussionScale;
				value.ToFloatSafe(val);
				Game.ConcussionScale = val;
			}
			else if (key == "breathrange")
			{
				f32 val = defaults.Game.BreathRange;
				value.ToFloatSafe(val);
				Game.BreathRange = val;
			}
			else if (key == "rollscale")
			{
				f32 val = defaults.Game.RollScale;
				value.ToFloatSafe(val);
				Game.RollScale = val;
			}
			else if (key == "weaponflash")
			{
				u8 val = defaults.Game.bWeaponFlash;
				value.ToU8Safe(val);
				Game.bWeaponFlash = val;
			}
			else if (key == "gore")
			{
				u8 val = defaults.Game.bGore;
				value.ToU8Safe(val);
				Game.bGore = val;
			}
			else if (key == "consolepause")
			{
				u8 val = defaults.Game.bConsolePause;
				value.ToU8Safe(val);
				Game.bConsolePause = val;
			}
			else if (key == "lastconnectedip")
			{
				Net.LastConnectedIP = value.Trim();
			}
			else if (key == "playername")
			{
				Net.PlayerName = entry[1].Trim();
			}
			else if (key == "mastervolume")
			{
				f32 val = defaults.Audio.MasterVolume;
				value.ToFloatSafe(val);
				Audio.MasterVolume = val;
			}
			else if (key == "framepad")
			{
				u32 val = defaults.Net.FramePad;
				value.ToU32Safe(val);
				Net.FramePad = val;
			}
			else if (key == "snapshotbuffer")
			{
				u32 val = defaults.Net.SnapshotBuffer;
				value.ToU32Safe(val);
				Net.SnapshotBuffer = val;
			}
			else if (key[0] == '+')
			{
				// keybind
				// "key" here is the action
				key.GetMut().erase(0, 1);
				TVector<KString> inputs;
				value.SplitByChar(inputs, ',');
				Keybinds.GameBinds[key].clear();
				for (KString& input : inputs)
				{
					input.ReplaceCharInline('{', ' ');
					input.ReplaceCharInline('}', ' ');
					input.TrimInPlace();

					if (KKeyNames::GetKeyFromName(input) != -1)
						Keybinds.GameBinds[key].push_back(input);
				}
			}
			else if (key[0] == '\"')
			{
				// custom keybind
				// make sure these start and end with quotes
				if (key.Size() < 3) continue;
				if (value.Size() < 3) continue;
				if (key[0] != '\"' || key[key.Size() - 1] != '\"') continue;
				if (value[0] != '\"' || value[value.Size() - 1] != '\"') continue;

				key = key.GetSubstring(1, key.Size() - 2);
				value = value.GetSubstring(1, value.Size() - 2);

				Keybinds.CustomBinds[key] = value;
			}
			else if (key[0] == '@')
			{
				// favorite server
				key.GetMut().erase(0, 1);

				// check if ip is valid
				KNetAddress addr;
				if (!addr.SetFromString(key))
					continue;

				if (value.IsEmpty())
					continue;

				if (value.Trim() == "local")
					continue;

				Net.FavoriteServers[value] = key;
			}
		}
	}
	Video.HFov = Fov_V_to_H(Video.VFov, Video.GetResX() / Video.GetResY());
	KGameInstance::Get().bWeaponFlash = Game.bWeaponFlash;
	KGameInstance::Get().bConsolePause = Game.bConsolePause;
	KGameInstance::Get().MasterVolume = Audio.MasterVolume;
	KInputProcessor::UpdateCustomBindsFromConfig(this);
	KInputProcessor::UpdateGameBindsFromConfig(this);
	Save(profile);
}

void KUserConfig::SetLastConnectedIP(const KString& ip)
{
	Net.LastConnectedIP = ip.Trim();
	Save(GetActiveConfig());
}

KUserConfig::MonitorSpecs KUserConfig::GetMonitorSpecs() const
{
	MonitorSpecs specs;
	
#if _WIN32
	DEVMODEA devmode;
	EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	specs.Refresh = devmode.dmDisplayFrequency;

	specs.X = GetSystemMetrics(SM_CXSCREEN);
	specs.Y = GetSystemMetrics(SM_CYSCREEN);
#endif

	return specs;
}

KUserConfig* GetUserConfig()
{
	return GetLocalPlayer()->GetConfig();
}

void LockConfig()
{
	GetUserConfig()->ConfigMutex.lock();
}

void UnlockConfig()
{
	GetUserConfig()->ConfigMutex.unlock();
}

#endif
