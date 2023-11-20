#pragma once

#include "poolable.h"
#include "data_pool_base.h"

namespace KPool
{

template <typename T>
class ObjRef
{
	uint32_t PoolID = 0;
	uint32_t PoolIndex = 0;
	DataPoolBase* MemPool = nullptr;

// 32 bit pointers are smaller and thus reduce the size of poolable classes that store these refs
// need consistent size for easier serialization
#if (_WIN32 && !_WIN64) 
|| (__arm__ && !__arm64__) 
|| (__GNUC__ && (!__x86_64__ && !__ppc64__ && !__arm64__))
private:
	size_t __Pad;
#endif

public:

	ObjRef() = default;
	ObjRef(Poolable* poolable);
	ObjRef(uint32_t id, uint32_t index, DataPoolBase* pool)
		: PoolID(id), PoolIndex(index), MemPool(pool) {}

	template <typename Type>
	ObjRef(const ObjRef<Type>& other)
	{
		static_assert(std::is_base_of<T, Type>::value, "assigned type must inherit from base type");
		PoolID = other.GetPoolID();
		PoolIndex = other.GetPoolIndex();
		MemPool = other.GetPool();
	}

	uint32_t GetPoolID() const { return PoolID; }
	uint32_t GetPoolIndex() const { return PoolIndex; }
	DataPoolBase* GetPool() const { return MemPool; }

	// whether or not this refers to a valid pool object
	bool IsValid() const;

	// updates the MemPool pointer that was stored to the new address for this run of the application
	void UpdateAfterDeserialize();

	// dynamic casts the referred object to the template type or a child of the template type
	template <typename Type>
	Type* As() const;

	// gets a pointer to the object
	// can be null
	T* Get() const;

	bool operator==(const ObjRef<T>& other) const
	{
		return MemPool == other.MemPool
			&& PoolID == other.PoolID
			&& PoolIndex == other.PoolIndex;
	}

	bool operator!=(const ObjRef<T>& other) const
	{
		return !(*this == other);
	}
	bool operator<(const ObjRef<T>& other) const
	{
		size_t a = (size_t)MemPool + PoolIndex;
		size_t b = (size_t)other.MemPool + other.PoolIndex;
		return a < b;
	}
#ifdef KPOOL_ARROW_OPERATOR
	T* operator->() const
	{
		return Get();
	}
#endif
};

template <typename T>
void KPool::ObjRef<T>::UpdateAfterDeserialize()
{
	
}

template <typename T>
ObjRef<T>::ObjRef(Poolable* poolable)
{
	if (poolable)
	{	
		poolable->Poolable_FillRefData(PoolID, PoolIndex, MemPool);
	}
}

template <typename T>
bool ObjRef<T>::IsValid() const
{
	return MemPool && PoolID != 0 && MemPool->GetIdAtIndex(PoolIndex) == PoolID;
}

template <typename T>
template <typename Type>
Type* ObjRef<T>::As() const
{
	//static_assert(std::is_base_of<T, Type>::value || std::is_same<T, Type>::value,
	//	"type being casted is not compatible with template type");

	if (!IsValid()) return nullptr;
	return dynamic_cast<Type*>(MemPool->GetObjectAtIndex(PoolIndex));
}

template <typename T>
T* ObjRef<T>::Get() const
{
	if (!IsValid()) return nullptr;
	return dynamic_cast<T*>(MemPool->GetObjectAtIndex(PoolIndex));
}

};
