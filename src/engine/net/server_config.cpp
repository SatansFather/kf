#include "server_config.h"
#include "../global/paths.h"
#include <fstream>

void KServerConfig::LoadConfig()
{
	std::ifstream file(CFG_DIR + "serverconfig.cfg");
	std::string l;
	while (std::getline(file, l))
	{
		KString line(l);
		TVector<KString> entry;
		line.SplitByChar(entry, ' ', 1);

		if (entry.size() == 2)
		{
			KString key = entry[0].ToLower().Trim();
			KString value = entry[1].Trim();

			if (key == "name")
			{
				Name = value;
			}
			else if (key == "port")
			{
				u16 port = 6668;
				value.ToU16Safe(port);
				Port = port;
			}
		}
	}
}
