#if !_SERVER

#include "wall_torch.h"
#include "../graphics/torch.h"

void KMapEntity_WallTorch::CreateTorch()
{
	auto torch = TDataPool<KEntity_WallTorch>::GetPool()->CreateNew().Get();
	torch->SetPosition(MapPosition);
	torch->Yaw = Angle;

	TVector<f32> main = MainColor.ToFloatArray();
	TVector<f32> off = OffColor.ToFloatArray();

	if (main.size() == 4)
		torch->MainColor = FColor32(main[0], main[1], main[2], main[3]).To8();

	if (off.size() == 4)
		torch->OffColor = FColor32(off[0], off[1], off[2], off[3]).To8();
}

#endif