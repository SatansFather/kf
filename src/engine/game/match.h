#pragma once

#include "engine/global/types_numeric.h"
#include "engine/global/types_container.h"
#include "accumulator_loop.h"
#include "engine/render/precipitation.h"
#include "local_map.h"
#include "damage_number.h"
#include "frag_message.h"
#include "kill_feed_message.h"
#include "../../game/on_screen_message.h"
#include "../../game/pickup_message.h"
#include "../replay/replay_write.h"
#include "../replay/replay_read.h"

class KGameMatch
{
	friend class KGameInstance;
	friend class KMapFile;

private:
	
	KAccumulatorLoop GameLoop;
	KTimePoint MatchStartTime;
	TVector<class KPool::DataPoolBase*> RelevantPools;
	TVector<class KPool::DataPoolBase*> RenderablePool;

	GFlt WorldGravity = 1150;

	// precipitation volumes as part of the level
	// store in a map with ID (as well as pending ID rather than pending pointer)
	//	to protect against the very small chance that a pending volume will be added
	//	and destroyed before passing to the renderer, leaving it to use a dangling pointer
	TMap<u32, UPtr<KPrecipitationVolume>> Precipitation;

	// volume IDs spawned in the level that need to be given to the renderer
	TVector<u32> PendingPrecipitationIDs;

#if !_COMPILER
	// entities placed in the map
	TVector<UPtr<class KLoadedMapEntity>> MapEntities;

	TMap<KString, class KLoadedMapEntity*> EntityTargets;
#endif

	UPtr<class KNetState> NetState;

	KLocalMap CurrentMapInfo;

public:

	TVector<class KDeferredDropSpawn> PendingDropSpawns;

	UPtr<class KBvhGrid> Grid;
	TVector<class KMapEntity_DeathmatchSpawn*> SpawnPoints;

	u32 GlobalCollisionIgnoreID = 0;

	// map entities as they were parsed from the file
	TVector<UPtr<class KMapEntity>> BaseMapEntities;

#if !_SERVER
	TVector<KDamageNumber> LiveDamageNumbers;
	TVector<KKillFeedMessage> NewKillFeedMessages;
	KFragMessage CurrentFragMessage;
	KDeathMessage CurrentDeathMessage;

	TVector<KOnScreenMessage> PendingOnScreenMessages;
	KPickupMessage PendingPickupMessage;
#endif

	KReplayWriter ReplayWriter;
	KReplayReader ReplayReader;

	TVector<UPtr<class KSpecPoint>> SpecPoints;

public:

	KGameMatch();
	~KGameMatch();

private:

	bool RunLoop();
	void RunFrame();
	void PreFrameUpdate();
	void ClearRelevantMemPools();
	void ProcessLoadedMapEntities();
	void FinishedLoadingMap();
	void ResetMatch();

	void UpdateBrushEntityAudioPositions();

public:
	
	u64 GetTimestep() { return GameLoop.GetTimestep(); }
	GFlt GetRenderAlpha();
	KAccumulatorLoop& GetAccumulator() { return GameLoop; }
	KTimePoint GetLastFrameConsumeTime();
	u64 GetAccumulatedTime() const;
	void UpdateLastFrameConsumeTime();
	GFlt GetWorldGravity() const { return WorldGravity; }
	KTimePoint GetMatchStart() { return MatchStartTime; }

	//void AddPause();
	//void RemovePause();
	//bool IsPaused() const;

	void InitNet();
	void ClearNet();

	void BuildPendingCollision();

	KLocalMap& GetCurrentMapInfo() { return CurrentMapInfo; }

#if !_COMPILER
	void AddLoadedEntity(UPtr<KLoadedMapEntity> ent);
	void AddTargetEntity(const KString& name, KLoadedMapEntity* ent);

	template <typename T>
	T* GetEntTarget(const KString& targetName);
	void LinkEntTargets();

	KLoadedMapEntity* GetEntByID(u32 id);

	class KNetState* GetNetState();

	class KEntity* SpawnPlayerMonsterCharacter(u32 playerIndex = 0);
	class KEntity* SpawnPlayerCharacter(u32 playerIndex = 0);
	void SpawnLocalSpectator();
#endif

	void ForceFrameCountAdjust(i32 count);

	void AddSpecPoint(const GVec3& pos, const GVec3 rotation);
};

#if !_COMPILER
template <typename T>
T* KGameMatch::GetEntTarget(const KString& targetName)
{
	if (EntityTargets.contains(targetName))
		return dynamic_cast<T*>(EntityTargets[targetName]);

	return nullptr;
}
#endif

KGameMatch* GetGameMatch();
bool HasMatchAuthority();