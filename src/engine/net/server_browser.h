#pragma once

#include "kfglobal.h"
#include <atomic>
#include "data/address.h"

#if !_SERVER
class KServerBrowser
{
public:
	
	struct ServerQueryResult
	{
		KNetAddress Address;
		KString Name;
	};

	std::atomic<bool> bQueryingMaster = { false };
	TVector<ServerQueryResult> QueryResults;
	TVector<std::function<void()>> Callbacks;

	static KServerBrowser& Get();
	void QueryMasterServer();
	void AddCallback(std::function<void()> func);
};
#endif
