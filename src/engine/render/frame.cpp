#if !_SERVER

#include "interface/render_interface.h"
#include "communication.h"
#include "hud/hud_engine_console.h"
#include "interface/render_target.h"
#include "hud/hud_net_stats.h"
#include "hud/hud_frame_stats.h"
#include "hud/hud_character_info.h"

// DELETE
#include "engine/system/terminal/terminal.h"
#include "color.h"
#include "../../compiler/compiler.h"
#include "light_spot.h"
#include "interface/blend_state.h"
#include "debug_draw.h"
#include "bufferslots.h"
#include "particle.h"
#include "../utility/random.h"
#include "../../game/entity/projectile.h"
#include "scene.h"
#include "../input/view.h"
#include "model_formats/model_obj.h"
#include "hud/hud_game_frame_stats.h"
#include "hud/hud_powerup_timers.h"
#include "hud/hud_load_screen.h"
#include "interface/rasterizer.h"
#include "hud/hud_chat_console.h"
#include "hud/hud_menu.h"
#include "engine/menus/basic_menu.h"
#include "../console/pending_command.h"
#include "draw_config.h"
#include "hud/hud_game_timer.h"
#include "hud/hud_damage_numbers.h"
#include "hud/hud_target_name.h"
#include "hud/hud_scoreboard.h"
#include "hud/hud_frag_message.h"
#include "hud/hud_kill_feed.h"
#include "hud/hud_on_screen_message.h"
#include "hud/hud_pickup_message.h"
#include "interface/window.h"

bool asdf = false;
bool thecorn = false;
void KRenderInterface::PrepFrame()
{

	FrameStats.ResetFrame();
	FrameStats.UpdatePeriod();

#if !_COMPILER

	KTimePoint resizeTime = LastResizeTime;

	KRenderBridge& bridge = KRenderBridge::Get();
	bridge.LockAccess();
	bIsMainMenu = bridge.bIsMainMenu;
	bridge.TotalRenderTime = GetTotalRenderTime();

	bridge.ProcessCommands();

	if (bridge.bLoadingMap)
	{
		//CharacterInfo->Info.Health = 0;
		//CharacterInfo->LastInfo.Health = 0;
		//CharacterInfo->LastHealthDecrease = false;
		bLoadingMap = true;//bridge.bLoadingMap;
		bResettingMap = bridge.bResettingMap;
		LoadingMapName = bridge.LoadingMapName;
		HudScoreboard->NumericLayouts.clear();
		HudScoreboard->PlayerNameLayouts.clear();
		if (!bResettingMap) ClearFog();
	}
	bridge.bRenderAcknowledgedMapChange = bLoadingMap;

	bridge.LastFrameTime = LastFrameTime;
	GameTimeDilation = bridge.GetGameTimeDilation();
	resizeTime = bridge.GetLastResizeTime();

	bool startedLoading = bLoadingMap;

	bDrawStats = bridge.bDrawStats;

	if (bridge.bNewFrameReady)
	{
		bridge.bNewFrameReady = false;
		bLoadingMap = bridge.bLoadingMap;

		bridge.CameraPush.UpdateForRender(Camera.Push);

		Camera.UpdateFromRenderBridge();

		if (HudNetStats.get())
			HudNetStats->NetStats = bridge.NetStats;

		LastGameUpdateTime = bridge.GetLastGameUpdateTime();

		GameFrameCount = bridge.GameFrameCount;

		LiveDamageNumbers = bridge.LiveDamageNumbers;

		for (const KPrecipitationVolume& p: bridge.PendingPrecipitation)
			AddPrecipitation(p);

		bridge.PendingPrecipitation.clear();

		KRenderBufferInfo::CopyMap(&bridge.RenderBufferMap, &RenderBufferPointers.Map, false, true);

		if (bridge.bPendingYaw)
			KInputView::SetYaw(bridge.PendingYaw);
		if (bridge.bPendingPitch)
			KInputView::SetPitch(bridge.PendingPitch);

		bridge.bPendingYaw = false;
		bridge.bPendingPitch = false;
		bridge.bRenderTookFrame = true;

		// TEMP
		WeaponSwitchAlpha = bridge.WeaponSwitchAlpha;

		HudGameTickStats->AddTickTime(bridge.LastTickTime);
		HudGameTickStats->AddNetTime(bridge.LastNetTime);
		HudGameTickStats->AddCopyTime(bridge.LastCopyTime);
		HudGameTickStats->AddRenderUpdateTime(bridge.LastRenderUpdateTime);
		
		CharacterInfo->Info.CopyFrom(bridge.CharacterInfo);

		HudScoreboard->PlayerInfo = bridge.PlayerInfo;

		HudFragMessage->CurrentFragMessage = bridge.CurrentFragMessage;
		HudFragMessage->CurrentDeathMessage = bridge.CurrentDeathMessage;
		bridge.CurrentFragMessage.bUpdated = false;
		bridge.CurrentDeathMessage.bUpdated = false;
		
		HudOnScreenMessages->SubmitPending(bridge.OnScreenMessages);

		if (!bridge.PendingPickupMessage.Text.IsEmpty())
		{
			if (bridge.PendingPickupMessage.Text.Get() == HudPickupMessage->Text.Get())
				HudPickupMessage->Combo++;
			else
				HudPickupMessage->Combo = 0;

			HudPickupMessage->Text = bridge.PendingPickupMessage.Text;
			HudPickupMessage->LastUpdateTime = KTime::Now();
			bridge.PendingPickupMessage.Text = "";
		}

		HudKillFeed->KillFeedMessages.insert(HudKillFeed->KillFeedMessages.end(), bridge.NewKillFeedMessages.begin(), bridge.NewKillFeedMessages.end());
		bridge.NewKillFeedMessages.clear();

		memcpy(HudPowerupTimers->FramesRemaining, bridge.CharacterInfo.PowerupFramesRemaining, sizeof(u16) * EPowerupID::NumPowerups);
	}

	if (bridge.PendingFog.size() > 0)
	{
		UpdateFog(bridge.PendingFog);
		bridge.PendingFog.clear();
	}	

	LastTeleportTime = bridge.LastTeleportTime;
	UnderWaterDepth = bridge.UnderWaterDepth;

	if (HudTargetName) HudTargetName->TargetPlayerName = bridge.TargetPlayerName;
	if (HudScoreboard) HudScoreboard->bShowing = bridge.bShowScoreboard;

	bridge.UnlockAccess();

	if (startedLoading && !bLoadingMap)
		TotalRenderTime = 0;

	for (KPendingConsoleCommand& comm : PendingConsoleCommands)
	{
		KString out = comm.Function(comm.Value);
		if (!out.IsEmpty()) LOG(out);
	}
	PendingConsoleCommands.clear();

	ProcessPendingScreenshot();

	RenderAlpha = GetDrawConfig()->bInterpolate ?
		((false && GetLastFrameTime() > (GameFrameDelta()) / GameTimeDilation) ? 1.f :
			KClamp(f32(KTime::Since(LastGameUpdateTime) / (GameFrameDelta() / GameTimeDilation)), 0.f, 1.f))
		: 1;

	//RenderAlpha = 1;

	//if (RenderAlpha == 1) LOG(RenderAlpha, 0, 0, 1);

	if (bPendingRescale || KTime::Between(LastResizeTime, resizeTime) > 0)
	{
		HandleResize();
		NotifyWidgetsResize();
		LastResizeTime = resizeTime;
		bPendingRescale = false;
	}

#endif
	
	ClearBackBuffer(1, 1, 1, 1);
	ClearDepthBuffer();
}

void KRenderInterface::DrawScene()
{
	if (GameTimeDilation == 0)
	{
		RenderAlpha = 1;
	//	RenderFinalTargetToBackBuffer();
	//	return;
	}

	FinalTarget->Clear(0, 0, 0, 0);
	WaterTarget->Clear(0, 0, 0, 0);
	WaterDepthTarget->Clear(1, 999999, 999999, 0);
	TransparencyTarget->Clear(0, 0, 0, 0);
	SolidTarget->Clear(0, 0, 0, 0);

	if (bLoadingMap || !WorldScene) 
	{
		//RenderPostTargetToBackBuffer();
		//ClearBackBuffer(.1, .1, .1, 1);
		ClearBackBuffer(0, 0, 0, 1);
		return;
	}

	/*if (Radiosity.bPendingTest)
	{
		CalculatePendingRadiosity();
		return;
	}*/

#if !_COMPILER

	SetDepthBufferReadOnly(false);

	// update time etc
	UpdatePerFrameData();

	SetViewport(GetRenderResX(), GetRenderResY());

	// draw sky
	if (SkyboxScene)
	{
		Camera.SetFromPosition(SkyboxScene->GetBounds().GetCenter().ToGLM());
		SkyboxScene->DrawOpaque(true);
		ClearDepthBuffer();
	}

	// update the camera matrix to the player's viewpoint
	Camera.SetFromPlayer();

	// draw opaque world geometry
	WorldScene->DrawOpaque();
	WorldScene->DrawLeakLines();

	LightOpaque();

	// draw opaque objects
	DrawOpaqueBuffers(false);

	// draw water
	WorldScene->DrawPrecipitation();
	WorldScene->DrawWater();

	CreateDownsampleDepthBuffer();

	// draw transparent objects
	DrawTransparentBuffers(false);
	SetTopology_LineList();


	// merge water opaque and transparent
	MergeTargetDomains();

	// draw view weapon
	GetCamera().SetForPlayerWeapon();
	ClearDepthBuffer();
	SetDepthBufferReadOnly(false);
	BindRenderTarget(FinalTarget.get(), SceneDepthBuffer.get());
	DrawOpaqueBuffers(true);
	DrawTransparentBuffers(true);

	// finish
	RenderFinalTargetToBackBuffer();
#endif
}


void KRenderInterface::DrawOpaqueBuffers(bool viewWeapon)
{
	if (!viewWeapon) BindRenderTarget(SolidTarget.get(), SceneDepthBuffer.get());
	DisableTransparency();
	SetDepthBufferReadOnly(false);
	for (KRenderBufferInfo* info :
		viewWeapon ? RenderBufferPointers.ViewWeaponOpaqueBuffers :
		RenderBufferPointers.OpaqueBuffers)
	{
		if (viewWeapon)
		{
			info->bDrawingViewWeapon = true;
			info->RenderFunction(*info);
			info->bDrawingViewWeapon = false;
		}
		else
		{
			if (info->ActiveCount > 0) info->RenderFunction(*info);
			if (info->bPendingViewWeapon)
				RenderBufferPointers.ViewWeaponOpaqueBuffers.push_back(info);
		}
	}

	if (viewWeapon) RenderBufferPointers.ViewWeaponOpaqueBuffers.clear();
}

void KRenderInterface::DrawTransparentBuffers(bool viewWeapon)
{
	// TODO view weapon doesnt need the same binds
	
	WaterTarget->BindTexture2D(11, EShaderStage::Pipeline);
	WaterDepthTarget->BindTexture2D(12, EShaderStage::Pipeline);
	//if (!viewWeapon) BindRenderTarget(TransparencyTarget.get(), SceneDepthBuffer.get());
	if (!viewWeapon) BindRenderTarget(TransparencyTarget.get(), SceneDepthBuffer_Downsampled.get());
	EnableTransparency();
	//SetDepthBufferReadOnly(true);
	SceneDepthBuffer_Downsampled->SetReadOnly(true);
	SetViewport(GetRenderResX() / 2, GetRenderResY() / 2);
	for (KRenderBufferInfo* info : 
		viewWeapon ? RenderBufferPointers.ViewWeaponTransparentBuffers:
		RenderBufferPointers.TransparentBuffers)
	{
		if (viewWeapon)
		{
			info->bDrawingViewWeapon = true;
			info->RenderFunction(*info);
			info->bDrawingViewWeapon = false;
		}
		else
		{
			if (info->ActiveCount > 0) info->RenderFunction(*info);
			if (info->bPendingViewWeapon)
				RenderBufferPointers.ViewWeaponTransparentBuffers.push_back(info);
		}
	}

	if (viewWeapon) RenderBufferPointers.ViewWeaponTransparentBuffers.clear();

	DisableTransparency();
	BindTexture2D(nullptr, 11, EShaderStage::Pipeline);
	BindTexture2D(nullptr, 12, EShaderStage::Pipeline);
	SetViewport(GetRenderResX(), GetRenderResY());
}

void KRenderInterface::MergeTargetDomains(KRenderTarget* target)
{
	// bloom test
	/*{
		BindRenderTarget(nullptr);
		BrightPixelsTarget->BindTexture2D(0, EShaderStage::Compute);
		SetUAVForCS(BlurOutputTarget.get());
		BindShaderProgram(Shaders.VerticalBlur.get());
		Dispatch(GetRenderResX(), GetRenderResY(), 1);
		BindShaderProgram(Shaders.HorizontalBlur.get());
		Dispatch(GetRenderResX(), GetRenderResY(), 1);
		BindTexture2D(0, 0, EShaderStage::Compute);
		SetUAVForCS(nullptr);
	}*/

if (true)//(((u32)KTime::SinceInit() / 3) % 2 == 0)
{
	Rasterizer->SetFillMode(EFillMode::Solid);
	Rasterizer->SetCullMode(ECullMode::CullNone);
	BindShaderProgram(Shaders.WaterEffects.get());
	BindRenderTarget(target ? target : FinalTarget.get(), nullptr);
	BindVertexBuffer(nullptr);
	SolidTarget->BindTexture2D(0);
	WaterTarget->BindTexture2D(1);
	WaterDepthTarget->BindTexture2D(2);
	SceneDepthBuffer->BindTexture2D(3);
	TransparencyTarget->BindTexture2D(4);
	Draw(3, 0);

	BindTexture2D(nullptr, 0);
	BindTexture2D(nullptr, 1);
	BindTexture2D(nullptr, 2);
	BindTexture2D(nullptr, 3);
	BindTexture2D(nullptr, 4);
}
else
{
	//Rasterizer->SetFillMode(EFillMode::Solid);
	//Rasterizer->SetCullMode(ECullMode::CullNone);
	BindShaderProgram(Shaders.WaterEffectsCS.get());
	//BindRenderTarget(target ? target : FinalTarget.get(), nullptr);
	BindRenderTarget(nullptr);
	BindVertexBuffer(nullptr);
	SetUAVForCS(FinalTarget.get());
	SolidTarget->BindTexture2D(0, EShaderStage::Compute);
	WaterTarget->BindTexture2D(1, EShaderStage::Compute);
	WaterDepthTarget->BindTexture2D(2, EShaderStage::Compute);
	SceneDepthBuffer->BindTexture2D(3, EShaderStage::Compute);
	TransparencyTarget->BindTexture2D(4, EShaderStage::Compute);
	Dispatch(GetRenderResX() / 8, GetRenderResY() / 8, 1);
	//Draw(3, 0);

	BindTexture2D(nullptr, 0, EShaderStage::Compute);
	BindTexture2D(nullptr, 1, EShaderStage::Compute);
	BindTexture2D(nullptr, 2, EShaderStage::Compute);
	BindTexture2D(nullptr, 3, EShaderStage::Compute);
	BindTexture2D(nullptr, 4, EShaderStage::Compute);
}
}

void KRenderInterface::RenderFinalTargetToBackBuffer()
{
	//BlurRenderTarget(FinalTarget.get());


	Rasterizer->SetFillMode(EFillMode::Solid);
	Rasterizer->SetCullMode(ECullMode::CullNone);
	BindShaderProgram(Shaders.FinalProcessing.get());
	SetViewport(GetViewportX(), GetViewportY());
	BindVertexBuffer(nullptr);
	BindBackBuffer(false);
	FinalTarget->BindTexture2D(0);
	Draw(3, 0);
	BindTexture2D(0, 0);
}

void KRenderInterface::DrawHud()
{
#if !_COMPILER
	if (Radiosity.bBuildingRadiosity) return;

	HUD_Begin();

	BindBackBuffer(false);

	if (!bIsMainMenu)
	{
		if (bDrawStats)
		{
			HudNetStats->Draw();
			HudFrameStats->Draw();
			HudGameTickStats->Draw();
		}

		HudPowerupTimers->Draw();

		f32 x = GetViewportX();
		f32 y = GetViewportY();
		x /= 2; // center screen
		y /= 2;
		f32 scale = GetScaledY();

		if (!GetIngameMenu()->IsShowing())
			HUD_DrawBitmap(crosshair.get(), x - 16 * scale, y - 16 * scale, scale * 32, scale * 32);
	
		CharacterInfo->Draw();
		HudDamageNumbers->Draw();
		HudGameTimer->Draw();
		HudTargetName->Draw();
		HudOnScreenMessages->Draw();
		HudFragMessage->Draw();
		HudPickupMessage->Draw();
		HudKillFeed->Draw();
		HudScoreboard->Draw();
		HudLoadScreen->Draw();
	}

	if (TotalRenderTime < .5 && !IsLoadingMap()) 
	{
		f32 fadeScale = 1 - KSaturate(sqrt(TotalRenderTime * 2));
		KHudRectF r;
		r.left = 0;
		r.top = 0;
		r.right = GetViewportX();
		r.bottom = GetViewportY();

		HUD_SetDrawColor(FColor32(0, 0, 0, fadeScale));
		HUD_FillRect(r);
	}

	Chat->Draw();
	Menu->Draw();
	Console->Draw();

	HUD_End();
#endif
}

void KRenderInterface::DrawDebugLines()
{
	
}


#endif
