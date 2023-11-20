#if !_SERVER

#include "input_processor.h"
#include "engine/os/windows/windows.h"
#include "glfw_input.h"
#include "GLFW/glfw3.h"
#include "view.h"
#include "listener.h"
#include "../console/engine_console.h"
#include "engine/input/listener_console.h"
#include "key_names.h"
#include "../game/config.h"

KDelegateBroadcasterArgs<EInputAction> KInputProcessor::EventBroadcaster;
KDelegateBroadcasterArgs<f64, f64> KInputProcessor::MouseMoveBroadcaster;
KDelegateBroadcasterArgs<u8> KInputProcessor::KeyStateBroadcaster;
std::atomic<bool> KInputProcessor::bUseMousePosition = { false };
bool KInputProcessor::bInitialized = false;
std::mutex KInputProcessor::StackMutex;
TVector<class KInputListener*> KInputProcessor::InputListenerStack;
std::mutex KInputProcessor::CursorMutex;
f32 KInputProcessor::MouseX;
f32 KInputProcessor::MouseY;
bool KInputProcessor::bSkipCharEvent = false;
bool KInputProcessor::bForceStateBroadcast;
u8 KInputProcessor::LastKeyState = 0;
bool KInputProcessor::bLastScoreboardState = false;


TMap<i32, EInputAction> KInputProcessor::KeyMappings;
TMap<i32, KString> KInputProcessor::CustomBinds;

TMap<EInputAction, TVector<i32>> KInputProcessor::ReverseKeyMappings;

const TMap<i32, EInputAction> DefaultKeyMappings =
{
	{ GLFW_KEY_W,				EInputAction::MoveForward },
	{ GLFW_KEY_S,				EInputAction::MoveBack },
	{ GLFW_KEY_A,				EInputAction::MoveLeft },
	{ GLFW_KEY_D,				EInputAction::MoveRight },
	{ GLFW_KEY_SPACE,			EInputAction::Jump },
	{ GLFW_KEY_C,				EInputAction::Crouch },
	{ GLFW_KEY_LEFT_CONTROL,	EInputAction::Crouch },
	{ GLFW_MOUSE_BUTTON_LEFT,	EInputAction::Fire },
	{ GLFW_MOUSE_BUTTON_RIGHT,	EInputAction::AltFire },

	{ KINPUT_SCROLL_UP,			EInputAction::NextWeapon },
	{ KINPUT_SCROLL_DOWN,		EInputAction::PrevWeapon },
	{ GLFW_KEY_1,				EInputAction::Weapon0 },
	{ GLFW_KEY_2,				EInputAction::Weapon1 },
	{ GLFW_KEY_3,				EInputAction::Weapon2 },
	{ GLFW_KEY_4,				EInputAction::Weapon3 },
	{ GLFW_KEY_5,				EInputAction::Weapon4 },
	{ GLFW_KEY_6,				EInputAction::Weapon5 },
	{ GLFW_KEY_7,				EInputAction::Weapon6 },
	{ GLFW_KEY_8,				EInputAction::Weapon7 },

	{ GLFW_KEY_GRAVE_ACCENT,	EInputAction::ShowConsole },
	{ GLFW_KEY_F1,				EInputAction::ShowConsole },
	{ GLFW_KEY_SLASH,			EInputAction::ShowConsole },
	{ GLFW_KEY_F12,				EInputAction::Pause },
	{ GLFW_KEY_ESCAPE,			EInputAction::Pause },
	{ GLFW_KEY_T,				EInputAction::OpenChat },
	{ GLFW_KEY_ENTER,			EInputAction::OpenChat },
	{ GLFW_KEY_KP_ENTER,		EInputAction::OpenChat },
	{ GLFW_KEY_TAB,				EInputAction::ShowScoreboard },

};

TMap<i32, EInputAction> InterfaceControls =
{
	{ GLFW_KEY_UP,				EInputAction::UpArrow },
	{ GLFW_KEY_DOWN,			EInputAction::DownArrow },
	{ GLFW_KEY_RIGHT,			EInputAction::RightArrow },
	{ GLFW_KEY_LEFT,			EInputAction::LeftArrow },
	{ GLFW_MOUSE_BUTTON_LEFT,	EInputAction::LeftClick },
	{ GLFW_KEY_ENTER,			EInputAction::Submit },
	{ KINPUT_SCROLL_UP,			EInputAction::UpArrow },
	{ KINPUT_SCROLL_DOWN,		EInputAction::DownArrow },
};

void KInputProcessor::InitInput()
{
	KGLFWInput::InitGlfwInput();
	BuildReverseKeyMappings();
	LOG("Input Initialized");
}

bool KInputProcessor::WaitInput()
{
	bool ret = KGLFWInput::WaitGlfwEvents();
	bSkipCharEvent = false;
	return ret;
}

KInputKeyEvent KInputProcessor::CreateKeyEvent(i32 input, i32 mods)
{
	EInputAction evnt = EInputAction::INVALID;

	bool iface = false;
	if (!GetActiveListener() || GetActiveListener()->UsesInterfaceControls())
	{
		if (InterfaceControls.count(input) > 0)
		{
			evnt = InterfaceControls[input];
			iface = true;
		}
	}

	if (!iface && KeyMappings.count(input) > 0)
		evnt = KeyMappings[input];

	return KInputKeyEvent(evnt, input, mods);
}

KInputMouseEvent KInputProcessor::CreateMouseEvent(f64 x, f64 y)
{
	KInputMouseEvent evnt(x, y);
	return evnt;
}

void KInputProcessor::ProcessKeyInput(i32 input, i32 mods)
{
	KInputListener* l = GetActiveListener();

	if (l && !l->UsesInterfaceControls() && CustomBinds.contains(input))
	{
		GetEngineConsole()->bPrintBack = false;
		GetEngineConsole()->ProcessSubmission(CustomBinds[input]);
		GetEngineConsole()->bPrintBack = true;
	}

	KInputKeyEvent evnt = CreateKeyEvent(input, mods);
	if (KInputListener* l = GetListener()) l->ProcessKey(evnt);
}

void KInputProcessor::ProcessCharInput(u32 input)
{
	if (!bSkipCharEvent)
		if (KInputListener* l = GetListener()) l->ProcessCharacter(input);
}

void KInputProcessor::ProcessMouseInput(f64 x, f64 y, EMouseMoveType type)
{
	if (GetActiveListener()->MouseControlsCamera() && type == EMouseMoveType::Move)
	{
		// update the camera directly
		KInputView::UpdateFromInput(x, y);
	}
	else
	{
		std::lock_guard<std::mutex> lock(CursorMutex);
		MouseX = x;
		MouseY = y;
		for (KInputListener* l : InputListenerStack)
		  if (!l->MouseControlsCamera())
			l->OnCursorMoved(x, y, type);
	}
}

void KInputProcessor::ProcessKeyState(u8 state, bool scoreboard)
{
	if (LastKeyState != state || bForceStateBroadcast)
	{
		LastKeyState = state;
		if (KInputListener* l = GetListener()) l->ProcessState(state);
	}
	if (bLastScoreboardState != scoreboard || bForceStateBroadcast)
	{
		bLastScoreboardState = scoreboard;
		if (KInputListener* l = GetListener()) l->ProcessScoreboard(scoreboard);
	}
	bForceStateBroadcast = false;
}

bool KInputProcessor::IsActionPressed(EInputAction action, struct GLFWwindow* win)
{
	for (i32 i : ReverseKeyMappings[action])
	{
		if (i < GLFW_MOUSE_BUTTON_LAST)
		{
			if (glfwGetMouseButton(win, i) == GLFW_PRESS)
				return true;
			continue;
		}
		
		if (glfwGetKey(win, i) == GLFW_PRESS)
			return true;
	}

	return false;
}

class KInputListener* KInputProcessor::GetListener()
{
	std::lock_guard<std::mutex> lock(StackMutex);
	const u32 size = InputListenerStack.size();
	if (size > 0) return InputListenerStack[size - 1];
	return nullptr;
}

void KInputProcessor::SetListener(class KInputListener* listener)
{
	std::lock_guard<std::mutex> lock(StackMutex);
	u32 size = InputListenerStack.size();
	
	// if console listener is active, it is always on top
	if (size > 0 && InputListenerStack[size - 1] == GetEngineConsole()->GetListener())
	{
		InputListenerStack.insert(InputListenerStack.begin() + (size - 1), listener);
	}
 	else
	{
		if (size > 0) InputListenerStack[size - 1]->OnUnset();
		InputListenerStack.push_back(listener);
		listener->OnSet();
	}

	bSkipCharEvent = true;
}

void KInputProcessor::PopListenerStack()
{
	std::lock_guard<std::mutex> lock(StackMutex);

	u32 size = InputListenerStack.size();
	if (size > 0) 
	{
		InputListenerStack[size - 1]->OnUnset();
		InputListenerStack[size - 1]->OnRemoved();
	}
		
	InputListenerStack.pop_back();

	size = InputListenerStack.size();
	if (size > 0) InputListenerStack[size - 1]->OnSet();

	bSkipCharEvent = true;
}

void KInputProcessor::PushListenerStack(class KInputListener* listener)
{
	SetListener(listener);
}

void KInputProcessor::RemoveListenerFromStack(class KInputListener* listener)
{
	if (!InputStackContains(listener)) return;

	KInputListener* active = GetActiveListener();
	bool isLast = active == listener;
	std::lock_guard<std::mutex> lock(StackMutex);

	VectorRemove(InputListenerStack, listener);
	if (active == listener) listener->OnUnset();
	listener->OnRemoved();

	// need to "set" a new listener if active one is being removed
	u32 size = InputListenerStack.size();
	if (size > 0 && isLast) InputListenerStack[size - 1]->OnSet();
}

bool KInputProcessor::InputStackContains(class KInputListener* listener)
{
	std::lock_guard<std::mutex> lock(StackMutex);
	return VectorContains(InputListenerStack, listener);
}

KInputListener* KInputProcessor::GetActiveListener()
{
	std::lock_guard<std::mutex> lock(StackMutex);
	if (InputListenerStack.size() == 0) return nullptr;
	return InputListenerStack[InputListenerStack.size() - 1];
}

void KInputProcessor::ShowMouseCursor()
{
	KGLFWInput::ShowMouseCursor();
}

void KInputProcessor::HideMouseCursor()
{
	KGLFWInput::HideMouseCursor();
}

void KInputProcessor::CenterMouseCursor()
{
	KGLFWInput::CenterMouseCursor();
}

bool KInputProcessor::MouseCursorIsActive()
{
	return bUseMousePosition.load();
}

bool KInputProcessor::MouseIsShowing()
{
	return KGLFWInput::MouseIsShowing();
}

bool KInputProcessor::LeftMouseDown()
{
	return KGLFWInput::LeftMousePressed();
}

void KInputProcessor::GetCursorPos(f32& x, f32& y)
{
	std::lock_guard<std::mutex> lock(CursorMutex);
	x = MouseX;
	y = MouseY;
}

void KInputProcessor::UpdateGameBindsFromConfig(class KUserConfig* cfg)
{
	for (const auto& kv : cfg->Keybinds.GameBinds)
	{
		const KString& actionName = kv.first;
		const TVector<KString>& keyNames = kv.second;
		u8 action = KKeyNames::GetActionFromName(actionName);

		for (const KString& name : keyNames)
		{
			i32 key = KKeyNames::GetKeyFromName(name);
			KeyMappings[key] = (EInputAction)action;
		}
	}

	BuildReverseKeyMappings();
}

void KInputProcessor::UpdateCustomBindsFromConfig(class KUserConfig* cfg)
{
	CustomBinds.clear();

	for (const auto& kv : cfg->Keybinds.CustomBinds)
	{
		i32 key = KKeyNames::GetKeyFromName(kv.first);
		if (key != -1) CustomBinds[key] = kv.second;
	}
}

void KInputProcessor::InitDefaultGameBinds(TMap<KString, TVector<KString>>& binds)
{
	binds.clear();

	for (const auto& kv : DefaultKeyMappings)
		binds[KKeyNames::GetActionName(kv.second).ToLower()].push_back(KKeyNames::GetKeyName(kv.first).ToLower());
}

void KInputProcessor::BuildReverseKeyMappings()
{
	ReverseKeyMappings.clear();

	for (const auto& kv : KeyMappings)
		ReverseKeyMappings[kv.second].push_back(kv.first);
}

#endif