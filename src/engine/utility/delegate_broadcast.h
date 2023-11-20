#pragma once

#include <functional>
#include <mutex>
#include "kfglobal.h"


class KDelegateBroadcaster
{
	KDelegateBroadcaster(const KDelegateBroadcaster&) = delete;
	KDelegateBroadcaster& operator=(const KDelegateBroadcaster&) = delete;

public:

	class DelegateListener
	{
		friend class KDelegateBroadcaster;

	private:

		std::function<void()> Function;
		std::mutex Mutex;
		bool bThreadSafe = false;
		bool bIsBound = false;

	public:

		void Bind(std::function<void()> func,
			KDelegateBroadcaster& del,
			WPtr<DelegateListener> listener, bool threadsafe)
		{
			Function = func;
			bIsBound = true;
			bThreadSafe = threadsafe;
			del.Listeners.push_back(listener);
		}

		inline bool IsBound() { return bIsBound; }
		void Lock() { Mutex.lock(); }
		void Unlock() { Mutex.unlock(); }
	};

private:

	TVector<WPtr<DelegateListener>> Listeners;

public:
	
	KDelegateBroadcaster() = default;

	void Broadcast()
	{
		for (i32 i = i32(Listeners.size() - 1); i >= 0; i--)
		{
			if (auto listener = Listeners[i].lock())
			{
				if (listener->bThreadSafe) listener->Mutex.lock();
				listener->Function();
				if (listener->bThreadSafe) listener->Mutex.lock();
			}
			else
			{
				Listeners.erase(Listeners.begin() + i);
			}
		}
	}
};

template <typename... TTypes>
class KDelegateBroadcasterArgs
{
	KDelegateBroadcasterArgs<TTypes...>(const KDelegateBroadcasterArgs<TTypes...>&) = delete;
	KDelegateBroadcasterArgs<TTypes...>& operator=(const KDelegateBroadcasterArgs<TTypes...>&) = delete;

public:

	class DelegateListener
	{
		friend class KDelegateBroadcasterArgs<TTypes...>;	

	private:

		std::function<void(TTypes...)> Function;
		std::mutex Mutex;
		bool bThreadSafe = false;
		bool bIsBound = false;

	public:

		void Bind(std::function<void(TTypes...)> func, 
		KDelegateBroadcasterArgs<TTypes...>& del, 
		WPtr<DelegateListener> listener, bool threadsafe)
		{
			Function = func;
			bIsBound = true;
			bThreadSafe = threadsafe;
			del.Listeners.push_back(listener);
		}

		inline bool IsBound() { return bIsBound; }
		void Lock() { Mutex.lock(); }
		void Unlock() { Mutex.unlock(); }
	};

private:

	TVector<WPtr<DelegateListener>> Listeners;

public:

	KDelegateBroadcasterArgs<TTypes...>() = default;
	
	void Broadcast(TTypes... args)
	{
		for (i32 i = i32(Listeners.size() - 1); i >= 0; i--)
		{
			if (auto listener = Listeners[i].lock())
			{
				if (listener->bThreadSafe) listener->Mutex.lock();
				listener->Function(std::forward<TTypes>(args)...);
				if (listener->bThreadSafe) listener->Mutex.unlock();
			}
			else
			{
				Listeners.erase(Listeners.begin() + i);
			}
		}
	}
};
