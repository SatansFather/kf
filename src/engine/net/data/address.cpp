#include "address.h"

KNetAddress::KNetAddress(u32 addr, u16 port)
{
	Address.host = addr;
	Address.port = port;
}

bool KNetAddress::SetFromString(const KString& str)
{
	KString ipStr = str;

	TVector<KString> octets;
	octets.reserve(4);
	str.SplitByChar(octets, '.');
	if (octets.size() != 4) return false;

	TVector<KString> portVec;
	KString portStr = "6668";
	portVec.reserve(2);
	str.SplitByChar(portVec, ':');
	if (portVec.size() == 2)
	{
		portStr = portVec[1];
		ipStr = portVec[0];
	}



	//u32 ip = 0;
	u32 buffer = 0;
	for (u32 i = 0; i < 4; i++)
	{
		if (!octets[i].ToU32Safe(buffer) && buffer <= 255)
		//	ip |= buffer << (i * 8);
		//else
			return false;
	}

	//Address.host = ip;
	enet_address_set_host(&Address, ipStr.CStr());

	if (!portStr.ToU16Safe(Address.port))
		Address.port = 6668;

	return true;
}

KString KNetAddress::GetString(bool port /*= true*/) const
{
	u32 addr[4];

	addr[0] = (0x000000FF & Address.host);
	addr[1] = (0x0000FF00 & Address.host) >> 8;
	addr[2] = (0x00FF0000 & Address.host) >> 16;
	addr[3] = (0xFF000000 & Address.host) >> 24;

	KString ip =
		KString(addr[0]) + "."
		+ KString(addr[1]) + "."
		+ KString(addr[2]) + "."
		+ KString(addr[3]);

	if (port)
		ip += ":" + KString(GetPort());

	return ip;
}

/*

void KNetAddress::SetHost()
{
	enet_address_set_host(&Address, GetString().CStr());
}*/
