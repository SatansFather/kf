#pragma once

#include <cstdint>
#include <vector>

namespace KPool
{

// all objects which will be pooled MUST derive from this
// functions are prefixed with Poolable_ to avoid naming conflicts
class Poolable
{
	friend class DataPoolBase;

	uint32_t _PoolID = 0;
	uint32_t _PoolIndex = 0;
	class DataPoolBase* _MemPool = nullptr;

public:

	uint32_t Poolable_GetID() const { return _PoolID; }
	uint32_t Poolable_GetIndex() const { return _PoolIndex; }
	class DataPoolBase* Poolable_GetPool() { return _MemPool; }

	void Poolable_FillRefData(uint32_t& id, uint32_t& index, class DataPoolBase*& pool);

	// calls the destruction of this object
	void Poolable_Destroy();

	// called after deserializing this object
	// extraData contains whatever data was stored in PreSerialize
	virtual void Poolable_PostDeserialize(std::vector<char>& extraData) {}

	// called before serializing this object
	// extraData is an empty vector that can be filled with any data that needs to be accessed in PostDeserialize
	virtual void Poolable_PreSerialize(std::vector<char>& extraData) {}

	// whether or not this object represents a valid pool entry
	bool Poolable_IsValid() const;

	// functions implementable by classes using this interface
	virtual void Poolable_OnInstantiated() {}
	virtual void Poolable_PreDestroy() {}

	virtual void Tick() {}
};

};