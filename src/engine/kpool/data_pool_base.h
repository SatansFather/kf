#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <list>
#include <map>
#include "pool_node.h"

namespace KPool
{

// destroys all of the pools, removing them from existence
void DestroyAllPools();

// destroys all objects in all pools
void ResetAllPools();

enum class EOverflowResponse
{
	Overflow,
	PreventNew,
	ReplaceOld
};

class DataPoolBase
{
	friend class Poolable;

protected:

	const uint32_t          ObjectCount      = 0; // number of elements in the main buffer
	const uint32_t          OverflowCount    = 0; // number of elements in each overflow buffer
	const EOverflowResponse OverflowResponse = EOverflowResponse::Overflow;
	const std::string       PoolName;   // used for serialization

	uint32_t NextID       = 1;          // next ID to assign a new object
	uint32_t CurrentCount = 0;          // current number of active objects/nodes
	uint32_t EndIndex     = UINT32_MAX; // index of the last entry to be iterated
	uint32_t StartIndex   = UINT32_MAX; // index of first entry to be iterated
	                                    // StartIndex and EndIndex also reflect age

public:
	// allow iteration over public member AllPools without directly exposing vector
	struct PoolData
	{
		friend class DataPoolBase;

	private:

		typedef std::vector<std::unique_ptr<DataPoolBase>> PoolVector;
		PoolVector Data;

	public:

		PoolVector::iterator begin() { return Data.begin(); }
		PoolVector::iterator end() { return Data.end(); }
		PoolVector::const_iterator begin() const { return Data.begin(); }
		PoolVector::const_iterator end() const { return Data.end(); }
		PoolVector::const_iterator cbegin() const { return Data.cbegin(); }
		PoolVector::const_iterator cend() const { return Data.cend(); }

	};

	static PoolData AllPools;
	static std::map<std::string, DataPoolBase*> PoolNameMap;

protected:

	DataPoolBase(
		uint32_t objectCount, 
		uint32_t overflowCount, 
		EOverflowResponse overflowResponse, 
		const std::string& name)
		: 
		OverflowResponse(overflowResponse), 
		OverflowCount(overflowCount), 
		ObjectCount(objectCount), 
		PoolName(name) {}

	void InitPoolable(class Poolable* obj, uint32_t index, uint32_t id, DataPoolBase* mempool);

public:

	uint32_t GetMaxCount()         const { return ObjectCount; }
	uint32_t GetMaxOverflowCount() const { return OverflowCount; }
	uint32_t GetCurrentCount()     const { return CurrentCount; }

	static void ResetAllPools();
	static void DestroyAllPools();

	//static std::vector<char> SerializeAllPools(std::string fileName = "");
	//static void DeserializeAllPools(std::vector<char> data);

protected:

	static void AddPool(std::unique_ptr<DataPoolBase> pool);
	void RemoveFromPools();

public:

	/* virtual methods that require template type knowledge */

	// destroys the object at the given index, if valid
	// returns true if an object was successfully destroyed
	virtual bool DestroyIndex(uint32_t index) = 0;

	// destroys all instantiated objects and clears overflow
	virtual void DestroyAll() = 0;

	// gets the id of the object residing at the given index
	virtual uint32_t GetIdAtIndex(uint32_t index) = 0;

	// returns the size of the template type
	virtual uint32_t GetTypeSize() = 0;

	// returns the object at the given index
	virtual class Poolable* GetObjectAtIndex(uint32_t index) = 0;

	// returns true if the given index is a currently active object
	virtual bool IndexIsActive(uint32_t index) = 0;

	// destroy all objects in this pool and then destroy the pool
	virtual void DestroyPool() = 0;

protected:

	// tells the derived class to call GetNodeAtIndex, only used for iterators
	virtual PoolNodeBase* GetNodeAtIndex_Base(uint32_t index) const = 0;

	virtual Poolable* GetObjectAtIndex(uint32_t index) const = 0;

	// convert pool data into a single buffer that can be loaded later
	// reloading this data will have no impact on simulation determinism
	//virtual std::vector<char> Serialize() = 0;

	// reset a pool and fill it with data created by Serialize()
	// returns number of bytes read
	//virtual size_t Deserialize(char* buffer) = 0;

public:

	/* range-based-for iteration */

	struct PoolIterator
	{
		PoolNodeBase* CurrentNode = nullptr;
		DataPoolBase* Pool = nullptr;
		uint32_t Index = 0;
		uint32_t Next = UINT32_MAX;

		void Increment()
		{
			CurrentNode = Pool->GetNodeAtIndex_Base(Next);
			if (CurrentNode)
			{
				Index = Next;
				Next = CurrentNode->Next;
			}
			else
			{
				Next = UINT32_MAX;
				Index = UINT32_MAX;
			}
		}
	};

	struct PoolIteratorHandle
	{
		std::list<PoolIterator>::iterator Iterator;

		~PoolIteratorHandle()
		{
			Iterator->Pool->ActiveIterators.erase(Iterator);
		}

		PoolIteratorHandle& operator++()
		{
			Iterator->Increment();
			return *this;
		}

		Poolable* operator*()
		{
			return Iterator->Pool->GetObjectAtIndex(Iterator->Index);
		}

		bool operator!=(const PoolIterator& other)
		{
			return Iterator->Index != other.Index;
		}
	};

	std::list<typename DataPoolBase::PoolIterator> ActiveIterators;

	PoolIteratorHandle begin()
	{
		// StartIndex is UINT32_MAX if startNode is null

		PoolNodeBase* startNode = GetNodeAtIndex_Base(StartIndex);
		uint32_t next = startNode ? startNode->Next : UINT32_MAX;

		// fill PoolIteratorHandle with list iterator to new PoolIterator
		return { ActiveIterators.insert(ActiveIterators.end(), { startNode, this, StartIndex, next } ) };
	}

	PoolIterator end()
	{
		// iteration ends when the index is no longer valid (aka max u32)
		return { nullptr, this, UINT32_MAX, UINT32_MAX };
	}

};

};