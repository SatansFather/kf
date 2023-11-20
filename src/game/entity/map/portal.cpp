#include "portal.h"
#include "engine/game/match.h"

void KMapEntity_Portal::LinkTarget()
{
	if (KGameMatch* match = GetGameMatch())
	  if (KMapEntity_PortalExit* exit = match->GetEntTarget<KMapEntity_PortalExit>(TargetName))
		Target = exit;

}

void KMapEntity_Portal::OnCollisionBrushAssociation()
{
	if (!bSilent)
	{
		bEmitsSound = true;
		KSoundProperties props;
		props.bLooping = true;
		props.MaxDistance = 512;
		AmbientInstance = KAudio::PlaySound3D(KSoundID::Ambient_Portal, MapPosition, props);
	}
}

void KMapEntity_Portal::UpdateSoundPosition(const GVec3& pos)
{
	if (!bSilent) KAudio::UpdateSoundPosition(AmbientInstance, pos);
}
