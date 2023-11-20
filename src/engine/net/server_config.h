#pragma once

#include "kfglobal.h"

class KServerConfig
{
	friend class KNetInterface_Server;

	KString Name = "Karnage Freak Server";
	u16 Port = 6668;
	void LoadConfig();
};
