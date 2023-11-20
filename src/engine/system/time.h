#pragma once

#include "engine/global/types_numeric.h"
#include <chrono>

typedef std::chrono::high_resolution_clock				KTimeClock;
typedef std::chrono::high_resolution_clock::time_point	KTimePoint;

class KTime
{
	static KTimePoint AppInit;

public:

	static KTimePoint Now() { return KTimeClock::now(); }
	static KTimePoint Init() { return AppInit; }

	static f64 Between(KTimePoint start, KTimePoint end)
	{
		return std::chrono::duration_cast<std::chrono::duration<f64>>(end - start).count();
	}

	static f64 Since(KTimePoint point)
	{
		return Between(point, Now());
	}

	static f64 SinceInit()
	{
		return Since(AppInit);
	}

	static u64 NanoSecondsBetween(KTimePoint start, KTimePoint end)
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	}

	static u64 NanoSecondsSince(KTimePoint time)
	{
		return NanoSecondsBetween(time, Now());
	}

	static u64 MicroSecondsBetween(KTimePoint start, KTimePoint end)
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	}

	static u64 MicroSecondsSince(KTimePoint time)
	{
		return MicroSecondsBetween(time, Now());
	}

	static void SleepThreadMicroseconds(u64 microseconds);

	static f64 FrameNow();
	static u32 FramesFromTime(f64 time);
	static u32 FramesSince(u32 frame);
	static u32 FrameCount();
};