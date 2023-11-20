#pragma once

#include "engine/math/vec3.h"

class KEntity : public KPoolable
{
	// projectiles need to adjust the creation frame during ping sweep
	friend class KEntity_Projectile;

	GVec3 Position;
	GVec3 CompedPositionOffset;
	GVec3 LastFrameNetPosition;
	u32 EntityFrameCreated = 0;

public:

	KEntity();

	const GVec3& GetPosition() const { return Position; }
	const GVec3& GetCompedPositionOffset() const { return CompedPositionOffset; }
	const GVec3& GetLastFrameNetPosition() const;
	u32 GetFrameCreated() const { return EntityFrameCreated; }

	u32 GetEntityFrameAge() const;
	GFlt GetEntityAge() const;

	void SetPosition(const GVec3& pos);

	void SetCompedPositionOffset(const GVec3& pos) { CompedPositionOffset = pos; }
	void ResetCompedPositionOffset() { CompedPositionOffset = 0; }
	
	// if this returns true, the entity has not yet been ticked
	// first tick is the frame after spawning
	bool CreatedThisFrame();

	void DestroyEntity();

	virtual void OnEntityDestroyed() {}
	virtual void OnPositionUpdated() {}
	void RenderUpdate(bool destroying = false);

	void Poolable_PreDestroy() override;

	virtual void AddPositionToHistory(u32 frame);

	virtual bool CanBeTelefragged() { return false; }

	class KEntity_Powerup* GetCarriedPowerup(u8 id);

	class KNetPlayer* GetOwningPlayer();

	template <typename T>
	T* As()
	{
		return dynamic_cast<T*>(this);
	}
};