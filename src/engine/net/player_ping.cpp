#include "player_ping.h"

#define PING_RESET_THRESHOLD 50

KPlayerPing::KPlayerPing()
{
	Reset();
}

void KPlayerPing::AddPingEntry(f64 ping)
{
	f64 avg = GetPing();
	if (abs(ping - avg) > PING_RESET_THRESHOLD)
		Reset();

	PingBuffer[CurrentIndex] = ping;
	CurrentIndex = (CurrentIndex + 1) % PING_BUFFER_SIZE;

	if (BufferSize < PING_BUFFER_SIZE)
		BufferSize++;
}

void KPlayerPing::Reset()
{
	memset(PingBuffer, 0, sizeof(f64) * PING_BUFFER_SIZE);
	CurrentIndex = 0;
	BufferSize = 0;
}

u8 KPlayerPing::GetDividedPing() const
{
	return std::clamp(GetPing() / 4.0, 0.0, 255.0);
}

f64 KPlayerPing::GetPing() const
{
	f64 total = 0;
	for (u8 i = 0; i < BufferSize; i++)
		total += PingBuffer[i];

	return total / BufferSize;
}
