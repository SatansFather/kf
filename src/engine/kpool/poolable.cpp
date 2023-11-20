#include "poolable.h"
#include "data_pool_base.h"
#include <cassert>

void KPool::Poolable::Poolable_FillRefData(uint32_t& id, uint32_t& index, DataPoolBase*& pool)
{
	id = _PoolID;
	index = _PoolIndex;	
	pool = _MemPool;
}

void KPool::Poolable::Poolable_Destroy()
{
	assert(Poolable_IsValid());
	_MemPool->DestroyIndex(_PoolIndex);
}

bool KPool::Poolable::Poolable_IsValid() const
{
	return 
		_PoolID > 0 && 
		_MemPool && 
		_MemPool->GetIdAtIndex(_PoolIndex) == _PoolID;
}

