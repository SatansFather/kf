#pragma once

#if !_SERVER

class KUnorderedAccessView
{
public:
	virtual void* GetUAV() = 0;
};

#endif