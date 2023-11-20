#include "hit_result.h"
#include "brush.h"

#if !_COMPILER
#include "engine/game/ent_creation.h"
#endif

template <typename T>
void KHitResult<T>::Reset()
{
	Time = 1;
	bHit = 0;
	Point = KVec3<T>(0, 0, 0);
	Normal = KVec3<T>(0, 0, 0);
	Object = nullptr;
	PlaneDistance = 0;
	PenetrationDepth = 0;
}

#if !_COMPILER
template <typename T>
class KLoadedMapEntity* KHitResult<T>::GetMapEntity()
{
	return ((KCollisionBrush*)Object)->Object;
}
#endif

template struct KHitResult<f32>;
template struct KHitResult<f64>;