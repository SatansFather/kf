#pragma once

#if !_SERVER

#include "kfglobal.h"

enum class ERenderMode : u8
{
	Lit, Unlit, Wireframe, Normal, NormalTex, Lightmapped
};

enum EPendingCommandType
{
	RenderMode					= 1,
	FOV							= 2,
	Resolution					= 4,
	Screenshot					= 8,
	ScaleFilter					= 16
};

struct KPendingCommands
{
	TMap<class KString, u8> ModeStrings;
	u8 RenderMode = 0;

	// flags for commands
	u64 List = 0;

	KPendingCommands();
	~KPendingCommands();

	void AddPendingCommand(EPendingCommandType comm);


	void UpdateRenderModeFromString(const KString& str);
};

#endif