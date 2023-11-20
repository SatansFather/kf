#pragma once

#include "kfglobal.h"

class KGameStateSnapshot
{
	TVector<u8>* Buffer = nullptr;
	class KNetPlayer* Player = nullptr;
	u32 CompareFrame = MAX_U32;
	bool bStoringReplay = false;
	

public:

	KGameStateSnapshot() = default;
	KGameStateSnapshot(u32 compareFrame, TVector<u8>* buffer, class KNetPlayer* player = nullptr, bool replay = false);
	void PackForFrame(u32 compareFrame, TVector<u8>* buffer, class KNetPlayer* player = nullptr, bool replay = false);

private:

	void BufferSnapshotData(u8* data, u32 byteCount);

};