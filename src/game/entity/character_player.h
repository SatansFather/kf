#pragma once

#include "character.h"
#include "properties/renderable.h"
#include "properties/weapon_inventory.h"
#include "mover_info.h"
#include "properties/move_states.h"
#include "engine/net/snapshottable.h"
#include "properties/powerup_inventory.h"

class KEntity_Character_Player : 
	public KEntity_Character, 
	public KEntProp_WeaponInventory,
	public KEntProp_PowerupInventory,
	public KMoveState_Walking,
	public KMoveState_Swimming,
	public KMoveState_Flying
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"player", "player_0">, KDynamicLight>
#endif
{
	glm::mat4 PrevMatrix;

public:

	SNAP_PROP_TRANSIENT(KNetVec3, DeathVelocity, SNAP_DESTROY)
	void GetTransient_DeathVelocity(KNetVec3& val);
	void SetTransient_DeathVelocity(KNetVec3& val);

	KEntity_Character_Player();

	void OnNetDestroy() override;

	void OnKilled(class KNetPlayer* player = nullptr) override;
	void OnEnteredWater(const GHitResult& hit) override;
	void CharacterTick() override;

#if !_SERVER
	void InitializeRenderable() override;
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"player", "player_0">& entry, KDynamicLight& light) override;
#endif
};