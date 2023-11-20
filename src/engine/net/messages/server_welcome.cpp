#include "server_welcome.h"
#include "../data/peer.h"
#include "../data/packet.h"
#include "../../game/match.h"
#include "../../game_instance.h"
#include "../player.h"
#include "../net_interface.h"
#include "../state.h"

void KServerWelcome::PrepareAndSend(KNetPeer* peer)
{
	if (KGameMatch* match = GetGameMatch())
	{
		KLocalMap& map = match->GetCurrentMapInfo();
		KServerWelcome w;

		for (u8 i = 0; i < 16; i++)
			w.MapHash[i] = map.Hash[i];

		w.NetPlayerIndex = peer->GetPlayer()->OwningPlayerIndex;
		w.NetPlayerID = peer->GetPlayer()->NetID;

		KNetPacket packet((u8*)&w, sizeof(KServerWelcome), EMessageType::S_Welcome);
		peer->SendPacket(&packet);
	}
}

bool KServerWelcome::ReceiveAndProcess(const KServerWelcome& data)
{
	// check if we have a map with this hash
	KMapHash hash;
	std::copy(std::begin(data.MapHash), std::end(data.MapHash), std::begin(hash));

	KGameInstance& inst = KGameInstance::Get();
	if (!inst.HasMapWithHash(hash))
	{
		// we dont have the map
		inst.bProcessingServerWelcome = false;
		LOG("Could not find map with matching hash", 1, 0, 0);
		KGameInstance::Get().NetDisconnect();
		return false;
	}

	inst.bProcessingServerWelcome = true;
	GetLocalPlayer()->NetPlayerIndex = data.NetPlayerIndex;
	KNetPlayer* p = GetNetInterface()->CreatePlayerAtIndex(data.NetPlayerIndex);
	//p->ViewedPlayerIndex = data.NetPlayerIndex;
	p->ViewedPlayerIndex = NULL_PLAYER;//data.NetPlayerIndex;
	GetLocalPlayer()->LocalNetPlayer = GetNetInterface()->GetPlayerFromIndex(data.NetPlayerIndex);
	GetLocalPlayer()->LocalNetPlayer->NetID = data.NetPlayerID;

	inst.LoadMap(inst.GetLocalMapByHash(hash).MapFileName);

	// we should have a net state now
	// TODO should players even be in net state? they persist longer than it
	// only benefit is that it allows them to be created and updated easily
	//GetNetState()->AddReplicatedObject(GetLocalPlayer()->LocalNetPlayer);
	inst.bProcessingServerWelcome = false;
	return true;
}
