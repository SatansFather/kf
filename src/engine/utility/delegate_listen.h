#pragma once

#include "delegate_broadcast.h"

// must be called from the object that owns the listener - "this" is used
#define BIND_DELEGATE(func, listener, broadcaster)	{ listener.Bind(std::bind_front(&func, this), broadcaster, false); }
#define BIND_DELEGATE_THREADSAFE(func, listener, broadcaster)	{ listener.Bind(std::bind_front(&func, this), broadcaster, true); }

class KDelegateListener
{
	KDelegateListener(const KDelegateListener&) = delete;
	KDelegateListener& operator=(const KDelegateListener&) = delete;

private:

	SPtr<KDelegateBroadcaster::DelegateListener> Listener;

public:

	KDelegateListener()
	{
		Listener = std::make_shared<KDelegateBroadcaster::DelegateListener>();
	}

	void Bind(std::function<void()> func, KDelegateBroadcaster& broadcaster, bool threadsafe)
	{
		Listener->Bind(func, broadcaster, Listener, threadsafe);
	}

	void Lock() { Listener->Lock(); }
	void Unlock() { Listener->Unlock(); }
};

template <typename... TTypes>
class KDelegateListenerArgs
{
	KDelegateListenerArgs<TTypes...>(const KDelegateListenerArgs<TTypes...>&) = delete;
	KDelegateListenerArgs<TTypes...>& operator=(const KDelegateListenerArgs<TTypes...>&) = delete;

private:

	SPtr<typename KDelegateBroadcasterArgs<TTypes...>::DelegateListener> Listener;

public:

	KDelegateListenerArgs()
	{
		Listener = std::make_shared<typename KDelegateBroadcasterArgs<TTypes...>::DelegateListener>();
	}

	void Bind(std::function<void(TTypes...)> func, KDelegateBroadcasterArgs<TTypes...>& broadcaster, bool threadsafe)
	{
		Listener->Bind(func, broadcaster, Listener, threadsafe);
	}

	void Lock() { Listener->Lock(); }
	void Unlock() { Listener->Unlock(); }
};