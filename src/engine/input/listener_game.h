#pragma once

#include "binding.h"
#include "engine/utility/delegate_listen.h"
#include "listener.h"

class KGameInputListener : public KInputListener
{
#if !_SERVER
	std::mutex UpdateMutex;

	struct GameInputData
	{
		enum EFlags
		{
			WantsMenu = 1,
			WantsConsole = 2,
			WantsChat = 4
		};

		u8 KeyState = 0;
		u8 Weapon = 0;
		u8 Flags = 0;
		bool bScoreboard = false;
		u16 Pitch = 0, Yaw = 0;
		i16 WeaponScroll = 0;
	};

	GameInputData PendingData;
	GameInputData CurrentData;

public:

	KGameInputListener();
	~KGameInputListener();

	void AcceptPendingData();

	// input read functions
	u8 GetState() const { return CurrentData.KeyState; }
	u8 GetWeapon() const { return CurrentData.Weapon; }
	u16 GetPitch() const { return CurrentData.Pitch; }
	u16 GetYaw() const { return CurrentData.Yaw; }
	i16 GetScroll() const { return CurrentData.WeaponScroll; }
	GFlt GetPitchAsFloat() const;
	GFlt GetYawAsFloat() const;
	bool WantsMenu() const { return CurrentData.Flags & GameInputData::WantsMenu; }
	bool WantsConsole() const { return CurrentData.Flags & GameInputData::WantsConsole; }
	bool WantsChat() const { return CurrentData.Flags & GameInputData::WantsChat; }
	bool WantsScoreboard() const { return CurrentData.bScoreboard; }

	void UpdatePitchYaw();

private:

	void ProcessState(u8 state) override;
	void ProcessScoreboard(bool show) override;
	void ProcessKey(class KInputKeyEvent e) override;
	void OnSet() override;
	void OnUnset() override;

	void UpdatePendingWeapon(EInputAction weapon);

	bool UsesInterfaceControls() override { return false; }

#else
public:
	void ProcessState(u8 state) override {}
	void ProcessKey(class KInputKeyEvent e) override {}
	u8 GetState() const { return 0; }
	u8 GetWeapon() const { return 0; }
#endif
};

KGameInputListener* GetGameInput();
