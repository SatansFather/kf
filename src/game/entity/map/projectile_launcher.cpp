#include "projectile_launcher.h"
#include "engine/game/match.h"
#include "portal.h"
#include "../projectiles/proj_rocket.h"

void KMapEntity_ProjectileLauncher::LinkTarget()
{
	if (KGameMatch* match = GetGameMatch())
	  if (KMapEntity_PortalExit* exit = match->GetEntTarget<KMapEntity_PortalExit>(TargetName))
		Target = exit;
}

void KMapEntity_ProjectileLauncher::CreateLauncher()
{
	KEntity_ProjectileLauncher* l = TDataPool<KEntity_ProjectileLauncher>::GetPool()->CreateNew().Get();
	l->SetPosition(GetMapPosition());
	l->MapEntity = this;
	l->Interval = Interval;
	l->InitialDelay = InitialDelay;
}

void KEntity_ProjectileLauncher::Tick()
{
	if (!MapEntity || !MapEntity->Target) return;

	u32 frame = KTime::FrameCount();
	if (frame < InitialDelay) return;
	frame = KTime::FrameCount() - InitialDelay;
	if (frame % Interval != 0) return;

	GVec3 pos = GetPosition();
	GVec3 targetPos = MapEntity->Target->GetMapPosition();

	const GVec3 dir = (targetPos - pos).GetNormalized();

	KEntity_Projectile_Rocket* proj =
		TDataPool<KEntity_Projectile_Rocket>::GetPool()->CreateNew().Get();
	proj->SetPosition(pos);
	proj->Velocity = dir * 800;
	proj->BuildMatrix(proj->PrevMatrix);
#if !_SERVER
	proj->Trail = KEntity_RocketTrail::Create(proj->GetPosition(), proj->Velocity, false, false);
#endif
}
