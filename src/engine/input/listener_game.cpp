#include "listener_game.h"
#include "engine/game_instance.h"
#if !_SERVER
#include "broadcast.h"
#include "event.h"
#include "engine/console/chat_console.h"
#include "engine/console/engine_console.h"
#include "view.h"
#include "GLFW/glfw3.h"
#include "input_processor.h"
#include "listener_menu.h"
#include "engine/menus/basic_menu.h"

#if _DEV
bool bLocalForward = false;
KString CCOM_GoForward(const KString& val)
{
	bLocalForward = !bLocalForward;
	return "";
}
#endif

KGameInputListener::KGameInputListener()
{
	bMouseControlsCamera = true;
}

KGameInputListener::~KGameInputListener() {}

void KGameInputListener::AcceptPendingData()
{
	UpdateMutex.lock();

	CurrentData = PendingData;
	UpdatePitchYaw();
	PendingData.Weapon = 0;
	PendingData.Flags = 0;
	PendingData.WeaponScroll = 0;

	if (GetDrawConfig()->Flip)
	{
		u8 state = CurrentData.KeyState;
		CurrentData.KeyState &= ~EInputKeyState::MoveRight;
		CurrentData.KeyState &= ~EInputKeyState::MoveLeft;

		u8 value = state & EInputKeyState::MoveRight;
		if (value) CurrentData.KeyState |= EInputKeyState::MoveLeft;
		
		value = state & EInputKeyState::MoveLeft;
		if (value) CurrentData.KeyState |= EInputKeyState::MoveRight;
	}
	
	UpdateMutex.unlock();
}

GFlt KGameInputListener::GetPitchAsFloat() const
{
	return KInputView::MapPitchToFloat(CurrentData.Pitch);
}

GFlt KGameInputListener::GetYawAsFloat() const
{
	return KInputView::MapYawToFloat(CurrentData.Yaw);
}

void KGameInputListener::ProcessState(u8 state)
{
	UpdateMutex.lock();
	PendingData.KeyState = state;
#if _DEV
	if (bLocalForward)
		PendingData.KeyState |= EInputKeyState::MoveForward;
#endif
	UpdateMutex.unlock();
}

void KGameInputListener::ProcessScoreboard(bool show)
{
	UpdateMutex.lock();
	PendingData.bScoreboard = show;
	UpdateMutex.unlock();
}

void KGameInputListener::ProcessKey(KInputKeyEvent e)
{
	if (e.GetEvent() == EInputAction::INVALID) return;

	u8 evnt = u8(e.GetEvent());
	if (evnt > u8(EInputAction::WEAPON_START) && evnt < u8(EInputAction::WEAPON_END))
	{
		UpdatePendingWeapon(e.GetEvent());
	}
	else if (e.GetEvent() == EInputAction::ShowConsole)
	{
		GetEngineConsole()->OpenConsole();
		PendingData.KeyState = 0;	
	}
	else if (e.GetEvent() == EInputAction::OpenChat && !(e.GetMods() & GLFW_MOD_ALT))
	{
		GetChatConsole()->OpenConsole();
		PendingData.KeyState = 0;	
	}
	else if (e.GetEvent() == EInputAction::Pause)
	{
		if (KGameInstance::Get().bIsMainMenu)
			GetMainMenu()->OpenMenu();
		else
			GetIngameMenu()->OpenMenu();
		PendingData.KeyState = 0;	
	}
}

void KGameInputListener::OnSet()
{
	KInputProcessor::bForceStateBroadcast = true;
	KInputProcessor::HideMouseCursor();
}

void KGameInputListener::OnUnset()
{
	PendingData.bScoreboard = false;
}

void KGameInputListener::UpdatePitchYaw()
{
	KInputView::GetViewForGame(CurrentData.Pitch, CurrentData.Yaw);
}

void KGameInputListener::UpdatePendingWeapon(EInputAction weapon)
{
	UpdateMutex.lock();

	if (weapon == EInputAction::NextWeapon)
		PendingData.WeaponScroll = 1;
	else if (weapon == EInputAction::PrevWeapon)
		PendingData.WeaponScroll = -1;
	else
		PendingData.Weapon = u8(weapon);
	
	UpdateMutex.unlock();
}

#endif

KGameInputListener* GetGameInput()
{
	static KGameInputListener listener;
	//return KGameInstance::Get().GetGameInputListener();
	return &listener;
}