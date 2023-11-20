#pragma once

#include "kfglobal.h"
#include <typeindex>

#pragma pack(push, 1)
// destroys are packed in their entirety and should not be virtual so we dont pack vpointers
struct KDestroySnapshot
{
	u8 ClassID = 0;
	u32 NetID = 0;

	static UPtr<KDestroySnapshot> FillDestroy(class KSnapshottable* obj);
	static u32 GetSize(u8 classID);
	static void Apply(class KSnapshottable* obj, u8* destroyData);
};
#pragma pack(pop)

struct KSnapshot
{
	virtual u32 MemberCount() = 0;
	virtual void GetAddressAndSize(u8 propIndex, u8*& addr, u8& size) = 0;
	virtual bool ClientNeedsProperty(u8 propIndex, u8 playerIndex, class KSnapshottable* object, u32 lastAcked, bool storingReplay) = 0;
	virtual u32 GetClassTypeSize() = 0;

	virtual void PackObject(class KSnapshottable* obj) = 0;

	KSnapshot* GetHistoryAtFrame(u32 frame);

	static u32 Apply(u8 index, class KSnapshottable* object, const u8* data);
	static KString NameFromClassID(u8 id);
	static u8 LookUpClassID(std::type_index ind);
	static KSnapshot* GetHistoryForObject(u8 classID, u32 netID);
	static void RemoveObjectFromHistory(u8 classID, u32 netID);
	static KSnapshot* GetDefaultSnapshot(u8 classID);
	static class KSnapshottable* CreateReplicatedObject(u32 classID, u32 netID);
	static void ApplyOnReps(bool destroy);

	static u8 GetPlayerClassID();
};

