#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "engine/utility/kstring.h"
#include "engine/render/draw_config.h"
#include <mutex>

class KUserConfig
{
public:
	
	std::mutex ConfigMutex;

	KDrawConfig Video;
	
	struct
	{
		struct Axis
		{
			f32 Sensitivity = 1;
			u8 bUseDistance = false;
			f32 SpinDegrees = 360;
			f32 MouseDPI = 1000;
			u8 bUseInches = true;
			f32 UnitsPerSpin = 6;
			
			f32 GetSensitivity()
			{
				if (bUseDistance)
					return (SpinDegrees / MouseDPI) /
					(bUseInches ? UnitsPerSpin : UnitsPerSpin * 2.54);

				const f32 scaling = .05;
				return Sensitivity * scaling;
			}
		};
		u8 bSeparateMouseAxes = false;
		Axis Horizontal, Vertical;
		u8 bInvertY = false;
		u8 bInvertX = false;
	} Mouse;

	struct 
	{
		KString LastConnectedIP;
		KString PlayerName = "Player";
		u32 FramePad = 0;
		u32 SnapshotBuffer = 1;

		//   name     ip
		TMap<KString, KString> FavoriteServers;
	} Net;

	struct 
	{
		f32 BobScale = 1;
		f32 RollScale = 1;
		f32 ShakeScale = 1;
		f32 ConcussionScale = 1;
		u8 bWeaponFlash = 1;
		f32 BreathRange = 1;
		u8 bGore = 1;
		u8 bConsolePause = 1;
	} Game;

	struct 
	{
		f32 MasterVolume = .5;
	} Audio;

	struct
	{
		TMap<KString, TVector<KString>> GameBinds;
		TMap<KString, KString> CustomBinds;
	} Keybinds;

public:

	void Save(const KString& profile, bool lock = false);
	void Load(const KString& profile);

#if !_SERVER
	KDrawConfig* GetDrawConfig() { return &Video; }
#endif

	template<typename T>
	KString UpdateValueFromConsole(T* val, const KString& arg, T min = 0, T max = 0);

	KString GetActiveConfig() { return "config"; }

	void SetLastConnectedIP(const KString& ip);

private:
	
	struct MonitorSpecs { u16 X, Y, Refresh; };
	MonitorSpecs GetMonitorSpecs() const;

};

KUserConfig* GetUserConfig();
void LockConfig();
void UnlockConfig();

template<typename T>
KString KUserConfig::UpdateValueFromConsole(T* val, const KString& arg, T min, T max)
{
	K_ASSERT(val, "passed nullptr into UpdateValueFromConsole");

	LockConfig();

	T v;
	if (!arg.ToNumSafe<T>(v) || arg.IsEmpty())
	{
		v = *val;
		UnlockConfig();
		return KString(v);
	}
	
	if (min != 0 || max != 0)
	{
		if (min > max)
		{
			if (v < min) v = min;
		}
		else
		{
			v = std::clamp(v, min, max);
		}
	}

	*val = v;
	Save(GetActiveConfig());
	UnlockConfig();

	return "";
}

#endif