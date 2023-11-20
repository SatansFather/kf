#include "game_instance.h"
#include "utility/k_assert.h"
#include "render/communication.h"
#include "render/interface/window.h"
#include "input/input_processor.h"
#include "run/k_main.h"
#include "compiler/compiler.h"
#include "compiler/map_file.h"
#include "global/paths.h"
#include <filesystem>
#include "engine/menus/basic_menu.h"
#include "net/player.h"
#include "kwad/kwad_chunk_map.h"
#include "kwad/kwad_file.h"

#if !_COMPILER
#include "game/match.h"
#include "net/server.h"
#include "net/client.h"
#include "net/data/address.h"
#endif

#if !_SERVER && !_COMPILER
#include "console/engine_console.h"
#include "console/chat_console.h"
#include "input/listener_game.h"
#include "input/listener_menu.h"
#include "input/listener_console.h"
#endif

#if !_NOSOUND
#include "audio/audio.h"
#endif
#include "net/server_browser.h"
#include "net/game_state.h"
#include "net/state.h"

bool IsNetAuthority()
{
#if _SERVER
	return true;
#else
	return !KGameInstance::Get().GetNetInterface() || KGameInstance::Get().GetNetInterface()->IsServer();
#endif
}
bool IsNetClient()
{
#if _SERVER
	return false;
#else
	return KGameInstance::Get().GetNetInterface() && KGameInstance::Get().GetNetInterface()->IsClient();
#endif
}
bool IsNetServer()
{
#if _SERVER
	return true;
#else
	return KGameInstance::Get().GetNetInterface() && KGameInstance::Get().GetNetInterface()->IsServer();
#endif
}

#if !_SERVER && !_COMPILER
KString CCOM_Replays(const KString& val)
{		
	KGameInstance::Get().ListReplays();
	return "";
}

KString CCOM_Replay(const KString& val)
{
	TMap<u32, KString>& replays = KGameInstance::Get().LastListedReplays;
	u32 index = 0;
	if (val.ToU32Safe(index))
	  if (replays.contains(index))
	    KGameInstance::Get().LoadReplay(replays[index]);

	return "";
}

KString CCOM_Exit(const KString& val)
{
	KGameInstance::Get().ExitGame();
	return "";
};

KString CCOM_Time(const KString& val)
{
	KGameInstance& inst = KGameInstance::Get();

	f32 t = 1;
	if (!val.ToFloatSafe(t)) return inst.GameTimeDilation;
	t = std::clamp(t, 0.f, 100.f);

	bool wasZero = inst.GameTimeDilation == 0 && t != 0;
	inst.GameTimeDilation = t;

	if (wasZero && !inst.IsPaused())
	{
		// reset accumulator
		inst.AddPause();
		inst.RemovePause();
	}

	return "";
}

KString CCOM_Connect(const KString& val)
{
	KNetAddress addr;
	
	if (GetEngineConsole()->bLastWasServers)
	{
		KServerBrowser& browser = KServerBrowser::Get();
		
		u32 index = MAX_U32;
		val.ToU32Safe(index);
		if (index > 0)
		{
			index --;	
			if (index < browser.QueryResults.size())
			{
				KGameInstance::Get().ConnectToServer(browser.QueryResults[index].Address);
				return "";
			}
		}
	}

	LockConfig();
	KUserConfig* cfg = GetUserConfig();
	bool fav = false;
	if (cfg->Net.FavoriteServers.contains(val))
	{
		if (addr.SetFromString(cfg->Net.FavoriteServers[val]))
			fav = true;
	}
	UnlockConfig();

	if (fav)
	{
		KGameInstance::Get().ConnectToServer(addr);
		return "";
	}

	if (val.Get() == "local")
	{
		addr.SetFromString(KString("127.0.0.1"));
	}
	else if (val.Get() == "")
	{
		KString last = GetLocalPlayer()->GetConfig()->Net.LastConnectedIP;
		addr.SetFromString(last);
	}
	else
		addr.SetFromString(val);

	KGameInstance::Get().ConnectToServer(addr);
	return "";
}

KString CCOM_Host(const KString& val)
{
	u16 port = 6668;
	val.ToU16Safe(port);
	KGameInstance::Get().BeginNetHost(port);
	return "";
}

KString CCOM_Map(const KString& val)
{
	if (IsNetClient()) KGameInstance::Get().NetDisconnect();
	KGameInstance::Get().LoadMap(val);
	return "";
};

KString CCOM_Maps(const KString& val)
{
	KGameInstance::Get().PrintMaps();
	return "";
}

KString CCOM_DrawStats(const KString& val)
{
	i32 v = 0;
	val.ToIntSafe(v);
	KRenderBridge::Get().bDrawStats = v != 0;
	return "";
}

#endif

KGameInstance& KGameInstance::KGameInstance::Get()
{
	static KGameInstance instance;
	return instance;
}

void KGameInstance::ProcessPendingConnection()
{
#if !_SERVER
	if (bPendingServerConnect)
	{
		KNetAddress address(PendingAddress, PendingPort);
		bPendingServerConnect = false;
		PendingAddress = 0;
		PendingPort = 0;

		if (PendingClient) return;

		// dont leave current match until connection is verified
		PendingClient = std::make_unique<KNetInterface_Client>();
		std::thread t(&KNetInterface_Client::AttemptConnection, PendingClient.get(), address);
		t.detach();
	}
#endif
}

KGameInstance::~KGameInstance() {}

void KGameInstance::RunGame()
{
	K_ASSERT(!bHasStarted, "Tried starting the game instance again!");

	Initialize();

	UpdateLocalMaps();

#if !_SERVER && !_COMPILER
#if _WIN32
	KApplication::ClearSplashScreen();
#endif
	GetRenderInterface()->GetGameWindow()->Show();
#endif

#if _SERVER
	BeginNetHost();
	MainLoop();
#elif _PACK

	// add stock maps
	MainWadFile.AddMap("violence");
	MainWadFile.AddMap("kastle");
	MainWadFile.AddMap("aerowalk");
	MainWadFile.AddMap("emptymap");
	MainWadFile.AddMap("mainmenu");

	_SYSLOG("Writing kwad");
	MainWadFile.WriteFile("kfdata");
	_SYSLOG("Finished writing kwad");

	if (KApplication::HasCommandLineArg("newconsole"))
		system("PAUSE");
#else
	StartGameThread();

	// program holds here until exit
	InputLoop();
#endif

	Shutdown();
}

f64 KGameInstance::GetTimeDilation()
{
	f64 game = GameTimeDilation;
	if (NetInterface && NetInterface->IsClient()) game = 1;
	return game * (NetInterface ? NetInterface->GetTimeDilation() : 1);
}

#if !_SERVER && !_COMPILER
class KInputListener* KGameInstance::GetPrimaryListener()
{
	//return GameInputListener.get();
	return GetGameInput();
}

#endif

void KGameInstance::ExitGame()
{
	bIsRunning = false;
}

void KGameInstance::LoadMap(const KString& map)
{
#if !_COMPILER
	KMapFile file;
	if (!file.ReadFromDisk(map, true))
		return;

#if _DEV
	bool log = true;
#else
	bool log = map.Get() != "emptymap" && map.Get() != "mainmenu";
#endif

#if !_SERVER
	if (IsMainMenu())
		KBasicMenu::CloseAllMenus();
#endif

	bIsMainMenu = map.Get() == "mainmenu";
	bIsEmptyMap = map.Get() == "emptymap";

	if (log) LOG("-------------------------------", 1, 1, 0);
	if (log) LOG("Loading map: " + file.MapName);

#if !_NOSOUND
	KAudio::StopAllSounds();
#endif

#if !_SERVER
	KRenderBridge::Get().MapLoadProgress = 0;

	KRenderInterface* iface = GetRenderInterface();
	bLoadingMap = true;
	
	bool showingConsole = GetEngineConsole()->IsShowing();
	GetEngineConsole()->SetShowing(false);
#endif

	bool currentMap = false;
	//if (GetMatch())
	//	currentMap = file.HashMatch(file.DataHash, GetMatch()->GetCurrentMapInfo().Hash.data());

#if !_SERVER
	// tell the renderer we want to load a new map so we can reset the scene
	//if (!currentMap)
	{
		KRenderBridge& bridge = KRenderBridge::Get();
		bridge.LockAccess();
		bridge.bRenderAcknowledgedMapChange = false;
		bridge.bLoadingMap = true;
		bridge.bResettingMap = currentMap;
		bridge.LoadingMapName = file.MapName;
		bridge.bIsMainMenu = bIsMainMenu || bIsEmptyMap;
		bridge.UnlockAccess();

		// wait for render thread finish its frame so we can destroy the scene data
		while (!bridge.bRenderAcknowledgedMapChange) {}
	}
#endif

	KTimePoint startTime = KTime::Now();

	std::thread serverQueryThread;

	if (currentMap) 
	{
		CurrentMatch->ResetMatch();
	}
	else
	{
		if (KNetInterface_Server* server = dynamic_cast<KNetInterface_Server*>(GetNetInterface()))
			server->NotifyClientsMapChange(file.DataHash);
#if !_SERVER
		/*else if (KNetInterface_Client* client = dynamic_cast<KNetInterface_Client*>(GetNetInterface()))
		{
			if (!bProcessingServerWelcome)
				client->ClearConnectedPlayers();
		}*/
#endif

		if (NetInterface)
		{
			const auto query = [this]() -> void
			{
				while (bRunningMapLoadNetQueries && NetInterface) 
					NetInterface->ProcessUpdates();
			};
			bRunningMapLoadNetQueries = true;
			serverQueryThread = std::thread(query);
		}

		// destroys collision and entities
		bIsDestroyingMatch = true;
		CurrentMatch.reset();
		bIsDestroyingMatch = false;

#if !_SERVER
		KRenderBridge::Get().MapLoadProgress = 1.f / 16.f;

		// resets skybox and world, creates new
		iface->AllocateScenes();
		KRenderBridge::Get().MapLoadProgress = 2.f / 16.f;
#endif
	}


	if (!currentMap)
	{
		CurrentMatch = std::make_unique<KGameMatch>();

		CurrentMatch->CurrentMapInfo.MapFileName = map;
		CurrentMatch->CurrentMapInfo.MapIngameName = file.MapName;
		std::copy(std::begin(file.DataHash), std::end(file.DataHash), std::begin(CurrentMatch->CurrentMapInfo.Hash));

		if (!currentMap) file.ReadFromDisk(map);

		f64 loadTime = KTime::Since(startTime);
		if (log) LOG("Map loaded in " + KString(loadTime, 2) + (loadTime == 1 ? " second" : " seconds"));
		if (log) LOG("-------------------------------", 1, 1, 0);
	}
	else
	{
		f64 loadTime = KTime::Since(startTime);
		if (log) LOG("Map reset in " + KString(loadTime, 2) + (loadTime == 1 ? " second" : " seconds"));
		if (log) LOG("-------------------------------", 1, 1, 0);
	}

	LastTeleportTime = -10;
	GetLocalPlayer()->CharacterInfo.Health = 0;

	CurrentMatch->FinishedLoadingMap();

	if (bRunningMapLoadNetQueries)
	{
		bRunningMapLoadNetQueries = false;
		serverQueryThread.join();
	}

	bLoadingMap = false;

#if !_SERVER


	// let the renderer know the map is ready
	//bridge.LockAccess();
	//bridge.bLoadingMap = false;
	//bridge.UnlockAccess();

	//GetDropConsole()->CloseConsole();
	//GetDropConsole()->SetShowing(showingConsole);

#endif

#endif
}

void KGameInstance::LoadMap(const KMapHash& hash)
{
	LoadMap(GetLocalMapByHash(hash).MapFileName);
}

void KGameInstance::LoadReplay(const KString& replay)
{
	NetDisconnect();
	PendingReplayName = replay;
	LoadMap("aerowalk");
	PendingReplayName = "";
}

void KGameInstance::CreateReplayNetInterface()
{
#if !_SERVER
	NetDisconnect();
	NetInterface = std::make_unique<KNetInterface_Client>();
	GetNetInterface()->CreatePlayerAtIndex(255);
	GetLocalPlayer()->LocalNetPlayer = GetNetInterface()->GetPlayerFromIndex(255);
	GetLocalPlayer()->LocalNetPlayer->NetID = 0xFFFFFFFF;
#endif
}

void KGameInstance::ListReplays()
{
	LastListedReplays.clear();

	if (std::filesystem::exists(KString(REPLAY_DIR).Get()))
	{
		u32 index = 1;
		for (const auto& entry : std::filesystem::directory_iterator(KString(REPLAY_DIR).Get()))
		{
			KString fileName = entry.path().string();
			if (!fileName.EndsWith(".kfdemo")) continue;

			fileName = fileName.Get().substr(0, fileName.Get().find_last_of("."));
			fileName = fileName.Get().substr(fileName.Get().find_last_of("\\/") + 1, fileName.Get().npos);
			LastListedReplays[index] = fileName;
			LOG(KString(index) + ". " + fileName);
			index++;
		}
	}
}

void KGameInstance::Initialize()
{
	bHasStarted = true;
	bIsRunning = true;

#if !_DEV
	LoadMainWad();
#endif

#if !_SERVER && !_COMPILER
	LastResizeTime = KTime::Now();
	LocalPlayer.LoadConfig("config");

	InitMenus();
#endif

#if !_NOSOUND
	KAudio::Init();
#endif

#if !_SERVER
	CreateRenderInterface();
#endif

#if !_SERVER && !_COMPILER
	KInputProcessor::InitInput();
	KInputProcessor::SetListener(GetGameInput());
#endif

#if !_COMPILER
	CreateMemPools();
#endif
}

void KGameInstance::CreateRenderInterface()
{
#if !_SERVER
	RenderInterface = KRenderInterface::CreateD3D11();
	RenderInterface->InitializeInterface();

	// InitializeInterface spawns the render thread 
	// wait for renderer tell us that thread has finished init
	while (!RenderInterface->IsInitialized()) {}
#endif
}

void KGameInstance::StartGameThread()
{
	GameThread = std::thread(&KGameInstance::MainLoop, this);
}

void KGameInstance::UpdateRenderBridge(bool paused /*= false*/)
{
#if !_SERVER && !_COMPILER

	// update render info

	KRenderBridge& bridge = KRenderBridge::Get();

	KNetInterface* netInterface = GetNetInterface();

	// TODO this loop will hang and eventually freeze if hosting wtf
	if (bridge.LastFrameTime * 1000000 < GetMatch()->GetTimestep())
		while (!bridge.bRenderTookFrame && bIsRunning) {}

	bridge.LockAccess();

	TotalRenderTime = bridge.TotalRenderTime + GameFrameDelta();
	bridge.bIsMainMenu = bIsMainMenu || bIsEmptyMap;
	KGameMatch* match = GetMatch();

	// TEMP
	bridge.WeaponSwitchAlpha = GetLocalPlayer()->WeaponRenderInfo.GetSwitchAlpha();

	if (netInterface) bridge.NetStats = GetNetInterface()->NetStats;

	if (bridge.bLoadingMap)
	{
		LocalPlayer.bTeleportedThisFrame = true;
		bridge.bLoadingMap = false;
	}

	LocalPlayer.CameraPosition += LocalPlayer.CameraCorrectionOffset;

	//bridge.bNewFrameReady = true;
	//bridge.LastGameUpdateTime = KTime::Now();
	bridge.LastCameraPosition = (LocalPlayer.bTeleportedThisFrame ? 
		(LocalPlayer.CameraPosition).ToGLM() : bridge.CameraPosition);

	bridge.CameraPosition = LocalPlayer.CameraPosition.ToGLM();

	bridge.LastCameraRoll = (LocalPlayer.bTeleportedThisFrame ?
		LocalPlayer.CameraRoll : bridge.CameraRoll);
	bridge.CameraRoll = LocalPlayer.CameraRoll;

	LocalPlayer.bTeleportedThisFrame = false;

	bridge.CameraPush = LocalPlayer.CameraPush * GetUserConfig()->Game.ShakeScale;
	bridge.CameraPush.Concussion *= GetUserConfig()->Game.ConcussionScale;
	LocalPlayer.CameraPush.Reset();

	if (LocalPlayer.bPendingPitch)
	{
		bridge.bPendingPitch = true;
		bridge.PendingPitch = LocalPlayer.PendingPitch;
	}
	if (LocalPlayer.bPendingYaw)
	{
		bridge.bPendingYaw = true;
		bridge.PendingYaw = LocalPlayer.PendingYaw;
	}

	bridge.bShowScoreboard = GetGameInput()->WantsScoreboard() && KInputProcessor::GetActiveListener() == GetGameInput();


	bridge.LastTeleportTime = LastTeleportTime;

	LocalPlayer.bPendingPitch = false;
	LocalPlayer.bPendingYaw = false;

	// TODO put non-game stuff in different function
	// call it if paused
	bridge.GameTimeDilation = GetTimeDilation() * PerformanceTimeDilation * KSaturate(!paused);
	bridge.LastResizeTime = LastResizeTime;

	bridge.TargetPlayerName = LocalPlayer.TargetPlayerName;

	if (match)
	{
		for (u32 id : match->PendingPrecipitationIDs)
		  if (match->Precipitation.contains(id))
			if (auto p = match->Precipitation[id].get())
			  bridge.PendingPrecipitation.push_back(*p);

		match->PendingPrecipitationIDs.clear();

		bridge.LiveDamageNumbers = match->LiveDamageNumbers;

		if (match->CurrentFragMessage.bUpdated)
			bridge.CurrentFragMessage = match->CurrentFragMessage;

		if (match->CurrentDeathMessage.bUpdated)
			bridge.CurrentDeathMessage = match->CurrentDeathMessage;

		match->CurrentFragMessage.bUpdated = false;
		match->CurrentDeathMessage.bUpdated = false;

		bridge.NewKillFeedMessages = match->NewKillFeedMessages;
		match->NewKillFeedMessages.clear();

		bridge.OnScreenMessages.insert(bridge.OnScreenMessages.end(), match->PendingOnScreenMessages.begin(), match->PendingOnScreenMessages.end());
		match->PendingOnScreenMessages.clear();

		bridge.PendingPickupMessage = match->PendingPickupMessage;
		match->PendingPickupMessage.Text = "";
	}

	bool powerup = false;
	bool weapon = false;
	GetLocalPlayer()->CharacterInfo.ShowFlags = 0;
	if (GetLocalPlayer()->ControlledEntity.IsValid())
	{
		bridge.UnderWaterDepth = GetLocalPlayer()->WaterDepth;
		
		if (KEntProp_PowerupInventory* inv = GetLocalPlayer()->ControlledEntity.As<KEntProp_PowerupInventory>())
		{
			powerup = true;
			u16 frames[EPowerupID::NumPowerups];
			inv->GetPowerupFrameCountArray(frames);
			memcpy(GetLocalPlayer()->CharacterInfo.PowerupFramesRemaining, frames, EPowerupID::NumPowerups * sizeof(u16));
			GetLocalPlayer()->CharacterInfo.ShowFlags |= KCharacterInfo::SF_Powerups;
		}
		if (KEntProp_Killable* kill = GetLocalPlayer()->ControlledEntity.As<KEntProp_Killable>())
		{
			GetLocalPlayer()->CharacterInfo.Health = kill->GetHealth();
			GetLocalPlayer()->CharacterInfo.ShowFlags |= KCharacterInfo::SF_Health;
		}
		if (KEntProp_WeaponInventory* inv = GetLocalPlayer()->ControlledEntity.As<KEntProp_WeaponInventory>())
		{
			weapon = true;
			KCharacterWeaponInfo wep[EWeaponID::NumWeapons];
			inv->GetWeaponInfoArray(wep);
			memcpy(GetLocalPlayer()->CharacterInfo.Weapons, wep, EWeaponID::NumWeapons * sizeof(KCharacterWeaponInfo));
			GetLocalPlayer()->CharacterInfo.ShowFlags |= KCharacterInfo::SF_Weapons;
		}

		if (KEntity_Character* guy = GetLocalPlayer()->ControlledEntity.As<KEntity_Character>())
			GetLocalPlayer()->CharacterInfo.DamageMultiplier = guy->DamageMultiplier;
		else
			GetLocalPlayer()->CharacterInfo.DamageMultiplier = 1;
	}
	bridge.CharacterInfo.CopyFrom(GetLocalPlayer()->CharacterInfo);
	if (!powerup)
	{
		memset(bridge.CharacterInfo.PowerupFramesRemaining, 0, EPowerupID::NumPowerups * sizeof(u16));
	}
	if (!weapon)
	{
		memset(bridge.CharacterInfo.Weapons, 0, EWeaponID::NumWeapons * sizeof(KCharacterWeaponInfo));		
	}

	if (PendingFog.size() > 0)
	{
		bridge.PendingFog = PendingFog;
		PendingFog.clear();
	}

	KTimePoint start = KTime::Now();
	KRenderBufferInfo::CopyMap(&RenderBufferMap, &bridge.RenderBufferMap, true);
	bridge.LastCopyTime = KTime::Since(start);
	bridge.LastNetTime = LastNetTime;
	bridge.LastTickTime = LastTickTime;
	bridge.LastRenderUpdateTime = LastRenderUpdateTime;

	// TODO dont create this every frame

	if (netInterface && netInterface->bPendingScoreUpdate) 
	{
		GetNetInterface()->PackPlayerInfo(bridge.PlayerInfo);

		std::sort
		(
			bridge.PlayerInfo.begin(),
			bridge.PlayerInfo.end(),
			[&](const KPlayerScoreInfo& a, const KPlayerScoreInfo& b) -> bool
			{
				if (a.Score > b.Score) return true;
				else if (a.Score == b.Score)
				{
					if (a.Frags > b.Frags) return true;
					else if (a.Frags == b.Frags)
					{
						if (a.Deaths < b.Deaths) return true;
						else if (a.Deaths == b.Deaths)
						{
							return a.Damage > b.Damage;
						}
					}
				}
				return false;
			}
		);
		
		netInterface->bPendingScoreUpdate = false;
	}

	bridge.GameFrameCount = KTime::FrameCount();

	bridge.UnlockAccess();

	GetLocalPlayer()->CharacterInfo.SetNewCharacter(false);

#endif
}

void KGameInstance::MainLoop()
{
	f64 loadTime = KTime::SinceInit();
	LOG("Engine initialization took " + KString(loadTime, 2) + (loadTime == 1 ? " second" : " seconds"));

#if _DEV || _SERVER
	LoadMap("aerowalk");
#else
	LoadMap("mainmenu");
#endif

	while (bIsRunning)
	{
#if !_COMPILER && !_SERVER
		RunPendingConsoleCommands();
		CheckPendingClientConnection();
		
		if (!bIsRunning) break;
#endif
	
		// server constantly checks for client input rather than once per frame
		// we need to know the exact time we received input so we can help clients adjust timing
		NetServerReceiveData();
#if !_SERVER
		if (NetInterface && (!NetInterface->IsWelcomed() || NetInterface->HasPendingServerMap()))
		{
			// this function wont be called inside the game loop if we're awaiting server welcome
			// risks destroying the match while running functions inside it otherwise
			UpdateNetClient(false);
		}
#endif
		if (CurrentMatch && CurrentMatch->RunLoop())
		{
			// game was ticked, update stuff
			NetServerSendFrame();
			UpdateRenderBridge();
			LastNetTime = 0;
#if !_NOSOUND
			KAudio::UpdateAudio();
#endif
#if !_SERVER
			if (IsMainMenu() && KInputProcessor::GetActiveListener() == GetGameInput())
				GetMainMenu()->OpenMenu();
#endif
		}
		else if (IsPaused() || GetTimeDilation() == 0)
		{
			UpdateRenderBridge(true);
		}
	}
}

void KGameInstance::InputLoop()
{
#if !_COMPILER && !_SERVER
	while (bIsRunning && KInputProcessor::WaitInput()) {}
	ExitGame();
#endif
}

void KGameInstance::CompileMap()
{
#if _COMPILER
	//K_ASSERT(KApplication::HasCommandLineArg("map"), "no map argument was passed on the command line");
	KMapCompiler::Get().CompileMap(KApplication::GetCommandLineArg("map"));
#endif
}

void KGameInstance::Shutdown()
{
#if !_SERVER && !_COMPILER
	if (KRenderInterface* iface = GetRenderInterface())
		iface->GetGameWindow()->Hide();

	KAudio::StopAllSounds();
#endif

	NetDisconnect();

	// kill renderer here
	// allowing the program exit to do it will cause a crash for opengl since glfw was destroyed
#if !_SERVER
	if (RenderInterface) 
	{
		RenderInterface->CallInterfaceDestruction();
		RenderInterface.reset();
	}
#endif

#if !_COMPILER
	bIsDestroyingMatch = true;
	CurrentMatch.reset();
	bIsDestroyingMatch = false;
	GameThread.join();
#endif

#if !_NOSOUND
	KAudio::Shutdown();
#endif
}

void KGameInstance::UpdateNetClient(bool requireWelcome)
{
#if !_SERVER
	if (KNetInterface_Client* c = dynamic_cast<KNetInterface_Client*>(NetInterface.get()))
	{
		if (GetGameMatch() && GetGameMatch()->ReplayReader.IsPlaying())
		{
			if (requireWelcome)
				GetGameMatch()->ReplayReader.AdvanceFrame();
			return;
		}

		if (requireWelcome && !c->IsWelcomed()) return;
		
		if (!requireWelcome)
			c->FollowServerMap();

		KTimePoint start = KTime::Now();
		c->ProcessUpdates();
		c->AdjustTiming();
		c->SendFrame();
		LastNetTime = KTime::Since(start);
	}
#endif
}

void KGameInstance::NetServerSendFrame()
{
	if (NetInterface && NetInterface->IsServer())
	{
		KTimePoint start = KTime::Now();
		NetInterface->SendFrame();
		LastNetTime += KTime::Since(start);
	}
#if !_SERVER
	else if (KNetInterface_Client* client = dynamic_cast<KNetInterface_Client*>(NetInterface.get()))
	{
		if (KReplayWriter* r = KReplayWriter::GetActive())
		{
			if (r->IsWriting())
			{
				// create snapshots
				//TDataPool<KNetPlayer>::GetPool()->CreateSnapshots();
				for (auto& kv : client->ConnectedPlayers)
					kv.second->CreateSnapshot();
				for (auto pool : SnapshotDataPools)
				  for (KPoolable* p : *pool)
				    dynamic_cast<KSnapshottable*>(p)->CreateSnapshot();

				KGameStateSnapshot snapshot(KTime::FrameCount() - 1, r->GetBufferPointer(), GetLocalNetPlayer(), true);
				GetNetState()->DestroyedNetItems.clear();
			}
		}
	}
#endif
}

void KGameInstance::NetServerReceiveData()
{
	if (NetInterface && NetInterface->IsServer())
	{
		KTimePoint start = KTime::Now();
		NetInterface->ProcessUpdates();
		NetInterface->UpdateMasterServer();
		LastNetTime += KTime::Since(start);
	}
}

void KGameInstance::CheckPendingClientConnection()
{
#if !_SERVER
	if (PendingClient)
	{
		if (PendingClient->HasConnectionResult())
		{
			if (PendingClient->WasConnectionSuccessful())
			{
				NetDisconnect();
				NetInterface = std::move(PendingClient);
				NetInterface->OnConnection(nullptr);
			}
			else
			{
				LOG("Failed to connect to " + PendingClient->GetServerIP(), 1, 0, 0);
			}
			PendingClient.reset();
		}
	}
	else
	{
		ProcessPendingConnection();
	}
#endif
}

void KGameInstance::UpdateLocalMaps()
{
#if !_COMPILER
	LocalMaps.clear();

	const auto indexMap = [&](const KString& fileName) -> void
	{
		KMapFile file;
		file.ReadFromDisk(fileName, true);

		KMapHash hash;
		std::copy(std::begin(file.DataHash), std::end(file.DataHash), std::begin(hash));

		// might already have this from a wad
		if (LocalMaps.contains(hash)) return;

		KLocalMap map;
		map.Hash = hash;
		map.MapFileName = fileName;
		map.MapIngameName = file.MapName;
		LocalMaps[hash] = map;
	};

	// check wads first
	auto& maps = MainWadFile.IndexedData[EWadFileChunkType::Map];
	for (auto& kv : maps) indexMap(kv.first);

	// iterate map directory
	if (std::filesystem::exists(KString(MAP_DIR).Get()))
	{
		for (const auto& entry : std::filesystem::directory_iterator(KString(MAP_DIR).Get()))
		{
			KString fileName = entry.path().string();
			if (!fileName.EndsWith(".kmap")) continue;

			fileName = fileName.Get().substr(0, fileName.Get().find_last_of("."));
			fileName = fileName.Get().substr(fileName.Get().find_last_of("\\/") + 1, fileName.Get().npos);
			indexMap(fileName);
		}
	}

	u32 size = LocalMaps.size();
	LOG("Local kmap files indexed - " + KString(size) + " map" + (size == 1 ? "" : "s") + " available");

#if !_SERVER
	InitMapsMenu();
#endif
#endif
}

void KGameInstance::RunPendingConsoleCommands()
{	
#if !_SERVER && !_COMPILER
	// copy commands so we dont hold up the input thread
	ConsoleCommandMutex.lock();
	TVector<KPendingConsoleCommand> comms = PendingConsoleCommands;
	PendingConsoleCommands.clear();
	ConsoleCommandMutex.unlock();

	for (KPendingConsoleCommand& comm : comms)
	{
		KString out = comm.Function(comm.Value);
		if (!out.IsEmpty()) GetEngineConsole()->AddMessage(out);
	}
#endif
}

#if !_DEV
void KGameInstance::LoadMainWad()
{
	MainWadFile.LoadWadFile("kfdata");
}
#endif

//class KGameInputListener* KGameInstance::GetGameInputListener()
//{
//	return GameInputListener.get();
//}

void KGameInstance::BeginNetHost(u16 port)
{
	NetDisconnect();
	NetInterface = std::make_unique<KNetInterface_Server>(port);
}

void KGameInstance::ConnectToServer(KNetAddress address)
{
	bPendingServerConnect = true;
	PendingAddress = address.GetAddress();
	PendingPort = address.GetPort();
}

void KGameInstance::NetDisconnect()
{
	if (NetInterface)
	{
		if (bIsRunning) LoadMap("emptymap");

		NetInterface.reset();
	}
}

class KNetInterface* KGameInstance::GetNetInterface()
{
	return NetInterface.get();
}

void KGameInstance::FinalizeClientConnection()
{
#if !_SERVER
	NetDisconnect();
	NetInterface = std::move(PendingClient);
	PendingClient.reset();
#endif
}

bool KGameInstance::HasMapWithHash(KMapHash hash) const
{
	return LocalMaps.contains(hash);
}

bool KGameInstance::HasMapWithFileName(const KString& name) const
{
	for (const auto& kv : LocalMaps)
	  if (kv.second.MapFileName == name) 
		return true;

	return false;
}

bool KGameInstance::HasMapWithIngameName(const KString& name) const
{
	for (const auto& kv : LocalMaps)
	  if (kv.second.MapIngameName == name)
		return true;

	return false;
}

KLocalMap KGameInstance::GetLocalMapByHash(KMapHash hash)
{
	KLocalMap map;
	if (HasMapWithHash(hash))
		map = LocalMaps[hash];

	return map;
}

void KGameInstance::SaveGame(const KString& name)
{
	
}

void KGameInstance::LoadGame(const KString& name)
{
	
}

#if !_SERVER
void KGameInstance::UpdateLastResizeTime()
{
	LastResizeTime = KTime::Now();
}

void KGameInstance::InitMenus()
{
	InitMainMenu();
	InitIngameMenu();
	InitMapsMenu(); // called again in UpdateLocalMaps
	InitServerBrowser();
	InitJoinMenu();
	InitOptionsMenu();
	InitVideoOptionsMenu();
	InitAudioOptionsMenu();
	InitGameOptionsMenu();
	InitMouseOptionsMenu();
	InitControlOptionsMenu();
	InitInterfaceOptionsMenu();
	InitMiscOptionsMenu();
}

#if !_COMPILER
void KGameInstance::SendChatMessage(const KString& msg)
{
	if (!NetInterface)
		ReceiveChatMessage(msg);
	else
	{
		KString message = msg;
#if !_SERVER
		if (IsNetServer())
		  if (KNetPlayer* p = GetLocalNetPlayer())
			message = p->PlayerName + ": " + msg;
#endif
		NetInterface->SendChatMessage(message);
	}
}

void KGameInstance::ReceiveChatMessage(const KString& msg, u8 messageType)
{
	f32 r = 0, g = 1, b = 0;

	if (messageType >= u8(EMessageType::S_Message) && messageType <= u8(EMessageType::S_MessageError))
	{
		r = 1;
		g = 1;
		b = 1;
	}

	GetChatConsole()->AddMessage(msg, r, g, b);
}

#endif
#endif	

void KGameInstance::PrintMaps()
{
	for (const auto& kv : LocalMaps)
		LOG(kv.second.MapFileName + " - " + kv.second.MapIngameName);	
}

void KGameInstance::AddPause()
{
	PauseHandles++;
}

void KGameInstance::RemovePause()
{
	if (PauseHandles == 0) return;
	if (PauseHandles == 1 && GetMatch())
	{
		if (GetMatch()->GetAccumulator().IsStarted())
			GetMatch()->GetAccumulator().StartAccumulating();
	}
	PauseHandles--;
}

bool KGameInstance::IsPaused() const
{
#if _SERVER
	return PauseHandles > 0;
#else
	return (PauseHandles > 0 || (GetEngineConsole()->IsShowing() && bConsolePause)) && !NetInterface.get();
#endif
}
