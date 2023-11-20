#include "kfglobal.h"

struct KRandomNumbers
{
	static u8 RngTable[256];
	static u8 DeterministicIndex;
	static u8 NonDeterministicIndex;
};

// gets a random number from 0-255 and increments the index
u8 Random();
void ResetRandom();

// helper functions to avoid calling Random manually
f32 RandRange(f32 min, f32 max);

f32 D_RandRange(f32 min, f32 max);
u8 D_Random();
f32 RandFloat(f32 min, f32 max);

u8 RandomByIndex(u8 index);