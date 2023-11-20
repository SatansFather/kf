#pragma once


#include "enet/enet.h"
#include "kfglobal.h"

struct KNetAddress
{
	ENetAddress Address;

	KNetAddress() = default;
	KNetAddress(u32 addr, u16 port);

	u16 GetPort() const { return Address.port; }
	u32 GetAddress() const { return Address.host; }
	bool SetFromString(const KString& str);
	KString GetString(bool port = true) const;
	bool IsValid() const;
	//void SetHost();
};
