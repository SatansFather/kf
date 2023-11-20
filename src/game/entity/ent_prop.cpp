#include "ent_prop.h"
#include "entity.h"

KEntProp::~KEntProp() {}

class KEntity* KEntProp::GetEntity()
{
	return dynamic_cast<KEntity*>(this);
}
