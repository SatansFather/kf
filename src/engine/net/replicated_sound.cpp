#include "replicated_sound.h"

void KReppedSound::SetVolume(f32 volume)
{
	Volume = volume * 255;
	if (Volume != 255)
		Flags |= HasVolume;
}

void KReppedSound::SetAttenuation(f32 atten)
{
	AttenuationDistance = atten;
	if (AttenuationDistance != 2048)
		Flags |= HasAttenuation;
}

void KReppedSound::SetFrame(u32 frame)
{
	Frame = frame;
}

void KReppedSound::SetSoundID(KSoundID id)
{
	ID = id;
}

void KReppedSound::SetPosition(const GVec3& pos)
{
	Position = pos;
	bIsPositional = true;
}

void KReppedSound::SetAtNetID(u32 id)
{
	NetID = id;
	Flags |= AtNetID;
}

void KReppedSound::SetAttachNetID(u32 id, bool playAfterDeath /*= false*/)
{
	NetID = id;
	Flags |= AttachNetID;
	if (playAfterDeath)
		Flags |= AttachedPlayAfterDeath;
}

void KReppedSound::SetPlayerIndex(u8 index)
{
	PlayerIndex = index;
}

void KReppedSound::SetRepMethod(EReplicatedSound method)
{
	RepMethod = method;
}

void KReppedSound::PreparePack()
{
	AllocSize = 5; // flags and frame
	AllocSize += sizeof(KSoundID::Index);

	if (Flags & HasVolume)
		AllocSize++;

	if (Flags & HasAttenuation)
		AllocSize += 2;

	if (bIsPositional)
	{
		bool is16 = true;
		for (u8 i = 0; i < 3; i++)
		{
			if (Position[i] < MIN_I16 || Position[i] > MAX_I16)
			{
				is16 = false;
				break;
			}
		}

		if (is16) 
		{
			Pos.x16 = Position.x;
			Pos.y16 = Position.y;
			Pos.z16 = Position.z;
			AllocSize += 6;
			Flags |= Position16;
		}
		else
		{
			Pos.x32 = Position.x;
			Pos.y32 = Position.y;
			Pos.z32 = Position.z;
			AllocSize += 12;
			Flags |= Position32;
		}
	}
	else if ((Flags & AtNetID) || (Flags & AttachNetID))
	{
		AllocSize += 4;
	}

	Pack(PackedData);
}

void KReppedSound::Pack(void* data)
{
	u32 cursor = 0;

	const auto copyData = [&cursor, data](void* d, u32 size) -> void
	{
		memcpy((u8*)data + cursor, d, size);
		cursor += size;
	};
	
	copyData(&Flags, 1);

	u32 sz = sizeof(KSoundID::Index);
	copyData(&ID.Index, sz);
	
	copyData(&Frame, 4);

	if (Flags & HasVolume) copyData(&Volume, 1);
	if (Flags & HasAttenuation) copyData(&AttenuationDistance, 2);
	if (Flags & Position16) copyData(&Pos.x16, 6);
	else if (Flags & Position32) copyData(&Pos.x32, 12);
	else if ((Flags & AtNetID) || (Flags & AttachNetID)) copyData(&NetID, 4);
	//else if (Flags & AtNetID) copyData(&NetID, 4);
	//else if (Flags & AttachNetID) {}
}
