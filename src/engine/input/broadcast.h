#pragma once

#if !_SERVER

#include "engine/global/types_container.h"
#include "engine/global/types_numeric.h"
#include "binding.h"
#include "engine/utility/delegate_listen.h"

class KInputBroadcaster
{
private:

	TSet<EInputAction> AcceptedInput;

	// receives a broadcast from the input processor
	KDelegateListenerArgs<EInputAction> EventListener;
	KDelegateListenerArgs<f64, f64> MouseMoveListener;
	KDelegateListenerArgs<u8> KeyStateListener;

public:

	// broadcasts input from the input processor after filtering it
	// objects can bind to these
	KDelegateBroadcasterArgs<EInputAction> EventBroadcaster;
	KDelegateBroadcasterArgs<f64, f64> MouseMoveBroadcaster;
	KDelegateBroadcasterArgs<u8> KeyStateBroadcaster;

public:
	
	KInputBroadcaster(bool accept_mouse = true, bool accept_events = true, bool accept_state = true);

	void AddAcceptedInput(EInputAction bind);
	void RemoveAcceptedInput(EInputAction bind);

private:

	void BroadcastEvent(EInputAction evnt);
	void BroadcastMouseMove(f64 x, f64 y);
	void BroadcastState(u8 state);

};

#endif