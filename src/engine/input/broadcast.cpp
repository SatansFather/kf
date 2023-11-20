#if !_SERVER

#include "broadcast.h"
#include "input_processor.h"

KInputBroadcaster::KInputBroadcaster(bool accept_mouse, bool accept_events, bool accept_state)
{
	if (accept_mouse)
	{
		BIND_DELEGATE(KInputBroadcaster::BroadcastMouseMove, 
			MouseMoveListener, KInputProcessor::MouseMoveBroadcaster);
	}

	if (accept_events)
	{
		BIND_DELEGATE(KInputBroadcaster::BroadcastEvent,
			EventListener, KInputProcessor::EventBroadcaster);
	}

	if (accept_state)
	{
		BIND_DELEGATE(KInputBroadcaster::BroadcastState,
			KeyStateListener, KInputProcessor::KeyStateBroadcaster);

		KInputProcessor::bForceStateBroadcast = true;
	}
}

void KInputBroadcaster::AddAcceptedInput(EInputAction bind)
{
	AcceptedInput.insert(bind);
}

void KInputBroadcaster::RemoveAcceptedInput(EInputAction bind)
{
	AcceptedInput.erase(bind);
}

void KInputBroadcaster::BroadcastEvent(EInputAction evnt)
{
	// need to check because we could have bound this but not want this specific input
	if (AcceptedInput.contains(evnt))
	{
		EventBroadcaster.Broadcast(evnt);
	}	
}

void KInputBroadcaster::BroadcastMouseMove(f64 x, f64 y)
{
	MouseMoveBroadcaster.Broadcast(x, y);	
}

void KInputBroadcaster::BroadcastState(u8 state)
{
	KeyStateBroadcaster.Broadcast(state);
}

#endif