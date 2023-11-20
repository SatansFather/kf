#include "server.h"
#include "server_browser.h"
#include <thread>
#include "engine/console/engine_console.h"

#define HTTP_IMPLEMENTATION
#include "http.h"
#include "json.hpp"
using json = nlohmann::json;

#define MASTER_ADDRESS "http://162.248.92.183:6667/info"

#if !_SERVER
KString CCOM_Servers(const KString& val)
{
	GetEngineConsole()->bLastWasServers = true;
	KServerBrowser::Get().QueryMasterServer();
	return "";
}
#endif

void PostMasterUpdate(std::string str)
{
	http_t* poster = http_post(MASTER_ADDRESS, str.data(), str.size(), NULL);
	if (poster)
	{
		KTimePoint queryTime = KTime::Now();
		while (http_process(poster) == HTTP_STATUS_PENDING) 
		{
			KTime::SleepThreadMicroseconds(1000);
			if (KTime::Since(queryTime) > 3)
				break;
		}
		http_release(poster);
	}
	else
	{
		LOG("post invalid");
	}
}

void KNetInterface_Server::UpdateMasterServer()
{
	if (KTime::Since(LastMasterUpdateTime) < 15) return;

	json serverDetails =
	{
		{ "name", Config.Name },
		{ "port", KString(Config.Port) },
	};

	std::thread t(&PostMasterUpdate, serverDetails.dump());
	t.detach();

	LastMasterUpdateTime = KTime::Now();
}

#if !_SERVER

KServerBrowser& KServerBrowser::Get()
{
	static KServerBrowser instance;
	return instance;
}

void QueryMaster()
{
	KServerBrowser& browser = KServerBrowser::Get();

	if (browser.bQueryingMaster) return;

	browser.bQueryingMaster = true;
	browser.QueryResults.clear();

	http_t* getter = http_get(MASTER_ADDRESS, NULL);
	if (getter)
	{
		KTimePoint queryTime = KTime::Now();
		while (http_process(getter) == HTTP_STATUS_PENDING)
		{
			KTime::SleepThreadMicroseconds(1000);
			if (KTime::Since(queryTime) > 3)
				break;
		}
		if (getter->status == HTTP_STATUS_COMPLETED)
		{
			json servers = json::parse((char const*)getter->response_data);
			u32 index = 1;
			for (auto& [key, value] : servers.items())
			{
				LOG(KString(index) + ". " + KString(value["name"]) + " - " + KString(key));

				KServerBrowser::ServerQueryResult result;
				result.Name = KString(value["name"]);
				KNetAddress addr;
				addr.SetFromString(KString(key));
				result.Address = addr;
				browser.QueryResults.push_back(result);

				index++;
			}
		}
		http_release(getter);
	}

	for (auto& func : KServerBrowser::Get().Callbacks)
		func();
		
	KServerBrowser::Get().bQueryingMaster = false;
}

void KServerBrowser::QueryMasterServer()
{
	std::thread t(&QueryMaster);
	t.detach();
}

void KServerBrowser::AddCallback(std::function<void()> func)
{
	Callbacks.push_back(func);
}

#endif

#undef MASTER_ADDRESS