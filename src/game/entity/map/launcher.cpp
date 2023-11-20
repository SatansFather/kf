#include "launcher.h"
#include "engine/game/match.h"
#include "portal.h"

GVec3 KMapEntity_Launcher::GetLaunchVelocity(const GVec3& offset, GFlt gravityScale) const
{
	if (!Target) return 0;

	GFlt grav = GetGameMatch()->GetWorldGravity() * gravityScale;
	GVec3 start = offset;//GetMapPosition() + offset;
	GVec3 toTarget = Target->GetMapPosition() - start;
	GVec3 dist = toTarget.Length();

	// TODO "start" is 0 0 0

	if (dist > 1)
	{
		grav = -abs(grav);
		GVec3 targetDir = toTarget / dist;
		GVec3 launchDir = GVec3::Lerp(GVec3(0, 0, 1), targetDir, LaunchArc).GetNormalized();
		GFlt angle = asin(launchDir.z);
		GFlt dx = GVec3(toTarget.x, toTarget.y, 0).Length();
		GFlt dz = toTarget.z;
		GFlt numerator = grav * (dx * dx) * .5;
		GFlt cosangle = cos(angle);
		GFlt denom = (dz - (dx * tan(angle))) * (cosangle * cosangle);
		GFlt insideSqrt = numerator / denom;
		if (insideSqrt >= 0)
		{
			GFlt speed = sqrt(insideSqrt);
			return launchDir * speed;
		}
	}

	return 0;
}

void KMapEntity_Launcher::LinkTarget()
{
	if (KGameMatch* match = GetGameMatch())
	  if (KMapEntity_PortalExit* exit = match->GetEntTarget<KMapEntity_PortalExit>(TargetName))
		Target = exit;
}
