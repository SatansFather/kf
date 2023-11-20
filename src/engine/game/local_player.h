#pragma once

#include "config.h"
#include "engine/math/vec3.h"
#include "engine/render/camera_push.h"
#include "weapon_render.h"

// DELETE
#include "../../game/entity/character_player.h"
#include "../../game/character_info.h"

class KLocalPlayer
{
#if !_SERVER
	KUserConfig Config;
#endif

public:

	u8 NetPlayerIndex = NULL_PLAYER;
	class KNetPlayer* LocalNetPlayer = nullptr;

	// for renderer
	f32 PendingPitch, PendingYaw;
	bool bPendingPitch = false, bPendingYaw = false;
	bool bTeleportedThisFrame = false;
	GVec3 CameraPosition;
	GFlt CameraRoll = 0;
	bool bPendingControlReset = false;
	f32 CameraLandOffset = 0;
	bool bPendingSpawnRequest = false;
	KString TargetPlayerName; // guy we're looking at, not spectating

	// net corrections
	GVec3 CameraCorrectionOffset;

	TObjRef<KEntity> ControlledEntity;

	KWeaponRenderInfo WeaponRenderInfo;

	KCharacterInfo CharacterInfo;

	f32 WaterDepth = 0;

#if !_SERVER
	KCameraPush CameraPush;
#endif

	void LoadConfig(const class KString& profile);
	void SaveConfig(const class KString& profile, bool lock = false);

	void PossessEntity(const TObjRef<KEntity>& ent);
	void PossessSpectator();

	bool HasPendingSpawnRequest();

#if !_SERVER
	KUserConfig* GetConfig() { return &Config; }
#endif

	void ResetForNewMatch();

	void Tick();

#if !_SERVER
	void PushCameraRotation(const FVec3& amount, f32 duration, f32 startAlpha = 0);
	void PushCameraFov(f32 amount, f32 duration, f32 startAlpha = 0);
	void ShakeCamera(f32 strength, f32 duration, f32 shakeRate);
	void AddConcussion(f32 concussion);
#else
	void PushCameraRotation(const FVec3& amount, f32 duration, f32 startAlpha = 0) {}
	void PushCameraFov(f32 amount, f32 duration, f32 startAlpha = 0) {}
	void ShakeCamera(f32 strength, f32 duration, f32 shakeRate) {}
	void AddConcussion(f32 concussion) {}
#endif
};

KLocalPlayer* GetLocalPlayer();
KEntity* GetControlledEntity();
KEntity* GetSpectatedEntity();
KEntity* GetViewedEntity();
