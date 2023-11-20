#pragma once


#include "event.h"
#include "engine/global/types_container.h"
#include "engine/utility/delegate_broadcast.h"

// GLFW does not define mouse scroll
// rather it sends doubles for offsets
#define KINPUT_SCROLL_UP	1111111
#define KINPUT_SCROLL_DOWN	1111112

// processes live user input
class KInputProcessor
{
#if !_SERVER
private:

	// key-to-action map
	static TMap<i32, EInputAction> KeyMappings;

	static TMap<i32, KString> CustomBinds;

	// backwards KeyMappings so we can check a key from an action
	// to be updated whenever KeyMappings is updated
	static TMap<EInputAction, TVector<i32>> ReverseKeyMappings;


	static u8 LastKeyState;
	static bool bLastScoreboardState;

	//static class KInputListener* CurrentListener;

	static KInputKeyEvent CreateKeyEvent(i32 input, i32 mods);
	static KInputMouseEvent CreateMouseEvent(f64 x, f64 y);

	static bool bInitialized;

	static std::mutex StackMutex;
	static TVector<class KInputListener*> InputListenerStack;

	static std::mutex CursorMutex;
	static f32 MouseX;
	static f32 MouseY;

	static bool bSkipCharEvent;

public:

	static KDelegateBroadcasterArgs<EInputAction> EventBroadcaster;
	static KDelegateBroadcasterArgs<f64, f64> MouseMoveBroadcaster;
	static KDelegateBroadcasterArgs<u8> KeyStateBroadcaster;
	
	static std::atomic<bool> bUseMousePosition;

	// forces a state broadcast next poll, whether it changed or not
	// new broadcasters will flag this true so they can receive the most recent state
	static bool bForceStateBroadcast;

	// broadcast binds a function to this's broadcast
	// demo player will call that function on the demo broadcaster (who does not subscribe to this)

public:
	
	static void InitInput();
	static bool WaitInput();

	static void ProcessKeyInput(i32 input, i32 mods);
	static void ProcessCharInput(u32 input);
	static void ProcessMouseInput(f64 x, f64 y, EMouseMoveType type);
	static void ProcessKeyState(u8 state, bool scoreboard);

	static EInputAction GetKeyMapping(i32 key) { return KeyMappings[key]; }
	static bool IsActionPressed(EInputAction action, struct GLFWwindow* win);

	static class KInputListener* GetListener();// { return CurrentListener; }
	static void SetListener(class KInputListener* listener);// { CurrentListener = listener; }

	static void PopListenerStack();
	static void PushListenerStack(class KInputListener* listener);
	static void RemoveListenerFromStack(class KInputListener* listener);
	static bool InputStackContains(class KInputListener* listener);
	static KInputListener* GetActiveListener();

	static void ShowMouseCursor();
	static void HideMouseCursor();
	static void CenterMouseCursor();
	static bool MouseCursorIsActive();
	static bool MouseIsShowing();
	static bool LeftMouseDown();
	static void GetCursorPos(f32& x, f32& y);
	
	static void UpdateGameBindsFromConfig(class KUserConfig* cfg);

	static void UpdateCustomBindsFromConfig(class KUserConfig* cfg);

	static void InitDefaultGameBinds(TMap<KString, TVector<KString>>& binds);

private:

	static void BuildReverseKeyMappings();
#endif
};
