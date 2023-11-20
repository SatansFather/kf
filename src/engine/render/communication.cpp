#if !_SERVER

#include "communication.h"
#include "interface/render_interface.h"
#include "engine/utility/kstring.h"

// DELETE
#include "engine/system/terminal/terminal.h"
#include "scene.h"

#if !_SERVER && !_COMPILER
KString CCOM_RMode(const KString& val)
{
	KRenderBridge::Get().LockAccess();
	KPendingCommands& pending = KRenderBridge::Get().GetPendingCommands();
	pending.AddPendingCommand(EPendingCommandType::RenderMode);
	pending.UpdateRenderModeFromString(val);
	KRenderBridge::Get().UnlockAccess();
	return "";
};

KString CCOM_Screenshot(const KString& val)
{
	KRenderBridge::Get().LockAccess();
	KPendingCommands& pending = KRenderBridge::Get().GetPendingCommands();
	pending.AddPendingCommand(EPendingCommandType::Screenshot);
	KRenderBridge::Get().UnlockAccess();
	return "";
};
#endif

KRenderBridge::KRenderBridge() 
{
	//memset(CharacterInfo.PowerupFramesRemaining, 0, sizeof(u16) * EPowerupID::NumPowerups);
}

KRenderBridge::~KRenderBridge() {}

bool KRenderBridge::GetNewResources()
{

	return false;
}

void KRenderBridge::ProcessCommands()
{
	if (PendingCommands.List == 0) return;
	
	for (u64 i = 1; i <= MAX_I64; i <<= 1ULL)
	  if (PendingCommands.List & i)
		GetRenderInterface()->ProcessCommand(i);

	PendingCommands.List = 0;
}

#endif
