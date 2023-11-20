#include "thread_pool.h"

void WaitForJob(KThreadPool* pool, u16 thread_index)
{
	while (pool->ReadFromPool() || !pool->IsPendingDestroy()) {}
}

KThreadPool::KThreadPool(u16 count)
{
	ThreadCount = count;
	Threads.reserve(count);
	for (u16 i = 0; i < ThreadCount; i++)
	{
		Threads.push_back(std::thread(WaitForJob, this, i));
	}
}

KThreadPool::~KThreadPool()
{
	bDestroyingPool = true; // allows threads to exit when they finish
	for (std::thread& t : Threads) t.join();
}

void KThreadPool::AddJobToPool(KThreadJobBase* job)
{
	std::lock_guard<std::mutex> lock(QueueMutex);
	PendingJobs.push_back(job);
}

bool KThreadPool::ReadFromPool()
{
	KThreadJobBase* job = nullptr;

	{
		std::lock_guard<std::mutex> lock(QueueMutex);
		if (PendingJobs.size() > 0)
		{
			// use last index so we're not constantly shifting the entire array
			// keep in mind, most recently queued will run first
			job = PendingJobs[PendingJobs.size() - 1];
			PendingJobs.erase(PendingJobs.begin() + (PendingJobs.size() - 1));
			ActiveJobCount++;
		}
	}

	if (job)
	{
		job->Execute();
		ActiveJobCount--;
		return true;
	}

	return false;
}

u16 KThreadPool::GetPendingJobCount()
{
	std::lock_guard<std::mutex> lock(QueueMutex);
	return PendingJobs.size();
}

void KThreadPool::WaitForFinish()
{
	while (GetPendingJobCount() > 0 || ActiveJobCount > 0) {}
}
