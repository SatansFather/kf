#pragma once

#if !_COMPILER

#include <memory>
#include "kfglobal.h"
#include "render/interface/render_interface.h"
#include <thread>
#include "game/local_player.h"
#include "game/local_map.h"

#if !_SERVER
#include "render/render_buffers.h"
#endif

#if !_COMPILER && !_SERVER
#include "console/pending_command.h"
#include "render/buffers/buffer_fog.h"
#endif

#include "kwad/kwad_file.h"

// the core "engine" object that drives the entire process

class KGameInstance
{
private:

	bool bIsRunning = false;
	bool bHasStarted = false;
	bool bLoadingMap = false;

	UPtr<class KGameMatch> CurrentMatch;
	bool bIsDestroyingMatch = false;

#if !_SERVER
	UPtr<class KRenderInterface> RenderInterface;
	KTimePoint LastResizeTime;
#endif

	UPtr<class KNetInterface> NetInterface;

#if !_SERVER
	UPtr<class KNetInterface_Client> PendingClient;
#endif

	std::thread GameThread;

//#if !_SERVER
	//UPtr<class KGameInputListener> GameInputListener;
	//UPtr<class KConsoleInputListener> ConsoleInputListener;
	//UPtr<class KChatInputListener> ChatInputListener;
//#endif

	KLocalPlayer LocalPlayer;

#if !_SERVER
	// precipitation buffers need to be removed from the renderer when the entity is destroyed
	TVector<u32> DestroyedPrecipitation;

	// tells GPU to show tele effect
#endif

	// key is hash
	TMap<KMapHash, KLocalMap> LocalMaps;

	// processing packets while loading map
	bool bRunningMapLoadNetQueries = false;

public:

	KWadFile MainWadFile;

	bool bProcessingServerWelcome = false;

	bool bConsolePause = false;
	bool bWeaponFlash = true;
	u32 PauseHandles = 0;

#if !_SERVER
	KBufferMap RenderBufferMap;
	TVector<KFogBuffer> PendingFog;
#if !_COMPILER
	std::mutex ConsoleCommandMutex;
	TVector<KPendingConsoleCommand> PendingConsoleCommands;
#endif
#endif


	f64 LastTeleportTime = -1;
	f64 LastTickTime = 0, LastNetTime = 0, LastRenderUpdateTime = 0;
	f64 GameTimeDilation = 1;

	// reduced when accumulator is over timestep and game is slowed
	// used for proper render interpolation in this situation
	f64 PerformanceTimeDilation = 1; 

	f64 TotalRenderTime = 0;

	f32 MasterVolume = 1;

	std::atomic<bool> bIsMainMenu = { false };
	bool bIsEmptyMap = false;

	TMap<u32, KString> LastListedReplays;
	KString PendingReplayName;

	bool bPendingServerConnect = false;
	u32 PendingAddress = 0;
	u32 PendingPort = 0;
	void ProcessPendingConnection();

	~KGameInstance();

	static KGameInstance& Get();

	// fire it up
	void RunGame();
	
	f64 GetTimeDilation();

#if !_SERVER
	KRenderInterface* GetRenderInterface() { return RenderInterface.get(); }
#endif

	inline class KGameMatch* GetMatch() { return CurrentMatch.get(); }
	bool IsDestroyingMatch() const { return bIsDestroyingMatch; }

	// gets the input listener for the current game state
	// either Game or Menu
	class KInputListener* GetPrimaryListener();
		
	void ExitGame();

	bool IsRunning() const { return bIsRunning; }

	void LoadMap(const KMapHash& hash);
	void LoadMap(const KString& map);


	void LoadReplay(const KString& replay);
	void CreateReplayNetInterface();
	void ListReplays();

private:
	
	void Initialize();

#if !_COMPILER
	// creates mem pools for various game objects
	// called at startup while our heap is open
	void CreateMemPools();

#endif

	// creates a render interface of the appropriate type
	void CreateRenderInterface();

	// called to start the game thread
	void StartGameThread();

	// pass data from the game to the renderer
public:
	void UpdateRenderBridge(bool paused = false);
private:

	// runs the game and doesnt return until a graceful shutdown
	// runs in game thread
	void MainLoop();

	// waits for input until the program closes
	void InputLoop();

	// attempts to compile a map from the command line
	void CompileMap();

	void Shutdown();

	// server will send out the last frame data
	void NetServerSendFrame();

	// client checks if it has connected to desired server yet
	void CheckPendingClientConnection();

	void UpdateLocalMaps();

	void RunPendingConsoleCommands();

#if !_DEV
	void LoadMainWad();
#endif

public:

	// server will check for received client input
	void NetServerReceiveData();

	// client will send input and process whatever the server has sent
	void UpdateNetClient(bool requireWelcome);

public:

#if !_SERVER
	void UpdateLastResizeTime();
	
	void InitMenus();

private:

	void InitMainMenu();
	void InitIngameMenu();
	void InitMapsMenu();
	void InitServerBrowser();
	void InitJoinMenu();
	void InitOptionsMenu();
	void InitVideoOptionsMenu();
	void InitAudioOptionsMenu();
	void InitGameOptionsMenu();
	void InitMouseOptionsMenu();
	void InitControlOptionsMenu();
	void InitInterfaceOptionsMenu();
	void InitMiscOptionsMenu();

public:

	void SendChatMessage(const KString& msg);
	void ReceiveChatMessage(const KString& msg, u8 messageType = 0);

	f64 GetTotalRenderTime() const { return TotalRenderTime; }

#endif

	void PrintMaps();

	KLocalPlayer* GetLocalPlayer() { return &LocalPlayer; }
	//class KGameInputListener* GetGameInputListener();

	bool IsLoadingMap() const { return bLoadingMap; }
	bool IsMainMenu() const { return bIsMainMenu; }

	void BeginNetHost(u16 port = 6668);
	void ConnectToServer(class KNetAddress address);
	void NetDisconnect();

	class KNetInterface* GetNetInterface();

	void FinalizeClientConnection();

	bool HasMapWithHash(KMapHash hash) const;
	bool HasMapWithFileName(const KString& name) const;
	bool HasMapWithIngameName(const KString& name) const;
	KLocalMap GetLocalMapByHash(KMapHash hash);

	void SaveGame(const KString& name);
	void LoadGame(const KString& name);

	void AddPause();
	void RemovePause();
	bool IsPaused() const;
};

#endif