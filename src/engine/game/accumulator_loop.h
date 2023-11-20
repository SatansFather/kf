#pragma once

#include "kfglobal.h"
#include "engine/system/time.h"

#define MICROSECOND_TIMESTEP (GameFrameDelta() * 1000000)//16667

class KAccumulatorLoop
{
	
	friend class KGameMatch;
	friend class KGameInstance;

private:
	
	bool bStarted = false;
	const u64 Timestep = MICROSECOND_TIMESTEP;
	u64 Accumulator = 0;
	KTimePoint LastFrameConsumeTime;
	KTimePoint LastAccumulationTime;
	KTimePoint LastElapsedCheckTime;
	u32 TotalFrames = 0;

public:
	
	KAccumulatorLoop();

	u64 GetTimestep() const { return Timestep; }
	f64 GetFloatTimestep() const { return GameFrameDelta(); }
	GFlt GetAlpha() { return (GFlt)Accumulator / (GFlt)Timestep; }
	u32 GetTotalFrames() const { return TotalFrames; }
	//bool IsPaused() const { return PauseHandles > 0; }
	bool IsStarted() const { return bStarted; }

private:

	void AccumulateTime();
	bool CanRunFrame();
	bool ConsumeFrame();
	void StartAccumulating();
	//void AddPause();
	//void RemovePause();
};