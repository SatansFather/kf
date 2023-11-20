#include "controllable.h"
#include "engine/input/listener_game.h"
#include "engine/input/view.h"
#include "engine/game/local_player.h"
#include "engine/game_instance.h"
#include "engine/net/player.h"
#include "engine/net/net_interface.h"

// DELETE
#include "../character_player.h"


#define HOST_RUN_FORWARD 0

#define minV -127
#define	maxV 127

void KEntProp_Controllable::GetTransient_ReppedPitch(u16& val)
{
	val = GetPitchAsInt();
}

void KEntProp_Controllable::SetTransient_ReppedPitch(u16& val)
{
	SetPitchInt(val, false);
}

void KEntProp_Controllable::GetTransient_ReppedYaw(u16& val)
{
	val = GetYawAsInt();
}

void KEntProp_Controllable::SetTransient_ReppedYaw(u16& val)
{
	SetYawInt(val, false);
}

void KEntProp_Controllable::GetTransient_ReplayPitch(u16& val)
{
	//val = GetPitchAsInt();
}

void KEntProp_Controllable::SetTransient_ReplayPitch(u16& val)
{
	//SetPitchInt(val, false);
}

void KEntProp_Controllable::GetTransient_ReplayYaw(u16& val)
{
	//val = GetYawAsInt();
}

void KEntProp_Controllable::SetTransient_ReplayYaw(u16& val)
{
	//SetYawInt(val, false);
}

GVec3 KEntProp_Controllable::GetInputVector()
{
	GVec3 out;

	out.x = ForwardInput == 0 ? 0 :
		ForwardInput < 0 ? MapRange(ForwardInput, minV, 0, -1.0, 0.0) :
		MapRange(ForwardInput, 0, maxV, 0.0, 1.0);

	out.y = RightInput == 0 ? 0 :
		RightInput < 0 ? MapRange(RightInput, minV, 0, -1.0, 0.0) :
		MapRange(RightInput, 0, maxV, 0.0, 1.0);

	out.z = UpInput == 0 ? 0 :
		UpInput < 0 ? MapRange(UpInput, minV, 0, -1.0, 0.0) :
		MapRange(UpInput, 0, maxV, 0.0, 1.0);

	if (IsControlledEntity()) out.Rotate(glm::degrees(Yaw), GVec3(0, 0, 1));
	out.Normalize();
	return out;
}

void KEntProp_Controllable::SetInputVector(GVec3 v)
{
	v.Normalize();
	v.ToPitchYaw(Pitch, Yaw);
	v = GVec3(1, 0, 0);

	ForwardInput = v.x == 0 ? 0 :
		v.x < 0 ? std::round(MapRange(v.x, 0, -1, 0, minV)) :
		std::round(MapRange(v.x, 0, 1, 0, maxV));

	RightInput = v.y == 0 ? 0 :
		v.y < 0 ? std::round(MapRange(v.y, 0, -1, 0, minV)) :
		std::round(MapRange(v.y, 0, 1, 0, maxV));

	UpInput = v.z == 0 ? 0 :
		v.z < 0 ? std::round(MapRange(v.z, 0, -1, 0, minV)) :
		std::round(MapRange(v.z, 0, 1, 0, maxV));
}

void KEntProp_Controllable::SetInputFromPlayer()
{
	// TODO check if player is local or net
	// dont put server input inside server block
#if _SERVER
	
#else
	Controllable_SetInputFromState(GetGameInput()->GetState());
	Pitch = GetGameInput()->GetPitchAsFloat();
	Yaw = GetGameInput()->GetYawAsFloat();
#endif
}

void KEntProp_Controllable::SetInputFromClient()
{
	u8 ind = Controllable_GetPlayerIndex();
	if (ind >= 0)
	{
		if (KNetPlayer* player = GetNetInterface()->GetPlayerFromIndex(ind))
		{
			u32 frame = KTime::FrameCount();
			if (player->ClientInput.contains(frame))
			{
				KBufferedClientInput input = player->ClientInput[frame];
				Controllable_SetInputFromState(input.Keys);

				GFlt rad = glm::radians(90.f);
				if (input.Pitch == (MAX_I16 / 2) + 1) Pitch = 0;
				Pitch = MapRange(input.Pitch, 0, MAX_I16, -rad, rad);

				rad = glm::radians(180.f);
				if (input.Yaw == MAX_I16 + 1) Yaw = 0;
				Yaw = MapRange(input.Yaw, 0, MAX_U16, -rad, rad);

				InputState.Keys = input.Keys;
				InputState.WeaponIndex = input.WeaponIndex;

				u32 comp = frame - input.ClientCompFrame;
				player->CurrentCompFrames = std::clamp(comp, 0u, 255u);
			}
			else
			{
				// we're missing this frame, let the client know
				if (frame > player->FinishLoadingFrame + 60 && player->ClientFrameDiff >= 0)
				{
					player->LostInputFrames.push_back(frame);
					//LOG(KString("----") + KString(frame));
				}
			}
		}
	}
}

GVec3 KEntProp_Controllable::GetInputVectorForMove()
{
	GVec3 forward(cos(Yaw), sin(Yaw), 0);
	GVec3 up(0, 0, 1);
	GVec3 right = forward ^ up;

	/*int count;
	const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

	RightInput = MapRange(-axes[0], -1, 1, minV, maxV);
	if (abs(RightInput) < 10) RightInput = 0;

	ForwardInput = MapRange(-axes[1], -1, 1, minV, maxV);
	if (abs(ForwardInput) < 10) ForwardInput = 0;*/

	GVec3 input(ForwardInput, -RightInput, UpInput);
	input = forward * input.x + right * input.y + up * input.z;

	return input;
}	

u16 KEntProp_Controllable::GetYawAsInt() const
{
	GFlt rad = glm::radians(180.f);
	return std::round(MapRange(Yaw, -rad, rad, 0, MAX_U16));
}

u16 KEntProp_Controllable::GetPitchAsInt() const
{
	GFlt rad = glm::radians(90.f);
	return std::round(MapRange(Pitch, -rad, rad, 0, MAX_I16));
}

bool KEntProp_Controllable::IsPlayerControlled()
{
	KNetInterface* iface = GetNetInterface();

	if (!iface)return IsControlledEntity();

	u8 ind = Controllable_GetPlayerIndex();
	if (ind != NULL_PLAYER)
	{
		KNetPlayer* p = GetNetInterface()->GetPlayerFromIndex(ind);
		return p->ControlledEntity == GetEntity();
	}

	return false;
}

bool KEntProp_Controllable::IsControlledEntity()
{
	KEntity* ent = GetEntity();
	return ent && ent == GetControlledEntity();
}

bool KEntProp_Controllable::IsSpectatedEntity()
{
	KEntity* ent = GetEntity();
	return ent && ent == GetSpectatedEntity();
}

bool KEntProp_Controllable::IsViewedEntity()
{
	KEntity* ent = GetEntity();
	return ent && ent == GetViewedEntity();
}

void KEntProp_Controllable::SetYaw(GFlt yaw, bool updateCamera)
{
	if (updateCamera && IsViewedEntity())
	{
		GetLocalPlayer()->bPendingYaw = true;
		GetLocalPlayer()->PendingYaw = yaw;
	}

	Yaw = yaw;
}

void KEntProp_Controllable::SetPitch(GFlt pitch, bool updateCamera)
{
	if (updateCamera && IsViewedEntity())
	{
		GetLocalPlayer()->bPendingPitch = true;
		GetLocalPlayer()->PendingPitch = pitch;
	}

	Pitch = pitch;
}

void KEntProp_Controllable::SetYawDegrees(GFlt yaw, bool updateCamera)
{
	SetYaw(glm::radians(yaw), updateCamera);
}

void KEntProp_Controllable::SetPitchDegrees(GFlt pitch, bool updateCamera)
{
	SetPitch(glm::radians(pitch), updateCamera);
}

void KEntProp_Controllable::SetYawInt(u16 yaw, bool updateCamera)
{
	GFlt rad = glm::radians(180.f);
	if (yaw == MAX_I16 + 1) Yaw = 0;
	SetYaw(MapRange(yaw, 0, MAX_U16, -rad, rad), updateCamera);
}

void KEntProp_Controllable::SetPitchInt(u16 pitch, bool updateCamera)
{
	GFlt rad = glm::radians(90.f);
	if (pitch == (MAX_I16 / 2) + 1) Pitch = 0;
	SetPitch(MapRange(pitch, 0, MAX_I16, -rad, rad), updateCamera);
}

u8 KEntProp_Controllable::Controllable_GetPlayerIndex()
{
	if (KSnapshottable* snap = GetEntity()->As<KSnapshottable>())
		return snap->OwningPlayerIndex;

	return NULL_PLAYER;
}

void KEntProp_Controllable::Controllable_SetInputFromState(u8 state)
{
	if (((state & EInputKeyState::MoveForward) > 0) != ((state & EInputKeyState::MoveBack) > 0))
		ForwardInput = (state & EInputKeyState::MoveForward) ? maxV : minV;
	else
		ForwardInput = 0;

	if (((state & EInputKeyState::MoveRight) > 0) != ((state & EInputKeyState::MoveLeft) > 0))
		RightInput = (state & EInputKeyState::MoveRight) ? minV : maxV;
	else
		RightInput = 0;

	if (((state & EInputKeyState::Jump) > 0) != ((state & EInputKeyState::Crouching) > 0))
		UpInput = (state & EInputKeyState::Jump) ? maxV : minV;
	else
		UpInput = 0;
}
