#include "net_interface.h"
#include "enet/enet.h"
#include "data/host.h"
#include "../game_instance.h"
#include "player.h"

KNetInterface::~KNetInterface() {}

static bool bEnetInitialized = false;

KNetInterface* GetNetInterface()
{
	return KGameInstance::Get().GetNetInterface();
}

bool KNetInterface::InitEnet()
{
	if (!bEnetInitialized)
	{
		bEnetInitialized = enet_initialize() == 0;
		if (bEnetInitialized) atexit(enet_deinitialize);
	}
	return bEnetInitialized;
}

void KNetInterface::ProcessUpdates()
{
	if (Host) Host->QueryEvents(this);

	if (KTime::FrameCount() % 60 == 0)
	{
		NetStats.InBytes = InBytesThisSecond;
		NetStats.OutBytes = OutBytesThisSecond;
		InBytesThisSecond = 0;
		OutBytesThisSecond = 0;
	}
}

void KNetInterface::AddPlayerToScoreInfo(KNetPlayer* player, TVector<KPlayerScoreInfo>& info)
{
	KPlayerScoreInfo entry;
	entry.bIsMine = player == GetLocalNetPlayer();
	entry.Damage = player->Damage;
	entry.Deaths = player->Deaths;
	entry.Frags = player->Frags;
	entry.Score = player->Score;
	entry.PlayerName = player->PlayerName;
	entry.Ping = player->ClientPing;
	info.push_back(entry);
}

void KNetInterface::FlagPendingScoreUpdate()
{
	bPendingScoreUpdate = true;
}

void KNetInterface::AddInBytesThisSecond(u32 count)
{
	InBytesThisSecond += count;
}

void KNetInterface::AddOutBytesThisSecond(u32 count)
{
	OutBytesThisSecond += count;
}

#if !_SERVER
UPtr<u8[]> KNetInterface::GetLocalPlayerNameAsData(u8& size)
{
	LockConfig();
	GetUserConfig()->Net.PlayerName.TrimInPlace();
	KString name = GetUserConfig()->Net.PlayerName;
	UnlockConfig();

	if (name.Length() > 32) name = name.GetSubstring(0, 32);
	size = name.Size();
	UPtr<u8[]> sendData = std::make_unique<u8[]>(1 + size);
	memcpy(sendData.get(), &size, 1);
	memcpy(sendData.get() + 1, name.Get().data(), size);
	size++;

	return sendData;
}
#endif

