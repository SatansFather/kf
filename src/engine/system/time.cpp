#include "time.h"

#if !_COMPILER
#include "engine/game_instance.h"
#include "engine/game/match.h"
#endif

KTimePoint KTime::AppInit = KTime::Now();

void KTime::SleepThreadMicroseconds(u64 microseconds)
{
	std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

f64 KTime::FrameNow()
{
#if !_COMPILER
	if (KGameMatch* match = KGameInstance::Get().GetMatch())
	{
		u32 frames = match->GetAccumulator().GetTotalFrames();
		f64 interval = match->GetAccumulator().GetFloatTimestep();

		return f64(frames) * interval;
	}
#endif

	return 0;
}

u32 KTime::FramesFromTime(f64 time)
{
#if !_COMPILER
	if (KGameMatch* match = KGameInstance::Get().GetMatch())
	{
		f64 interval = match->GetAccumulator().GetFloatTimestep();
		return time / interval;
	}
#endif
	return 0;
}

u32 KTime::FramesSince(u32 frame)
{
	return FrameCount() - frame;
}

u32 KTime::FrameCount()
{
#if !_COMPILER
	if (KGameMatch* match = KGameInstance::Get().GetMatch())
		return match->GetAccumulator().GetTotalFrames();
#endif
	return 0;
}