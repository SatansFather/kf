#pragma once

class KEntProp
{
public:

	virtual ~KEntProp();

	class KEntity* GetEntity();

	template<typename T>
	T* GetProperty()
	{
		return dynamic_cast<T*>(this);
	}	
};