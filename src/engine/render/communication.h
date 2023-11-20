#pragma once

#if !_SERVER

#include <memory>
#include <mutex>
#include "kfglobal.h"
#include "engine/math/glm.h"
#include "engine/system/time.h"
#include "pending_commands.h"
#include "engine/net/net_stats.h"
#include "camera_push.h"
#include "render_buffers.h"

// DELETE
#include "bufferslots.h"
#include "engine/render/precipitation.h"
#include "../../game/entity/powerup_index.h"
#include "buffers/buffer_fog.h"
#include "../../game/character_info.h"
#include "../game/damage_number.h"
#include "../../game/player_score_info.h"
#include "../game/frag_message.h"
#include "../game/kill_feed_message.h"
#include "../../game/on_screen_message.h"
#include "../../game/pickup_message.h"

// handles communication between game and renderer
// must be locked/unlocked when using
class KRenderBridge
{
	friend class KGameInstance;

private:
	

	std::mutex Mutex;

	KTimePoint LastResizeTime;

	glm::vec3 LastCameraPosition;
	glm::vec3 CameraPosition;
	f32 LastCameraRoll = 0;
	f32 CameraRoll = 0;

	f64 GameTimeDilation = 1;

	KPendingCommands PendingCommands;


public:

	bool bLoadingMap = false;
	bool bResettingMap = false;
	KString LoadingMapName;
	std::atomic<f32> MapLoadProgress = { 0 };
	std::atomic<bool> bRenderAcknowledgedMapChange = { false };

	// loaded with map
	TVector<KFogBuffer> PendingFog;
	 
	f32 WeaponSwitchAlpha = 0;

	KCameraPush CameraPush;

	KTimePoint LastGameUpdateTime;
	bool bNewFrameReady = false;
	TVector<KPrecipitationVolume> PendingPrecipitation;
	KBufferMap RenderBufferMap;

	TVector<KDamageNumber> LiveDamageNumbers;
	TVector<KKillFeedMessage> NewKillFeedMessages;
	KFragMessage CurrentFragMessage;
	KDeathMessage CurrentDeathMessage;
	TVector<KOnScreenMessage> OnScreenMessages;
	KPickupMessage PendingPickupMessage;

	f32 PendingPitch, PendingYaw;
	bool bPendingPitch = false, bPendingYaw = false;
	bool bIsMainMenu = false;
	std::atomic<bool> bRenderTookFrame = { false };

	std::atomic<f64> LastFrameTime = { 0 };

	u32 GameFrameCount = 0;

	KNetStats NetStats;

	f64 LastTickTime = 0;
	f64 LastNetTime = 0;
	f64 LastCopyTime = 0;
	f64 LastRenderUpdateTime = 0;

	f64 TotalRenderTime = 0;

	f64 LastTeleportTime = -1;
	f32 UnderWaterDepth = 0;

	bool bDrawStats = true;

#if !_COMPILER
	KCharacterInfo CharacterInfo;
	TVector<KPlayerScoreInfo> PlayerInfo;
#endif

	bool bShowScoreboard = false;

	KString TargetPlayerName;

public:
	
	KRenderBridge();
	~KRenderBridge();

	static KRenderBridge& Get()
	{
		static KRenderBridge instance;
		return instance;
	}

	glm::vec3 GetLastCameraPosition() const { return LastCameraPosition; }
	glm::vec3 GetCameraPosition() const { return CameraPosition; }
	f32 GetLastCameraRoll() const { return LastCameraRoll; }
	f32 GetCameraRoll() const { return CameraRoll; }
	KTimePoint GetLastGameUpdateTime() const { return LastGameUpdateTime; }
	f64 GetGameTimeDilation() const { return GameTimeDilation; }
	KTimePoint GetLastResizeTime() const { return LastResizeTime; }
	KPendingCommands& GetPendingCommands() { return PendingCommands; }
	bool GetNewResources();

	// called from renderer only
	void ProcessCommands();

	void LockAccess() { Mutex.lock(); }
	void UnlockAccess() { Mutex.unlock(); }
};

#endif