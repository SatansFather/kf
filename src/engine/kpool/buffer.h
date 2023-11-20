#pragma once

#include <forward_list>
#include <memory>

namespace KPool
{

template <typename T>
class PoolBuffer
{
	static_assert(std::is_base_of<Poolable, T>::value, "type type must derive from Poolable");
	typedef PoolNode<T> PoolNode;
	typedef std::unique_ptr<PoolNode[]> NodePtr;

	// vector index 0 is the base pool, all other indices are overflow
	// unique ptr is used so resizing wont cause the entire buffer to be reallocated
	//   which would invalidate any pointers to pool objects
	std::vector<NodePtr> Buffers;

	const uint32_t BufferCount = 0;
	const uint32_t OverflowCount = 0;

	PoolBuffer() = default;
	PoolBuffer(uint32_t count, uint32_t overflowCount);

	// gets the pool node at the given index starting at index 0 of buffer 0
	// can be null if the index is out of range
	PoolNode* GetNodeAtIndex(uint32_t index);

	void AllocateOverflow();
	void ClearOverflow(uint32_t i);
	void ClearAll();

	bool IsInitialized() const { return Buffer.size() > 0; }
};

template <typename T>
void KPool::PoolBuffer<T>::ClearOverflow(uint32_t i)
{
	
}

template <typename T>
void KPool::PoolBuffer<T>::AllocateOverflow(uint32_t i)
{
	
}

template <typename T>
PoolNode* KPool::PoolBuffer<T>::GetNodeAtIndex(uint32_t index)
{
	const uint32_t bufferIndex = index > BufferCount ? (index - BufferCount) % OverflowCount : 0;
	
	if (bufferIndex > 0 && bufferIndex >= Buffers.size())
	{
		// attempting to get overflow index that doesnt exist yet
		return nullptr;
	}

	uint32_t indexInBuffer = index;
	if (bufferIndex > 0)
	{	// adjust object index for overflow buffer index
		indexInBuffer -= BufferCount * (OverflowCount * index);
	}
	
	return Buffers[bufferIndex].get() + indexInBuffer;
}

template <typename T>
PoolBuffer<T>::PoolBuffer(uint32_t count)
{
	Buffers.resize(1);
	const size_t sz = count * sizeof(PoolNode);

	// allocate data as char to avoid calling constructors
	char* data = new char[sz];
	memset(data, 0, sz);
	Buffers[0] = std::unique_ptr<PoolNode[]>((PoolNode*)data);
}

};
