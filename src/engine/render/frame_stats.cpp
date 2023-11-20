#if !_SERVER
#include "frame_stats.h"

void KFrameStats::ResetFrame()
{
	DrawCalls = 0;
	Triangles = 0;
}	

void KFrameStats::ResetPeriod()
{
	FramesSinceReset = 0;
	LastReset = KTime::Now();
}

void KFrameStats::UpdatePeriod()
{
	f64 timeSince = KTime::Since(LastReset);
	if (timeSince > .25)
	{
		f32 rate = f32(FramesSinceReset) / timeSince;
		FrameRate = std::round(rate);

		ResetPeriod();
	}
}

#endif
