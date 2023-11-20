#include "match.h"
#include "engine/utility/thread_pool.h"
#include "engine/utility/k_assert.h"
#include "engine/collision/broadphase/bvh_grid.h"
#include "engine/net/state.h"
#include "../input/listener_game.h"
#include "../../game/entity/properties/renderable.h"
#include "../render/communication.h"
#include "../utility/random.h"
#include "../net/net_interface.h"
#include "../game_instance.h"
#include "game/spec_point.h"

#if !_COMPILER
#include "game/entity/character_player.h"
#include "game/entity/map/spawn_point.h"
#include "ent_creation.h"
#endif

// DELETE
#include "engine/system/terminal/terminal.h"
#include "../../game/testmover.h"
#include "../../compiler/compiler.h"
#include "../../game/entity/projectile.h"
#include "../../game/entity/character_monster.h"
#include "../audio/audio.h"
#include "../../game/entity/projectiles/proj_rocket.h"
#include "../../compiler/entity.h"
#include "../../game/underwater_sound.h"
#include "../net/player.h"
#include "../collision/trace.h"
#include "../../game/entity/deferred_drop.h"
#include "../../game/entity/smoke_sheet.h"
#include "../input/view.h"
#include "../menus/basic_menu.h"
#include "../net/game_state.h"

u32 FramesBetweenMonsters = 0;

template <typename T>
T* SpawnCharacter(u8 playerIndex)
{
	KNetInterface* iface = GetNetInterface();
	if (iface && !iface->IsServer()) return nullptr;

	// server spawning their own local character
	bool mine = playerIndex == 0 && (!iface || !iface->IsDedicated());

	TObjRef<T> guyref = TDataPool<T>::GetPool()->CreateNew();
	T* guy = guyref.Get();
	guy->OwningPlayerIndex = playerIndex;

	if (mine) GetLocalPlayer()->PossessEntity(guy);

	if (GetGameMatch()->SpawnPoints.size() == 0) return guy;

	u32 pcCount = TDataPool<KEntity_Character_Player>::GetPool()->GetCurrentCount();

	// find a spawn point
	KMapEntity_DeathmatchSpawn* spawn = nullptr;
	if ((!std::is_same<T, KEntity_Character_Player>::value && pcCount > 0) || pcCount > 1)
	{
		// reset spawn distances
		for (KMapEntity_DeathmatchSpawn* spawn : GetGameMatch()->SpawnPoints)
			spawn->Closest = -1;

		GFlt furthestDist = -1;
		for (KMapEntity_DeathmatchSpawn* spawn : GetGameMatch()->SpawnPoints)
		{
			for (KPoolable* p : *TDataPool<KEntity_Character_Player>::GetPool())
			{
				if (p == guy) continue;

				GFlt dist = dynamic_cast<KEntity*>(p)->GetPosition().Distance(spawn->GetMapPosition());
				if (spawn->Closest == -1 || dist < spawn->Closest)
					spawn->Closest = dist;
			}

			if (spawn->Closest > furthestDist)
				furthestDist = spawn->Closest;
		}

		TVector<KMapEntity_DeathmatchSpawn*> farSpawns;

		for (KMapEntity_DeathmatchSpawn* spawn : GetGameMatch()->SpawnPoints)
		  if (spawn->Closest > furthestDist * .6)
			farSpawns.push_back(spawn);

		if (farSpawns.size() == 0)
			spawn = GetGameMatch()->SpawnPoints[D_Random() % GetGameMatch()->SpawnPoints.size()];
		else
			spawn = farSpawns[D_Random() % farSpawns.size()];
	}
	else
	{
		spawn = GetGameMatch()->SpawnPoints[D_Random() % GetGameMatch()->SpawnPoints.size()];
	}

	if (spawn)
	{
		guy->SetPosition(spawn->GetMapPosition());
		guy->SetYawDegrees(spawn->Angle);
	}

	guy->SpawnYaw = guy->GetYawAsInt();
	guy->SetPitch(0);

	if (mine)
	{
		GetLocalPlayer()->bTeleportedThisFrame = true;
		GetLocalPlayer()->CameraPosition = guy->GetPosition().AdjustZ(20);
	}

	return guy;
}

KString CCOM_Monsters(const KString& val)
{
	f32 f;
	if (val.ToFloatSafe(f))
	{
		if (f > 60) f = 60;
		if (f > 0) FramesBetweenMonsters = 60.f / f;
		else  FramesBetweenMonsters = 0;
		return "";
	}
	
	return FramesBetweenMonsters;
}

// global frame delta set before ticking so we can just call GetFrameDelta from anywhere
//	instead of passing  "delta" through a bunch of functions
GFlt GameFrameDelta()
{
	static GFlt delta = 1.0 / 60.0;
	return delta;
}

GFlt GetFrameAlpha()
{
	return (KTime::Since(GetGameMatch()->GetLastFrameConsumeTime()) / GameFrameDelta());
	//u64 accum = GetGameMatch()->GetAccumulatedTime();
	//u64 step = GetGameMatch()->GetTimestep();
	//return GFlt(accum) / GFlt(step);
}

GFlt MatchLifeTime()
{
	if (KGameMatch* m = GetGameMatch())
		return KTime::Since(m->GetMatchStart());
	return 0;
}

KGameMatch* GetGameMatch()
{
	return KGameInstance::Get().GetMatch();
}

bool HasMatchAuthority()
{
	if (GetNetInterface())
		return GetNetInterface()->IsServer();
	return true;
}

KGameMatch::KGameMatch()
{
	Grid = std::make_unique<KBvhGrid>(512, 96);
	
	MatchStartTime = KTime::Now();
	
	KString replay = KGameInstance::Get().PendingReplayName;
	if (!replay.IsEmpty())
		ReplayReader.PlayReplay(replay);
	//else if /*(IsNetServer())*/(GetNetInterface())
	//	ReplayWriter.StartWriting();

	if (GetNetInterface()) InitNet();

	// TEMPORARY
	/*for (UPtr<KPrecipitationVolume>& prec : KMapCompiler::Get().PendingPrecipitation)
	{
		PendingPrecipitationIDs.push_back(prec->GetID());
		Precipitation[prec->GetID()] = std::move(prec);	
	}

	KMapCompiler::Get().PendingPrecipitation.clear();*/
}

KGameMatch::~KGameMatch()
{
	ClearRelevantMemPools();
	ClearNet();
}

bool KGameMatch::RunLoop()
{
	bool ran = false;

	GameLoop.AccumulateTime();
	KGameInstance& inst = KGameInstance::Get();

	while (GameLoop.ConsumeFrame() && KGameInstance::Get().IsRunning())
	{
		if (!ran) PreFrameUpdate();

		KTimePoint start = KTime::Now();
		RunFrame();
		inst.LastTickTime = KTime::Since(start);

		//if (KTime::FrameCount() % 20 == 0)
		//	LOG(inst.LastTickTime);

#if !_SERVER
		// only update render on the last loop
		if (!GameLoop.CanRunFrame())
		{
			// if we're running multiple frames, it looks smoother (and saves time) 
			// to update render data only once, after all logic is done
			start = KTime::Now();
			for (auto pool : RenderableDataPools)
			  for (auto obj : *pool)
			    dynamic_cast<KEntity*>(obj)->RenderUpdate();
			inst.LastRenderUpdateTime = KTime::Since(start);
		}
#endif
	
		//if (ReplayWriter.IsWriting())
		//	KGameStateSnapshot snapshot(KTime::FrameCount() - 1, ReplayWriter.GetBufferPointer(), GetLocalNetPlayer(), true);

		if (ReplayWriter.IsWriting() && KTime::FrameCount() % 300 == 0)
			ReplayWriter.AppendBufferToFile();

		ran = true;
	}
	
	return ran;
}

void KGameMatch::RunFrame()
{
#if !_SERVER
	for (i32 i = LiveDamageNumbers.size() - 1; i >= 0; i--)
	{
		KDamageNumber& num = LiveDamageNumbers[i];
		num.FramesRemaining--;
		if (num.FramesRemaining == 0)
			VectorRemoveAt(LiveDamageNumbers, i);
	}
	GetLocalPlayer()->Tick();
#endif

	if (IsNetServer())
	{
		// act on data received between frames
		KNetPlayer::bMatchTick = true;
		for (KPoolable* p : *TDataPool<KNetPlayer>::GetPool())
			p->Tick();
		KNetPlayer::bMatchTick = false;
	}

	// spawn pending drop items
	for (KDeferredDropSpawn& drop : PendingDropSpawns)
		drop.Spawn();
	PendingDropSpawns.clear();

	// where the game happens
	bool nextPool = false;
	for (auto pool : EntityDataPools)
	{
		for (KPoolable* p : *pool)
		{
			KEntity* ent = dynamic_cast<KEntity*>(p);
			if (ent->GetFrameCreated() == KTime::FrameCount())
			{
				nextPool = true;
				break;
			}
			ent->Tick();
		}

		if (nextPool)
		{
			nextPool = false;
			continue;
		}
	}
	
#if !_NOSOUND
	UpdateBrushEntityAudioPositions();
	KUnderwaterSound::Update();
#endif

	if (FramesBetweenMonsters > 0 && KTime::FrameCount() % FramesBetweenMonsters == 0)
	{
		auto pool = TDataPool<KEntity_Character_Monster>::GetPool();
	//	if (pool->GetCurrentCount() < 100)
		{
			u32 index = std::round(D_RandRange(0, SpawnPoints.size() - 1));
			KEntity_Character_Monster* mon = pool->CreateNew().Get();
			mon->SetPosition(SpawnPoints[index]->GetMapPosition());
		}
	}

	if (IsNetServer() && GetNetState())
	{
		// clear the data from the last time this index was used
		u32 frame = KTime::FrameCount();
		KCellMap& cell = GetNetState()->PositionSnapshots[frame % MAX_SNAPSHOTS];
		cell.ClearCellData();

		// update snapshot position history for ping comp
		for (auto pool : HistoryDataPools)
		  for (KPoolable* p : *pool)
		    dynamic_cast<KEntity*>(p)->AddPositionToHistory(frame);
	}	
}

void KGameMatch::PreFrameUpdate()
{
#if !_SERVER
	// let the renderer start using the last completed frame
	// do this here rather than earlier when the data is copied
	// so the update rate is consistent
	KRenderBridge::Get().LockAccess();
	KRenderBridge::Get().LastGameUpdateTime = KTime::Now();
	KRenderBridge::Get().bNewFrameReady = true;
	KRenderBridge::Get().bRenderTookFrame = false;
	KRenderBridge::Get().UnlockAccess();

	GetGameInput()->AcceptPendingData();

	// we're about to run a frame, current input is now final
	// send input to server and process whatever we received
	
	// do not do this in the game loop if we dont have the server welcome yet
	// doing that here would cause us to destroy this KGameMatch inside this function
	KGameInstance::Get().UpdateNetClient(true);
#endif
}

void KGameMatch::ClearRelevantMemPools()
{
	for (auto pool : EntityDataPools)
		pool->DestroyAll();

	for (auto pool : RenderableDataPools)
		pool->DestroyAll();
}

void KGameMatch::ProcessLoadedMapEntities()
{
	MapEntities.clear();
	for (UPtr<KMapEntity>& ent : BaseMapEntities)
	{
		auto e = KLoadedMapEntity::CreateFromFile(ent.get());
		if (e) AddLoadedEntity(std::move(e));
	}
	LinkEntTargets();
}

void KGameMatch::FinishedLoadingMap()
{
	if (CurrentMapInfo.MapFileName != "mainmenu" && CurrentMapInfo.MapFileName != "emptymap")
	{
		FramesBetweenMonsters = 0;

		if (KGameInstance::Get().GetNetInterface())
			InitNet();

#if !_SERVER && !_COMPILER
		if (!SpawnPlayerCharacter())
			SpawnLocalSpectator();
#endif
	}
	else
	{
		GetLocalPlayer()->CameraPosition = 0;
		GetLocalPlayer()->PendingPitch = 0;
		GetLocalPlayer()->PendingYaw = 0;
		GetLocalPlayer()->bPendingPitch = true;
		GetLocalPlayer()->bPendingYaw = true;
	}

	GameLoop.StartAccumulating();
}

void KGameMatch::ResetMatch()
{
	ClearRelevantMemPools();
	SpawnPoints.clear();
	ProcessLoadedMapEntities();
	GameLoop.LastAccumulationTime = {};
	GameLoop.LastElapsedCheckTime = {};
	GameLoop.LastFrameConsumeTime = {};
	GameLoop.bStarted = 0;
	GameLoop.Accumulator = 0;
	GameLoop.TotalFrames = 0;
	MatchStartTime = KTime::Now();
	GlobalCollisionIgnoreID = 0;
	//if (GetNetInterface()) InitNet();
}

void KGameMatch::UpdateBrushEntityAudioPositions()
{
	for (UPtr<KLoadedMapEntity>& ent : MapEntities)
	{
		if (!ent || !ent->bIsBrushEntity || !ent->bEmitsSound) continue;

		GVec3 camPos = GetLocalPlayer()->CameraPosition;
		if ((camPos.Distance(ent->MapPosition) - ent->MaxBoundsDimension) < 1024)
		{
			// if we arent in front of any planes, we are inside
			bool inside = true;
			for (GPlane& plane : ent->Planes)
			{
				if (plane.ClassifyPoint(camPos, 1) == EPointSide::Front)
				{
					inside = false;
					break;
				}
			}
			
			if (inside)
			{
				ent->UpdateSoundPosition(camPos);
				continue;
			}

			GVec3 closest;
			GFlt minDist = -1;
			for (GPolygon& poly : ent->Polygons)
			{
				GVec3 p = poly.ClosestPoint(camPos);
				GFlt sq = camPos.DistanceSq(p);
				if (sq < minDist || minDist == -1)
				{
					minDist = sq;
					closest = p;
				}
			}

			ent->UpdateSoundPosition(closest);
		}
	}
}

GFlt KGameMatch::GetRenderAlpha()
{
	return GameLoop.GetAlpha();
}

KTimePoint KGameMatch::GetLastFrameConsumeTime()
{
	return GameLoop.LastFrameConsumeTime;
}

u64 KGameMatch::GetAccumulatedTime() const
{
	f64 alpha = KTime::Since(GameLoop.LastFrameConsumeTime);
	alpha += (f64)GameLoop.Accumulator * 1e6;
	alpha /= GameFrameDelta();
	return alpha;
}

void KGameMatch::UpdateLastFrameConsumeTime()
{
	GameLoop.LastFrameConsumeTime = KTime::Now();
}

/*
void KGameMatch::AddPause()
{
	GameLoop.AddPause();
}

void KGameMatch::RemovePause()
{
	GameLoop.RemovePause();
}

bool KGameMatch::IsPaused() const
{	
	return GameLoop.IsPaused();
}*/

void KGameMatch::InitNet()
{
	if (!NetState.get())
	{
		NetState = std::make_unique<KNetState>();
		
		// add all snapshottables to the replicated objects map
	}
}

void KGameMatch::ClearNet()
{
	NetState.reset();
}

void KGameMatch::BuildPendingCollision()
{
	Grid->BuildFromPending();
}

#if !_COMPILER

void KGameMatch::AddLoadedEntity(UPtr<KLoadedMapEntity> ent)
{
	MapEntities.push_back(std::move(ent));
}

void KGameMatch::AddTargetEntity(const KString& name, KLoadedMapEntity* ent)
{	
	EntityTargets[name] = ent;
}

void KGameMatch::LinkEntTargets()
{	
	for (UPtr<KLoadedMapEntity>& ent : MapEntities)
		ent->LinkTarget();
}

KLoadedMapEntity* KGameMatch::GetEntByID(u32 id)
{
	for (UPtr<KLoadedMapEntity>& ent : MapEntities)
	  if (ent->GetID() == id)
	    return ent.get();

	return nullptr;
}

class KNetState* KGameMatch::GetNetState()
{
	return NetState.get();
}	

class KEntity* KGameMatch::SpawnPlayerMonsterCharacter(u32 playerIndex /*= 0*/)
{
	KEntity_Character_Monster* guy = SpawnCharacter<KEntity_Character_Monster>(playerIndex);
	guy->FrameTickInterval = 1;
	return guy;
}

KEntity* KGameMatch::SpawnPlayerCharacter(u32 playerIndex /*= 0*/)
{
	KEntity_Character_Player* guy = SpawnCharacter<KEntity_Character_Player>(playerIndex);
	guy->AddWeaponByID(EWeaponID::Shotgun, 20);
#if _DEV
	guy->AddWeaponByID(EWeaponID::Rocket, 9999);
	guy->AddWeaponByID(EWeaponID::Cannon, 9999);
	guy->AddWeaponByID(EWeaponID::Blast, 9999);
	guy->AddWeaponByID(EWeaponID::Zapper, 9999);
#endif
	return guy;
}

void KGameMatch::SpawnLocalSpectator()
{
	GetLocalPlayer()->PossessSpectator();
}

void KGameMatch::ForceFrameCountAdjust(i32 count)
{
	GameLoop.TotalFrames += count;
}

void KGameMatch::AddSpecPoint(const GVec3& pos, const GVec3 rotation)
{
	UPtr<KSpecPoint> p = std::make_unique<KSpecPoint>();
	p->Position = pos;
	p->Rotation = rotation;
	
	if (SpecPoints.size() > 0)
	{
		KSpecPoint* last = SpecPoints[SpecPoints.size() - 1].get();
		last->Next = p.get();
	}

	SpecPoints.push_back(std::move(p));
}

#endif
