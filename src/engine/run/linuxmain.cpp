#ifdef __linux__

#include "k_main.h"

int main(int argc, char* argv[])
{
	KString cmd;
	if (argc > 1)
	  for (u32 i = 1; i < argc; i++)
		cmd += " " + KString(argv[i]);

	KApplication::PreMain();
	KApplication::KarnageMain(cmd);
	return 0; 
}

#endif
