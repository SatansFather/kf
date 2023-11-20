#pragma once

#include "kfglobal.h"

struct KHashedPoolable
{
	std::array<u32, 4> Hash;

	KHashedPoolable() = default;
	KHashedPoolable(KPoolable* obj)
	{
		HashObject(obj);
	}

	KHashedPoolable(std::array<u32, 4> arr)
	{
		Hash = arr;
	}

	void HashObject(KPoolable* obj)
	{
		u64 mempoolAddr = (u64)(obj->Poolable_GetPool());
		u32 addr1 = mempoolAddr & 0xFFFFFFFF00000000ULL;
		u32 addr2 = mempoolAddr & 0x00000000FFFFFFFFULL;

		Hash[0] = obj->Poolable_GetID();
		Hash[1] = obj->Poolable_GetIndex();
		Hash[2] = addr1;
		Hash[3] = addr2;

	}

	bool IsValid()
	{
		u64 poolAddr = 0;
		poolAddr = Hash[2] | Hash[3];
		KPool::DataPoolBase* pool = (KPool::DataPoolBase*)poolAddr;
		TObjRef<KPoolable> obj(Hash[0], Hash[1], pool);
		return obj.IsValid();
	}
};
struct ArrayHasher 
{
	std::size_t operator()(const std::array<u32, 4>& a) const 
	{
		std::size_t h = 0;

		for (u32 e : a) 
			h ^= std::hash<int>{}(e)+0x9e3779b9 + (h << 6) + (h >> 2);

		return h;
	}
};