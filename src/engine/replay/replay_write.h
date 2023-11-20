#pragma once

#include "kfglobal.h"
#include <fstream>
#include <thread>

class KReplayWriter
{
	std::ofstream File;
	TVector<u8> Buffer;
	u32 FirstFrame = MAX_U32;
	u32 LastWriteFrame = MAX_U32;
	bool bIsWriting = false;

	std::mutex FileMutex;
	std::mutex BufferMutex;
	std::atomic<bool> bCopyInProgress = { false };

public:

	KReplayWriter();
	~KReplayWriter();

	static KReplayWriter* GetActive();
	void StartWriting();
	void CopySnapshot(u8 classID, u32 netID, class KSnapshot* prevData, class KSnapshot* data);
	void AppendBufferToFile();
	void CloseFile();
	bool IsWriting() const { return bIsWriting; }
	TVector<u8>* GetBufferPointer() { return &Buffer; }

private:

	void WaitForCopy();
	void WriteFrameNumber();

};