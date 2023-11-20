#include "grid_cell.h"
#include "game/entity/properties/collidable.h"
#include "../../../game/entity/properties/movable.h"

KGridCell::KGridCell()
{
	Entities.reserve(16);
}	

void KGridCell::AddEntity(KEntProp_CollidableBBox* ent)
{
	Entities.insert(ent);
}

void KGridCell::RemoveEntity(KEntProp_CollidableBBox* ent)
{
	Entities.erase(ent);
}

void KGridCell::AddCompEntity(KCompEntity& ent)
{
#if !_COMPILER
	CompEntities.push_back(ent);
#endif
}
