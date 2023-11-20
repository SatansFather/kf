#pragma once

#include "kfglobal.h"

#define PING_BUFFER_SIZE 10

class KPlayerPing
{
	f64 PingBuffer[PING_BUFFER_SIZE];
	u8 CurrentIndex = 0;
	u8 BufferSize = 0;

	void Reset();

public:

	KPlayerPing();
	void AddPingEntry(f64 ping);
	u8 GetDividedPing() const;
	f64 GetPing() const;
};