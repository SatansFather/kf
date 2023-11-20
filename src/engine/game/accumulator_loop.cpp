#include "accumulator_loop.h"
#include "engine/game_instance.h"
#include "../net/net_interface.h"

// DELETE
#include "../render/communication.h"

KAccumulatorLoop::KAccumulatorLoop() {}

void KAccumulatorLoop::AccumulateTime()
{
	KGameInstance& inst = KGameInstance::Get();

	bool paused = inst.IsPaused() || inst.GameTimeDilation == 0;
	if (paused && inst.GetNetInterface()) paused = false;

	if (bStarted && !paused)
	{
		f64 dilation = inst.GetTimeDilation();

		u64 checkSplit = KTime::MicroSecondsSince(LastElapsedCheckTime) * dilation;
		LastElapsedCheckTime = KTime::Now();

		// waiting to increment reduces the compounding of sub-microsecond imprecision
		// ^^ not true anymore, but does prevent client and server from desyncing slowly (wtf)
		while (KTime::MicroSecondsSince(LastElapsedCheckTime) < 50) {}

		u64 elapsed = KTime::MicroSecondsSince(LastAccumulationTime) * dilation;

		// sub-microsecond imprecision cannot compound if we dont increment it each accumulation
		if (elapsed < (Timestep - Accumulator)) return;

		// enough time has passed and a frame will be able to run
		Accumulator += elapsed;
		LastAccumulationTime = KTime::Now();

		if (Accumulator > Timestep && checkSplit > Timestep)
		{
			// renderer will use this ratio to fine tune interpolation
			inst.PerformanceTimeDilation = f64(Timestep) / f64(Accumulator);

			// clearly having a problem, need to slow down
			if (Accumulator > Timestep * dilation)
				Accumulator = Timestep; 
		}
		else
		{
			// accumulator should be equal to timestep, cannot get here if less
			inst.PerformanceTimeDilation = 1;
		}
	}
}	

bool KAccumulatorLoop::CanRunFrame()
{
	return Accumulator >= Timestep;
}

bool KAccumulatorLoop::ConsumeFrame()
{
	if (CanRunFrame())
	{
		LastFrameConsumeTime = KTime::Now();
		Accumulator -= Timestep;
		TotalFrames++;
		return true;
	}
	return false;
}

void KAccumulatorLoop::StartAccumulating()
{
	bStarted = true;
	LastAccumulationTime = KTime::Now();
	LastFrameConsumeTime = KTime::Now();
}
