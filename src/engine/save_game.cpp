#include "save_game.h"

void KSaveGame::SaveState()
{
	/*FrameNumber = KTime::FrameCount();

	UPtr<u8[]> buffer;
	u32 size = 0;

	// find out size required
	for (auto& kv : *KDataPoolBase::GetEntityPoolMap())
	{
		u32 count = kv.second->GetCurrentCount();
		if (count > 0)
		{
			KString typeName = kv.first->name();
			u32 typeSize = kv.second->GetTypeSize();
			size += sizeof(count) + sizeof(typeSize) + typeName.Size() + 1; // 1 byte for name length
			size += count * typeSize;
		}	
	}*/
}

void KSaveGame::WriteToFile(const KString& fileName)
{
	
}
