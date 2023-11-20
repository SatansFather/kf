#if !_SERVER

#include "bullet_hole.h"
#include "engine/game/local_player.h"
#include "engine/game_instance.h"

void KEntity_BulletHole::Create(const GVec3& position, const GVec3& normal)
{
	auto holeref = TDataPool<KEntity_BulletHole>::GetPool()->CreateNew();
	if (KEntity_BulletHole* hole = holeref.Get())
	{
		hole->Position = position.ToType<f32>();
		hole->Normal = normal.ToType<f32>();
		hole->RenderableLifespan = 5;
		hole->RenderTimeCreated = KGameInstance::Get().GetTotalRenderTime();
	}
}

KBufferUpdateResult KEntity_BulletHole::UpdateBuffers(KBulletHole& entry)
{
	entry.SetPosition(Position);
	entry.SetNormal(Normal);
	entry.SetTimeCreated(RenderTimeCreated);

	GVec3 camPos = GetLocalPlayer()->CameraPosition;

	return KBufferUpdateResult(false, camPos.DistanceSq(Position.ToType<GFlt>()) < (768 * 768));
}

#endif
