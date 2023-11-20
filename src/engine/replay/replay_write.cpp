#include "replay_write.h"
#include "engine/net/snapshot.h"
#include "engine/game/match.h"
#include "engine/global/paths.h"

KReplayWriter::KReplayWriter()
{
}

KReplayWriter::~KReplayWriter()
{
	AppendBufferToFile();
	WaitForCopy();
	CloseFile();
}

KReplayWriter* KReplayWriter::GetActive()
{
	if (KGameMatch* match = GetGameMatch())
		return &match->ReplayWriter;

	return nullptr;
}

bool IsWritingReplay()
{
	if (KReplayWriter* w = KReplayWriter::GetActive())
		return w->IsWriting();
	return false;
}

void KReplayWriter::WaitForCopy()
{
	while (bCopyInProgress) {}
}

void KReplayWriter::WriteFrameNumber()
{
	Buffer.resize(Buffer.size() + 4);
	u32 frame = KTime::FrameCount();
	memcpy(Buffer.data() - 4, &frame, 4);
}

void KReplayWriter::StartWriting()
{
	if (!bIsWriting)
	{
		File = std::ofstream((REPLAY_DIR + KString::GetTimeDateString() + ".kfdemo").Get(), std::ios::out | std::ios::binary);
		bIsWriting = true;
	}
}

void KReplayWriter::CopySnapshot(u8 classID, u32 netID, KSnapshot* prevData, KSnapshot* newData)
{
	u32 frame = KTime::FrameCount();
	if (frame <	FirstFrame)
		FirstFrame = frame;

	// first snapshot of this frame
	bool writeFrame = frame > LastWriteFrame || LastWriteFrame == 0;

	if (frame == FirstFrame)
	{
		if (writeFrame)
			WriteFrameNumber();

		// copy entire snapshot
		u32 size = newData->GetClassTypeSize();
		Buffer.resize(Buffer.size() + size + 5);
		u8* bufferEnd = Buffer.data() + Buffer.size() - size - 5;
		memcpy(bufferEnd, &classID, 1);
		memcpy(bufferEnd + 1, &netID, 4);
		memcpy(bufferEnd + 5, newData, size);
	}
	else
	{
		bool wrote = false;

		// diff against last frame
		for (u8 i = 0; i < newData->MemberCount(); i++)
		{
			bool change = true;

			// get the member offset and member size
			u8* addr = nullptr;
			u8 size = 0;
			newData->GetAddressAndSize(i, addr, size);

			// compare current frame with the last acked frame
			if (prevData)
			{
				u8* prev = nullptr;
				prevData->GetAddressAndSize(i, prev, size);
				change = std::memcmp(addr, prev, size) != 0;
			}

			if (change)
			{
				if (!wrote)
				{
					if (writeFrame)
					{
						WriteFrameNumber();
						writeFrame = false;
					}

					// we only need this info if properties will be written
					Buffer.resize(Buffer.size() + 5);
					u8* bufferEnd = Buffer.data() + Buffer.size() - 5;
					memcpy(bufferEnd, &classID, 1);
					memcpy(bufferEnd + 1, &netID, 4);
					wrote = true;
				}

				Buffer.resize(Buffer.size() + size);
				u8* bufferEnd = Buffer.data() + Buffer.size() - size;
				memcpy(bufferEnd, addr, size);
			}
		}
	}
}

void KReplayWriter::AppendBufferToFile()
{
	WaitForCopy();
	std::atomic<bool> buffCopied = { false };

	const auto copyBuffer = [&]() -> void
	{
		bCopyInProgress = true;
		TVector<u8> buffer = Buffer;
		buffCopied = true;
		File.write((char*)buffer.data(), buffer.size());
		bCopyInProgress = false;
	};

	std::thread t(copyBuffer);
	t.detach();
	while (!buffCopied) {}
	Buffer.clear();
}

void KReplayWriter::CloseFile()
{
	if (File.is_open())
		File.close();
}
