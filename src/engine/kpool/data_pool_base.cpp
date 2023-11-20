#include "data_pool_base.h"
#include "poolable.h"
#include <fstream>
#include <cassert>

KPool::DataPoolBase::PoolData KPool::DataPoolBase::AllPools;
std::map<std::string, KPool::DataPoolBase*> KPool::DataPoolBase::PoolNameMap;

void KPool::DataPoolBase::InitPoolable(class Poolable* obj, uint32_t index, uint32_t id, DataPoolBase* mempool)
{
	obj->_MemPool = mempool;
	obj->_PoolIndex = index;
	obj->_PoolID = id;
}

void KPool::DataPoolBase::ResetAllPools()
{
	for (auto& pool : AllPools.Data)
		pool->DestroyAll();
}

void KPool::DataPoolBase::DestroyAllPools()
{
	for (int i = (int)AllPools.Data.size() - 1; i >= 0; i--)
		AllPools.Data[i]->DestroyPool();
}

#if 0
std::vector<char> KPool::DataPoolBase::SerializeAllPools(std::string fileName)
{
	std::vector<char> data;
	for (auto& pool : AllPools.Data)
	{
		std::vector<char> ser = pool->Serialize();
		data.insert(data.end(), ser.begin(), ser.end());
	}

	if (!fileName.empty())
	{
		std::ofstream file(fileName, std::ios::out | std::ios::binary);
		file.write(data.data(), data.size());
	}

	return data;
}

void KPool::DataPoolBase::DeserializeAllPools(std::vector<char> data)
{
	uint32_t offset = 0;

	while (offset < data.size())
	{
		uint32_t nameSize;
		memcpy(&nameSize, data.data() + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		std::string name = "";
		name.resize(nameSize, '\0');
		memcpy(name.data(), data.data() + offset, nameSize);
		offset += nameSize;

		assert(PoolNameMap.contains(name));
		DataPoolBase* pool = PoolNameMap[name];
		offset += pool->Deserialize(data.data() + offset);

		assert(offset <= data.size());
	}
}
#endif

void KPool::DataPoolBase::AddPool(std::unique_ptr<DataPoolBase> pool)
{
	PoolNameMap[pool->PoolName] = pool.get();
	AllPools.Data.push_back(std::move(pool));
}

void KPool::DataPoolBase::RemoveFromPools()
{
	if (PoolNameMap.contains(PoolName))
		PoolNameMap.erase(PoolName);

	for (uint32_t i = 0; i < AllPools.Data.size(); i++)
	{
		if (AllPools.Data[i].get() == this)
		{
			AllPools.Data.erase(AllPools.Data.begin() + i);
			return;
		}
	}
}

void KPool::DestroyAllPools()
{
	DataPoolBase::DestroyAllPools();
}

void KPool::ResetAllPools()
{
	DataPoolBase::ResetAllPools();
}
