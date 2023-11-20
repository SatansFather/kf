#include "smoke_sheet.h"

void KEntity_SmokeSheet::Create(const GVec3& pos)
{
	auto smoke = TDataPool<KEntity_SmokeSheet>::GetPool()->CreateNew().Get();
	smoke->SetPosition(pos);
}

#if !_SERVER

KBufferUpdateResult KEntity_SmokeSheet::UpdateBuffers(KSmokeSheet& entry)
{
	entry.SetPosition(GetPosition());
	return true;
}

#endif