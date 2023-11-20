#pragma once

#include "../ent_prop.h"
#include "kfglobal.h"
#include "engine/net/snapshottable.h"
#include "engine/net/net_vector.h"


class KEntProp_NetPosition : public KEntProp
{

	GVec3 NetPosA;
	GVec3 NetPosB;

public:
	SNAP_PROP_TRANSIENT(KNetVec3, ReppedNetPos, SNAP_SEND_OTHERS)
	void GetTransient_ReppedNetPos(KNetVec3& val);
	void SetTransient_ReppedNetPos(KNetVec3& val);

	GVec3 GetNetPosition();

	GVec3 GetNetPosA() const { return NetPosA; }
	GVec3 GetNetPosB() const { return NetPosB; }
};