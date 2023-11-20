#pragma once

#include <cstdint>
#include <cassert>
#include <type_traits>
#include "poolable.h"

namespace KPool
{

class PoolNodeBase
{
public:

	uint32_t ID = 0;
	uint32_t Prev = UINT32_MAX;
	uint32_t Next = UINT32_MAX;

	bool IsActive() const { return ID > 0; }
};

template <typename T>
class PoolNode : public PoolNodeBase
{
	static_assert(std::is_base_of<Poolable, T>::value, "pool node object must derive from Poolable");

public:

	T Object;

	// instantiate this object with the given ID
	template <typename... TArgs>
	void Instantiate(uint32_t id, TArgs&&... args);

	// resets this node and destroys the object
	void Invalidate();

	T* operator*()
	{
		return &Object;
	}
};

template <typename T>
template <typename... TArgs>
void PoolNode<T>::Instantiate(uint32_t id, TArgs&&... args)
{
	assert(!IsActive());

	ID = id;
	Object = *(new (&Object) T(std::forward<TArgs>(args)...));

	// reset these and let the calling code adjust them
	Prev = UINT32_MAX;
	Next = UINT32_MAX;
}

template <typename T>
void PoolNode<T>::Invalidate()
{
	ID = 0;
	Object.~T();
	Prev = UINT32_MAX;
	Next = UINT32_MAX;
}

};