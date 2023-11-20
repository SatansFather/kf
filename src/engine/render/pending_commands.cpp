#if !_SERVER

#include "pending_commands.h"
#include "engine/utility/kstring.h"

KPendingCommands::KPendingCommands()
{
	ModeStrings =
	{
		{ "lit",			0 },
		{ "unlit",			1 },
		{ "wire",			2 },
		{ "wireframe",		2 },
		{ "norm",			3 },
		{ "normal",			3 },
		{ "normtex",		4 },
		{ "normaltex",		4 },
		{ "normaltexure",	4 },
		{ "normaltexured",	4 },
		{ "light",			5 },
		{ "lightmap",		5 },
		{ "lightmapped",	5 },
	};
}

KPendingCommands::~KPendingCommands() {}

void KPendingCommands::AddPendingCommand(EPendingCommandType comm)
{
	List |= (u64)comm;
}

void KPendingCommands::UpdateRenderModeFromString(const KString& str)
{	
	if (ModeStrings.contains(str))
	{
		RenderMode = ModeStrings[str];
		return;
	}

	u8 val = 0;
	str.ToU8Safe(val);
	RenderMode = val;
}

#endif
