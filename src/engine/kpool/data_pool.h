#pragma once

#include <forward_list>
#include "data_pool_base.h"
#include "pool_ref.h"

#if __linux__
#include <string.h>
#endif

namespace KPool
{

template <typename T>
class DataPool : public DataPoolBase
{
	static_assert(std::is_base_of<Poolable, T>::value, "type type must derive from Poolable");
	typedef PoolNode<T> PoolNode;
	typedef std::unique_ptr<PoolNode[]> NodePtr;
	
	struct NodeBuffer
	{
		NodePtr Data;

		// free object indices that can be used when instantiating into this buffer
		std::forward_list<uint32_t> FreeList;
		
		// max number of objects this buffer can hold
		uint32_t MaxObjects = 0;

		// index offset for free list, based on buffer index
		uint32_t IndexOffset = 0;								

		NodeBuffer(uint32_t count, uint32_t indexOffset = 0, bool noFreeList = false) : MaxObjects(count), IndexOffset(indexOffset)
		{
			// allocate data as char to avoid calling constructors
			const size_t sz = count * sizeof(PoolNode);
			char* data = new char[sz];
			memset(data, 0, sz);

			// take ownership
			Data = NodePtr( (PoolNode*) data );

			if (!noFreeList) ResetFreeList();
		}

		void ResetFreeList()
		{
			FreeList.clear();
			FreeList.resize(MaxObjects);
			uint32_t i = 0;
			for (uint32_t& val : FreeList)
			{
				val = i + IndexOffset;
				i++;
			}
		}
	};

	// vector index 0 is the base pool, all other indices are overflow
	// unique ptr is used so resizing wont cause the entire buffer to be reallocated
	//   which would invalidate any pointers to pool objects
	std::vector<NodeBuffer> Buffers;

	// pointer to this pool, gettable through GetPool()
	static DataPool<T>* StaticPool;

public:

	static DataPool<T>* CreatePool(
		uint32_t objectCount, 
		uint32_t overflowCount = 0, 
		EOverflowResponse overflowResponse = EOverflowResponse::Overflow, 
		const std::string& name = "");

	template <typename... TArgs>
	ObjRef<T> CreateNew(TArgs&&... args);

	static DataPool<T>* GetPool() { return StaticPool; }

private:

	DataPool() = default;

	DataPool(
		uint32_t objectCount, 
		uint32_t overflowCount, 
		EOverflowResponse overflowResponse, 
		const std::string& name);

	// adds a new buffer with the number of elements
	void AddBuffer(uint32_t count, bool noFreeList = false);

	// gets the pool node at the given index starting at index 0 of buffer 0
	// can be null if the index is out of range
	// bufferIndex is filled with the index of the buffer used
	PoolNode* GetNodeAtIndex(uint32_t index, uint32_t& bufferIndex) const;

	// overloaded version if out buffer index is not desired
	PoolNode* GetNodeAtIndex(uint32_t index) const;

	PoolNodeBase* GetNodeAtIndex_Base(uint32_t index) const override
	{
		return GetNodeAtIndex(index);
	}

	Poolable* GetObjectAtIndex(uint32_t index) const override;

	bool IsInitialized() const { return Buffers.size() > 0; }

	// gets the next available free index from the lowest buffer index possible
	// returns UINT32_MAX if nothing is available
	uint32_t TakeNextFreeIndex();

public:

	/* virtual interface */
	bool DestroyIndex(uint32_t index) override;
	void DestroyAll() override;
	uint32_t GetIdAtIndex(uint32_t index) override;
	uint32_t GetTypeSize() override;
	class Poolable* GetObjectAtIndex(uint32_t index) override;
	bool IndexIsActive(uint32_t index) override;
	void DestroyPool() override;
	//std::vector<char> Serialize() override;
	//size_t Deserialize(char* buffer) override;
};

template <typename T>
Poolable* KPool::DataPool<T>::GetObjectAtIndex(uint32_t index) const
{
	PoolNode* entry = GetNodeAtIndex(index);
	if (entry) return &entry->Object;
	return nullptr;
}

#if 0
template <typename T>
size_t KPool::DataPool<T>::Deserialize(char* buffer)
{
	DestroyAll();

	size_t offset = 0;

	// read meta data
	size_t metaStartAddr = (size_t)(&NextID);
	size_t metaEndAddr = (size_t)(&StartIndex) + sizeof(StartIndex);
	size_t metaSize = metaEndAddr - metaStartAddr;
	memcpy((void*)metaStartAddr, buffer + offset, metaSize);
	offset += metaSize;

	std::vector<uint32_t> freeIndices(ObjectCount);

	// probably a better way to do this
	for (uint32_t i = 0; i < ObjectCount; i++)
		freeIndices[i] = i;

	int count = int(CurrentCount - ObjectCount);

	// allocate overflow buffers
	if (OverflowResponse == EOverflowResponse::Overflow)
	{
		for (count += OverflowCount; count > (int)OverflowCount; count -= OverflowCount)
		{
			AddBuffer(OverflowCount);

			uint32_t indSize = (uint32_t)freeIndices.size();

			std::vector<uint32_t> overflowIndices(OverflowCount);
			for (uint32_t i = indSize; i < indSize + OverflowCount; i++)
				overflowIndices[i - indSize] = i;

			freeIndices.insert(freeIndices.end(), overflowIndices.begin(), overflowIndices.end());
		}
	}
	else if (CurrentCount > ObjectCount)
	{
		throw std::runtime_error("pool was serialized with more data than the current capacity, but overflow is not enabled");
	}

	for (uint32_t i = 0; i < CurrentCount; i++)
	{
		// first bytes here are node index
		uint32_t nodeIndex;
		memcpy(&nodeIndex, buffer + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		// first bit of index is flagged if we have extra data packed
		bool hasExtra = false;
		if (nodeIndex & 0x80000000)
		{
			nodeIndex &= ~0x80000000;
			hasExtra = true;
		}

		// interpret node from buffer data
		PoolNode* node = (PoolNode*)(buffer + offset);
		offset += sizeof(PoolNode);

		// place node at proper buffer location
		memcpy(GetNodeAtIndex(nodeIndex), node, sizeof(PoolNode));
		
		// read extra data if we have it
		std::vector<char> extra;
		if (hasExtra)
		{
			uint32_t extraSize = *(uint32_t*)(buffer + offset);
			offset += sizeof(uint32_t);

			extra.resize(extraSize);
			memcpy(extra.data(), buffer + offset, extraSize);
			offset += extraSize;
		}

		node->Object.Poolable_PostDeserialize(extra);

		// invalidate index
		freeIndices[nodeIndex] = UINT32_MAX;
	}

	uint32_t startIndex = 0;
	for (NodeBuffer& buff : Buffers)
	{
		buff.FreeList.clear();
		for (uint32_t i = startIndex; i < buff.MaxObjects; i++)
		{
			if (freeIndices[i] != UINT32_MAX)
				buff.FreeList.push_front(freeIndices[i]);
		}
		startIndex += buff.MaxObjects;
	}

	return offset;
}

template <typename T>
std::vector<char> KPool::DataPool<T>::Serialize()
{
	// size considers each node plus extra bytes for its index
	size_t sizeInBytes = GetCurrentCount() * (sizeof(PoolNode) + sizeof(uint32_t));

	// first thing written is name and its size
	sizeInBytes += sizeof(uint32_t) + PoolName.size();

	// also need to consider X bytes for pool metadata
	// all of these values are packed in the same block of memory
	//   get start and end address so it can be done in one copy
	size_t metaStartAddr = (size_t)(&NextID);
	size_t metaEndAddr = (size_t)(&StartIndex) + sizeof(StartIndex);
	size_t metaSize = metaEndAddr - metaStartAddr;
	sizeInBytes += metaSize;

	// shouldnt actually be 0 since meta data is included
	if (sizeInBytes == 0)
		return std::vector<char>();

	// main pool data
	std::vector<char> data(sizeInBytes);

	size_t bufferPos = 0;

	uint32_t nameSize = PoolName.size();
	memcpy(data.data() + bufferPos, &nameSize, sizeof(uint32_t));
	bufferPos += sizeof(uint32_t);

	memcpy(data.data() + bufferPos, PoolName.c_str(), nameSize);
	bufferPos += nameSize;

	// store meta data first
	memcpy(data.data() + bufferPos, (void*)(metaStartAddr), metaSize);
	bufferPos += metaSize;

	// write object data to buffer
	uint32_t index = StartIndex;
	while (index != UINT32_MAX)
	{
		PoolNode* entry = GetNodeAtIndex(index);

		std::vector<char> extra;
		entry->Object.Poolable_PreSerialize(extra);
		uint32_t extraSize = extra.size();

		// nodes dont store their own index but we need to 
		// know where to place it when this is deserialized
		// if we have extra data, flag the first bit of the index

		uint32_t serialIndex = index;

		// set flag in index
		if (extraSize > 0) serialIndex |= 0x80000000UL;

		memcpy(data.data() + bufferPos, &serialIndex, sizeof(uint32_t));
		bufferPos += sizeof(uint32_t);

		memcpy(data.data() + bufferPos, entry, sizeof(PoolNode));
		bufferPos += sizeof(PoolNode);
		index = entry->Next;

		// pack extra data
		if (extraSize > 0)
		{
			char* exSize = (char*)&extraSize;
			extra.insert(extra.begin(), exSize, exSize + sizeof(uint32_t));

			data.insert(data.begin() + bufferPos, extra.begin(), extra.end());
			bufferPos += extra.size();
		}
	}

	return data;
}
#endif

template <typename T>
void KPool::DataPool<T>::DestroyPool()
{
	DestroyAll();
	StaticPool = nullptr;
	Buffers.clear();

	RemoveFromPools();
}

template <typename T>
KPool::DataPool<T>* KPool::DataPool<T>::StaticPool = nullptr;

template <typename T>
template <typename... TArgs>
ObjRef<T> KPool::DataPool<T>::CreateNew(TArgs&&... args)
{
	uint32_t i = TakeNextFreeIndex();
	if (i == UINT32_MAX)
	{
		switch (OverflowResponse)
		{
			case EOverflowResponse::Overflow:
			{
				// should never happen
				// TakeNextFreeIndex allocates a new buffer
				assert(false);
				break;
			}
			case EOverflowResponse::PreventNew:
			{
				return ObjRef<T>(nullptr);
			}
			case EOverflowResponse::ReplaceOld:
			{
				// destroy the oldest object and force StartIndex back into the free list
				DestroyIndex(StartIndex);
				i = TakeNextFreeIndex();
				break;
			}
		}
	}

	// if we're adding to an empty pool, our list starts here
	if (CurrentCount == 0) StartIndex = i;

	CurrentCount++;

	PoolNode* entry = GetNodeAtIndex(i);
	entry->Instantiate(NextID, args...); // calls constructor
	InitPoolable(&entry->Object, i, NextID, this);

	if (EndIndex != UINT32_MAX)
	{
		// make this the End object and make the current End object point to this
		PoolNode* prevEnd = GetNodeAtIndex(EndIndex);
		prevEnd->Next = i;
		entry->Prev = EndIndex;
	}

	// this is now at the end of the list
	EndIndex = i;

	ObjRef<T> out(&(entry->Object));
	entry->Object.Poolable_OnInstantiated();

	NextID++;

	// might have overflowed (max u32, not buffer)
	// TODO this actually just breaks everything but is really really really unlikely
	if (NextID == 0) NextID = 1;

	return out;
}

template <typename T>
void KPool::DataPool<T>::AddBuffer(uint32_t count, bool noFreeList)
{
	uint32_t sz = (uint32_t)Buffers.size();

	uint32_t offset = 0;
	if (sz > 0) offset += ObjectCount + OverflowCount * (sz - 1);
	
	NodeBuffer buff(count, offset, noFreeList);
	Buffers.push_back(std::move(buff));
}

template <typename T>
uint32_t KPool::DataPool<T>::TakeNextFreeIndex()
{
	uint32_t index = UINT32_MAX;
	for (uint32_t i = 0; i < Buffers.size(); i++)
	{
		NodeBuffer& buff = Buffers[i];
		if (buff.FreeList.empty()) continue;
		index = buff.FreeList.front();
		Buffers[i].FreeList.pop_front();
		return index;
	}

	// nothing available, allocate overflow if desired
	if (OverflowResponse == EOverflowResponse::Overflow)
	{
		AddBuffer(OverflowCount);
		NodeBuffer& buff = Buffers[Buffers.size() - 1];
		index = buff.FreeList.front();
		buff.FreeList.pop_front();
	}

	return index;
}

template <typename T>
bool KPool::DataPool<T>::IndexIsActive(uint32_t index)
{
	if (PoolNodeBase* node = GetNodeAtIndex(index))
		return node->IsActive();
		
	return false;
}

template <typename T>
class Poolable* KPool::DataPool<T>::GetObjectAtIndex(uint32_t index)
{
	if (PoolNode* node = GetNodeAtIndex(index))
		return &(node->Object);

	return nullptr;
}

template <typename T>
uint32_t KPool::DataPool<T>::GetTypeSize()
{
	return sizeof(T);
}

template <typename T>
uint32_t KPool::DataPool<T>::GetIdAtIndex(uint32_t index)
{
	if (PoolNode* node = GetNodeAtIndex(index))
		return node->ID;

	return UINT32_MAX;
}

template <typename T>
bool KPool::DataPool<T>::DestroyIndex(uint32_t index)
{
	uint32_t bufferIndex;                // out parameter
	PoolNode* entry = GetNodeAtIndex(index, bufferIndex);
	if (!entry || !entry->IsActive()) return false;

	// join prev with next
	PoolNode* prev = (entry->Prev == UINT32_MAX) ? nullptr : GetNodeAtIndex(entry->Prev);
	PoolNode* next = (entry->Next == UINT32_MAX) ? nullptr : GetNodeAtIndex(entry->Next);
	if (prev) prev->Next = entry->Next;
	if (next) next->Prev = entry->Prev;

	// if the current node being destroyed is next in line for iteration,
	// we need to adjust next iteration to be this node's next
	//if (index == IterIndex) IterIndex = entry->Next;
	for (PoolIterator& it : ActiveIterators)
	{
		if (it.Index == index)
			it.Index = entry->Next;
	}

	// adjust StartIndex and EndIndex if needed

	// this entry is the start
	if (!prev) StartIndex = (next ? entry->Next : UINT32_MAX);

	// this entry is the end
	if (!next) EndIndex = (prev ? entry->Prev : UINT32_MAX);

	entry->Object.Poolable_PreDestroy();

	CurrentCount--;
	Buffers[bufferIndex].FreeList.push_front(index);
	entry->Invalidate();

	return true;
}

template <typename T>
void KPool::DataPool<T>::DestroyAll()
{
	// call destructors on all live objects
	uint32_t index = StartIndex;
	while (index != UINT32_MAX)
	{
		PoolNode* entry = GetNodeAtIndex(index);
		uint32_t nextIndex = entry->Next;
		DestroyIndex(index);
		index = nextIndex;
	}

	CurrentCount = 0;

	// free all overflow buffers
	Buffers.erase(Buffers.begin() + 1, Buffers.end());

	// reset main buffer
	Buffers[0].ResetFreeList();
	memset(Buffers[0].Data.get(), 0, sizeof(T) * ObjectCount);

	StartIndex = UINT32_MAX;
	EndIndex = UINT32_MAX;
}

template <typename T>
DataPool<T>* KPool::DataPool<T>::CreatePool(uint32_t objectCount, uint32_t overflowCount, EOverflowResponse overflowResponse, const std::string& name)
{
	assert(!StaticPool);


	if (overflowResponse == EOverflowResponse::Overflow && overflowCount == 0)
	{
		overflowCount = objectCount / 2;
		if (overflowCount == 0) 
			overflowCount = 1;
	}

	// cant use make_unique because DataPool constructor is private
	std::unique_ptr<DataPool> pool(new DataPool(
		objectCount, 
		overflowCount, 
		overflowResponse, 
		name.empty() ? typeid(T).name() : name));

	StaticPool = pool.get();
	AddPool(std::move(pool));

	return StaticPool;
}

template <typename T>
KPool::DataPool<T>::DataPool(uint32_t objectCount, uint32_t overflowCount, EOverflowResponse overflowResponse, const std::string& name)
	: DataPoolBase(objectCount, overflowCount, overflowResponse, name)
{
	AddBuffer(objectCount);
}

template <typename T>
PoolNode<T>* KPool::DataPool<T>::GetNodeAtIndex(uint32_t index, uint32_t& bufferIndex) const
{
	if (index == UINT32_MAX) return nullptr;

	bufferIndex = index >= ObjectCount && OverflowCount > 0 ? 
		((index - ObjectCount) / OverflowCount) + 1 
		: 0;

	if (bufferIndex > 0 && bufferIndex >= Buffers.size())
	{
		// attempting to get overflow index that doesnt exist yet
		return nullptr;
	}

	uint32_t indexInBuffer = index;
	if (bufferIndex > 0)
	{	// adjust object index for overflow buffer index
		indexInBuffer -= ObjectCount + (OverflowCount * (bufferIndex - 1));
	}

	return Buffers[bufferIndex].Data.get() + indexInBuffer;
}

template <typename T>
PoolNode<T>* KPool::DataPool<T>::GetNodeAtIndex(uint32_t index) const
{
	uint32_t out = 0;
	return GetNodeAtIndex(index, out);
}

};