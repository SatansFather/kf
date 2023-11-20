#pragma once

#include "../math/vec3.h"
#include "../audio/audio.h"

// pending data to build a packet for sound info to clients
struct KReppedSound
{
	enum 
	{	
		// send 16 bit int position if sound position fits that, send float if its further out
		// neither of these flags means 2D sound
		Position16 = 1,
		Position32 = 2,

		// spawn sound at position of net entity
		AtNetID = 4,
		AttachNetID = 8,
		AttachedPlayAfterDeath = 16,

		// flag these if they are non default
		HasAttenuation = 32,
		HasVolume = 64,
	};
	
	u8 Flags = 0;
	u16 AttenuationDistance = 2048;
	u8 Volume = 255; // 0-255 == 0-1
	KSoundID ID = 0;
	GVec3 Position;
	u32 Frame = 0;
	u32 NetID = 0;
	u8 PlayerIndex = NULL_PLAYER;
	EReplicatedSound RepMethod;

	bool bIsPositional = false;
	u32 CutoffFrame = MAX_U32;

	u32 AllocSize = 0;

	// 25 is max size for a packed sound
	u8 PackedData[25];

#pragma pack(push, 1)
	struct
	{
		i16 x16 = 0, y16 = 0, z16 = 0;
		f32 x32 = 0, y32 = 0, z32 = 0;
	} Pos;
#pragma pack(pop)

	void SetSoundID(KSoundID id);
	void SetPosition(const GVec3& pos);
	void SetVolume(f32 volume);
	void SetAttenuation(f32 atten);
	void SetFrame(u32 frame);
	void SetAtNetID(u32 id);
	void SetAttachNetID(u32 id, bool playAfterDeath = false);
	void SetPlayerIndex(u8 index);
	void SetRepMethod(EReplicatedSound method);

	void PreparePack();
	void Pack(void* data);
};