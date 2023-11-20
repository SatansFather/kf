#pragma once

#include <functional>
#include <mutex>
#include "engine/global/types_numeric.h"
#include "engine/global/types_container.h"
#include <atomic>
#include <tuple>
#include "engine/run/k_main.h"
#include <thread>

class KThreadPool
{
private:

	class KThreadJobBase
	{
	public:
		virtual void Execute() = 0;
	};

	template <typename Functor, typename... TArgs>
	class TThreadJob : public KThreadJobBase
	{
		friend class KThreadPool;

		Functor Function;
		std::tuple<TArgs...> Args;

		TThreadJob(Functor func, TArgs... args) : Function(func)
		{
			Args = std::make_tuple(args...);
		}

		virtual void Execute() override
		{
			std::apply(Function, Args);
			delete this;
		}
	};

	u16 ThreadCount = 0;
	std::atomic<u16> ActiveJobCount { 0 };
	std::atomic<bool> bDestroyingPool { false };
	TVector<std::thread> Threads;
	std::mutex QueueMutex;
	TVector<KThreadJobBase*> PendingJobs;
	f64 LockWaitTime = 0;

public:

	KThreadPool(u16 count);
	~KThreadPool();
	
private:
	void AddJobToPool(KThreadJobBase* job);
public:

	template <typename Functor, typename... TArgs>
	void AddToPool(Functor func, TArgs... args);

	bool ReadFromPool();
	inline bool IsRunning() const { return PendingJobs.size() > 0; }
	inline bool IsPendingDestroy() { return bDestroyingPool; }
	u16 GetPendingJobCount();

	void WaitForFinish();

	template <typename Functor>
	static void Iterate(Functor func, u16 pool_size, u32 container_size);
};

template <typename Functor, typename... TArgs>
void KThreadPool::AddToPool(Functor func, TArgs... args)
{
	AddJobToPool(new TThreadJob(func, std::forward<TArgs>(args)...));
}

template <typename Functor>
void KThreadPool::Iterate(Functor func, u16 pool_size, u32 container_size)
{
	if (pool_size == 0) pool_size = KApplication::GetCoreCount();
	if (pool_size > container_size) pool_size = container_size;

	KThreadPool pool(pool_size);
	f32 step = 1.f / pool_size;
	for (u16 i = 0; i < pool_size; i++)
	{
		u32 start = container_size * (step * i);
		u32 end = container_size * (step * (i + 1));
		pool.AddToPool(func, start, end);
	}
}
