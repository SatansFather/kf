#if !_SERVER && _WIN32

#include "d3d11_interface.h"
#include "shader_include.h"
#include "../../input_layout.h"
#include "../../interface/shader_program.h"

#if _DEV
void KRenderInterface_D3D11::LoadShaders()
{
	KRenderInterface::LoadShaders();
}
#else
void KRenderInterface_D3D11::LoadShaders()
{
	FInputElement position("POSITION", EInputElementType::Float, 3, EInputSlotClass::PerVertex);
	FInputElement texcoord("TEXCOORD", EInputElementType::Float, 2, EInputSlotClass::PerVertex);
	FInputElement normal("NORMAL", EInputElementType::Float, 3, EInputSlotClass::PerVertex);
	FInputElement lightcoord("LIGHTCOORD", EInputElementType::Float, 2, EInputSlotClass::PerVertex);
	FInputElement coordclamp("COORDCLAMP", EInputElementType::Float, 4, EInputSlotClass::PerVertex);
	FInputElement lightcolor("LIGHTCOLOR", EInputElementType::Float, 3, EInputSlotClass::PerVertex);
	FInputElement bufferindex("BUFFERINDEX", EInputElementType::UInt32, 1, EInputSlotClass::PerVertex);

	{
		Shaders.StaticMesh = CreateShaderProgram();
		Shaders.StaticMeshScaledOutline = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_test_vs;
		params.VertexShader.Size = sizeof(HLSL_test_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.InputLayout.Elements.push_back(position);
		params.InputLayout.Elements.push_back(texcoord);
		params.InputLayout.Elements.push_back(normal);

		FInputElement prev0("PREV", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 0);
		FInputElement prev1("PREV", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 1);
		FInputElement prev2("PREV", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 2);
		FInputElement prev3("PREV", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 3);
		FInputElement current0("CURRENT", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 0);
		FInputElement current1("CURRENT", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 1);
		FInputElement current2("CURRENT", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 2);
		FInputElement current3("CURRENT", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 3);
		FInputElement data("DATA", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 0);

		params.InputLayout.Elements.push_back(prev0);
		params.InputLayout.Elements.push_back(prev1);
		params.InputLayout.Elements.push_back(prev2);
		params.InputLayout.Elements.push_back(prev3);
		params.InputLayout.Elements.push_back(current0);
		params.InputLayout.Elements.push_back(current1);
		params.InputLayout.Elements.push_back(current2);
		params.InputLayout.Elements.push_back(current3);
		params.InputLayout.Elements.push_back(data);

		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.VertexShader.Data = (void*)HLSL_static_mesh_vs;
		params.VertexShader.Size = sizeof(HLSL_static_mesh_vs);
		params.PixelShader.Data = (void*)HLSL_static_mesh_ps;
		params.PixelShader.Size = sizeof(HLSL_static_mesh_ps);
		Shaders.StaticMesh->CreateShadersFromParams(params);
		params.VertexShader.Data = (void*)HLSL_static_mesh_scaled_outline_vs;
		params.VertexShader.Size = sizeof(HLSL_static_mesh_scaled_outline_vs);
		params.PixelShader.Data = (void*)HLSL_static_mesh_scaled_outline_ps;
		params.PixelShader.Size = sizeof(HLSL_static_mesh_scaled_outline_ps);
		Shaders.StaticMeshScaledOutline->CreateShadersFromParams(params);

		params.VertexShader.Data = (void*)HLSL_brain_powerup_vs;
		params.VertexShader.Size = sizeof(HLSL_brain_powerup_vs);
		params.PixelShader.Data = (void*)HLSL_brain_powerup_ps;
		params.PixelShader.Size = sizeof(HLSL_brain_powerup_ps);
		Shaders.BrainPowerup = CreateShaderProgram();
		Shaders.BrainPowerup->CreateShadersFromParams(params);

		params.VertexShader.Data = (void*)HLSL_health_crystal_vs;
		params.VertexShader.Size = sizeof(HLSL_health_crystal_vs);
		params.PixelShader.Data = (void*)HLSL_health_crystal_ps;
		params.PixelShader.Size = sizeof(HLSL_health_crystal_ps);
		Shaders.HealthCrystal = CreateShaderProgram();
		Shaders.HealthCrystal->CreateShadersFromParams(params);
	}

	{
		Shaders.WaterEffects = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_water_effects_ps;
		params.PixelShader.Size = sizeof(HLSL_water_effects_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.WaterEffects->CreateShadersFromParams(params);
	}

	{
		Shaders.WaterEffectsCS = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.Data = (void*)HLSL_water_effects_cs;
		params.ComputeShader.Size = sizeof(HLSL_water_effects_cs);
		params.ComputeShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.WaterEffectsCS->CreateShadersFromParams(params);
	}

	{
		Shaders.DepthDownsample = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_depth_downsample_ps;
		params.PixelShader.Size = sizeof(HLSL_depth_downsample_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.DepthDownsample->CreateShadersFromParams(params);
	}

	{
		Shaders.FinalProcessing = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_final_effects_ps;
		params.PixelShader.Size = sizeof(HLSL_final_effects_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.FinalProcessing->CreateShadersFromParams(params);
	}

	{
		Shaders.BlurCreate = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_blur_create_ps;
		params.PixelShader.Size = sizeof(HLSL_blur_create_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.BlurCreate->CreateShadersFromParams(params);
	}

	{
		Shaders.BlurApply = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_blur_apply_ps;
		params.PixelShader.Size = sizeof(HLSL_blur_apply_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.BlurApply->CreateShadersFromParams(params);
	}

	{
		Shaders.TextureToScreen = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_texture_to_screen_ps;
		params.PixelShader.Size = sizeof(HLSL_texture_to_screen_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.TextureToScreen->CreateShadersFromParams(params);
	}

	{
		Shaders.Radiosity = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.PixelShader.Data = (void*)HLSL_radiosity_copy_ps;
		params.PixelShader.Size = sizeof(HLSL_radiosity_copy_ps);

		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.Radiosity->CreateShadersFromParams(params);
	}

	{
		Shaders.DebugLine = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_debug_line_vs;
		params.VertexShader.Size = sizeof(HLSL_debug_line_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_debug_ps;
		params.PixelShader.Size = sizeof(HLSL_debug_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.DebugLine->CreateShadersFromParams(params);
	}

	{
		Shaders.DoomSprite = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_doomsprite_vs;
		params.VertexShader.Size = sizeof(HLSL_doomsprite_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_doomsprite_ps;
		params.PixelShader.Size = sizeof(HLSL_doomsprite_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.DoomSprite->CreateShadersFromParams(params);
	}

	{
		Shaders.SmokeBeam = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_smoke_beam_vs;
		params.VertexShader.Size = sizeof(HLSL_smoke_beam_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_smoke_dynamic_ps;
		params.PixelShader.Size = sizeof(HLSL_smoke_dynamic_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.SmokeBeam->CreateShadersFromParams(params);
	}

	{
		Shaders.HitSpark = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_hit_spark_vs;
		params.VertexShader.Size = sizeof(HLSL_hit_spark_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.HitSpark->CreateShadersFromParams(params);
	}

	{
		Shaders.BlasterParticle = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_blaster_particle_vs;
		params.VertexShader.Size = sizeof(HLSL_blaster_particle_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.BlasterParticle->CreateShadersFromParams(params);
	}

	{
		Shaders.BlasterExplosion = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_blaster_explosion_vs;
		params.VertexShader.Size = sizeof(HLSL_blaster_explosion_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.BlasterExplosion->CreateShadersFromParams(params);
	}

	{
		Shaders.WaterSplash = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_water_splash_vs;
		params.VertexShader.Size = sizeof(HLSL_water_splash_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.WaterSplash->CreateShadersFromParams(params);
	}

	{
		Shaders.PortalTravel = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_portal_travel_vs;
		params.VertexShader.Size = sizeof(HLSL_portal_travel_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.PortalTravel->CreateShadersFromParams(params);
	}

	{
		Shaders.SmokeSheet = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_smoke_sheet_vs;
		params.VertexShader.Size = sizeof(HLSL_smoke_sheet_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_smoke_sheet_ps;
		params.PixelShader.Size = sizeof(HLSL_smoke_sheet_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.SmokeSheet->CreateShadersFromParams(params);
	}

	{
		Shaders.Explosion = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_explosion_vs;
		params.VertexShader.Size = sizeof(HLSL_explosion_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_noclip_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_noclip_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.Explosion->CreateShadersFromParams(params);
	}

	{
		Shaders.RocketTrail = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_rocket_trail_vs;
		params.VertexShader.Size = sizeof(HLSL_rocket_trail_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_smoke_dynamic_ps;
		params.PixelShader.Size = sizeof(HLSL_smoke_dynamic_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.RocketTrail->CreateShadersFromParams(params);
	}

	{
		Shaders.Cube = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_bounding_box_vs;
		params.VertexShader.Size = sizeof(HLSL_bounding_box_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_bounding_box_ps;
		params.PixelShader.Size = sizeof(HLSL_bounding_box_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.Cube->CreateShadersFromParams(params);
	}

	{
		Shaders.ShotgunShard = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_shotgun_shard_vs;
		params.VertexShader.Size = sizeof(HLSL_shotgun_shard_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_shotgun_shard_ps;
		params.PixelShader.Size = sizeof(HLSL_shotgun_shard_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.ShotgunShard->CreateShadersFromParams(params);
	}

	{
		Shaders.BulletHole = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_bullet_hole_vs;
		params.VertexShader.Size = sizeof(HLSL_bullet_hole_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_bullet_hole_ps;
		params.PixelShader.Size = sizeof(HLSL_bullet_hole_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.BulletHole->CreateShadersFromParams(params);
	}

	{
		Shaders.BloodTrail = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_blood_trail_vs;
		params.VertexShader.Size = sizeof(HLSL_blood_trail_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.BloodTrail->CreateShadersFromParams(params);
	}

	{
		Shaders.TorchFlame = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_torch_flame_vs;
		params.VertexShader.Size = sizeof(HLSL_torch_flame_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_noclip_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_noclip_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.TorchFlame->CreateShadersFromParams(params);
	}

	{
		Shaders.AtomProjectile = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_atom_projectile_vs;
		params.VertexShader.Size = sizeof(HLSL_atom_projectile_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.AtomProjectile->CreateShadersFromParams(params);
	}

	{
		Shaders.BloodTrail_UnderWater = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_blood_trail_underwater_vs;
		params.VertexShader.Size = sizeof(HLSL_blood_trail_underwater_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_particle_ps;
		params.PixelShader.Size = sizeof(HLSL_particle_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.BloodTrail_UnderWater->CreateShadersFromParams(params);
	}

	{
		Shaders.Snow = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_snow_vs;
		params.VertexShader.Size = sizeof(HLSL_snow_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_snow_ps;
		params.PixelShader.Size = sizeof(HLSL_snow_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.Snow->CreateShadersFromParams(params);
	}
	
	{
		Shaders.LightDepth = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_light_depth_vs;
		params.VertexShader.Size = sizeof(HLSL_light_depth_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.InputLayout.Elements.push_back(position);
		params.InputLayout.Elements.push_back(texcoord);

		params.PixelShader.Data = (void*)HLSL_light_depth_ps;
		params.PixelShader.Size = sizeof(HLSL_light_depth_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.LightDepth->CreateShadersFromParams(params);
	}

	{
		Shaders.WorldLit = CreateShaderProgram();
		Shaders.WorldWireframe = CreateShaderProgram();
		Shaders.WorldUnlit = CreateShaderProgram();
		Shaders.WorldNormal = CreateShaderProgram();
		Shaders.WorldNormalTex = CreateShaderProgram();
		Shaders.WorldLightmap = CreateShaderProgram();
		Shaders.Water = CreateShaderProgram();
		Shaders.Portal = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_test_vs;
		params.VertexShader.Size = sizeof(HLSL_test_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.InputLayout.Elements.push_back(position);
		params.InputLayout.Elements.push_back(texcoord);
		params.InputLayout.Elements.push_back(normal);
		params.InputLayout.Elements.push_back(lightcoord);
		params.InputLayout.Elements.push_back(coordclamp);
		params.InputLayout.Elements.push_back(lightcolor);
		params.InputLayout.Elements.push_back(bufferindex);
		
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_test_lightmapped_ps;
		params.PixelShader.Size = sizeof(HLSL_test_lightmapped_ps);
		Shaders.WorldLit->CreateShadersFromParams(params);

		params.PixelShader.Data = (void*)HLSL_test_unlit_ps;
		params.PixelShader.Size = sizeof(HLSL_test_unlit_ps);
		Shaders.WorldUnlit->CreateShadersFromParams(params);

		params.PixelShader.Data = (void*)HLSL_test_wireframe_ps;
		params.PixelShader.Size = sizeof(HLSL_test_wireframe_ps);
		Shaders.WorldWireframe->CreateShadersFromParams(params);

		params.PixelShader.Data = (void*)HLSL_test_norm_ps;
		params.PixelShader.Size = sizeof(HLSL_test_norm_ps);
		Shaders.WorldNormal->CreateShadersFromParams(params);

		params.PixelShader.Data = (void*)HLSL_test_norm_tex_ps;
		params.PixelShader.Size = sizeof(HLSL_test_norm_tex_ps);
		Shaders.WorldNormalTex->CreateShadersFromParams(params);

		params.PixelShader.Data = (void*)HLSL_world_lightmap_ps;
		params.PixelShader.Size = sizeof(HLSL_world_lightmap_ps);
		Shaders.WorldLightmap->CreateShadersFromParams(params);

		params.PixelShader.Data = (void*)HLSL_test_portal_ps;
		params.PixelShader.Size = sizeof(HLSL_test_portal_ps);
		Shaders.Portal->CreateShadersFromParams(params);

		params.PixelShader.Data = (void*)HLSL_water_ps;
		params.PixelShader.Size = sizeof(HLSL_water_ps);
		Shaders.Water->CreateShadersFromParams(params);

		Shaders.ActiveWorld = Shaders.WorldLit.get();
	}

	{
		Shaders.LeakLine = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_leak_line_vs;
		params.VertexShader.Size = sizeof(HLSL_leak_line_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.InputLayout.Elements.push_back(position);

		params.PixelShader.Data = (void*)HLSL_leak_line_ps;
		params.PixelShader.Size = sizeof(HLSL_leak_line_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.LeakLine->CreateShadersFromParams(params);
	}

	{
		Program7 = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_test_portal_vs;
		params.VertexShader.Size = sizeof(HLSL_test_portal_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.InputLayout.Elements.push_back(position);

		params.PixelShader.Data = (void*)HLSL_test_portal_ps;
		params.PixelShader.Size = sizeof(HLSL_test_portal_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Program7->CreateShadersFromParams(params);
	}

	{
		Shaders.TextureDownscale = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.Data = (void*)HLSL_texture_downscale_cs;
		params.ComputeShader.Size = sizeof(HLSL_texture_downscale_cs);
		params.ComputeShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.TextureDownscale->CreateShadersFromParams(params);
	}

	{
		Shaders.VerticalBlur = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.Data = (void*)HLSL_blur_vertical_cs;
		params.ComputeShader.Size = sizeof(HLSL_blur_vertical_cs);
		params.ComputeShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.VerticalBlur->CreateShadersFromParams(params);
	}

	{
		Shaders.HorizontalBlur = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.Data = (void*)HLSL_blur_horizontal_cs;
		params.ComputeShader.Size = sizeof(HLSL_blur_horizontal_cs);
		params.ComputeShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.HorizontalBlur->CreateShadersFromParams(params);
	}

	{
		Shaders.LightGBufferCS = CreateShaderProgram();
		FShaderProgramCreateParams params;


		params.ComputeShader.Data = (void*)HLSL_light_gbuffer_cs;
		params.ComputeShader.Size = sizeof(HLSL_light_gbuffer_cs);
		params.ComputeShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.LightGBufferCS->CreateShadersFromParams(params);
	}

	{
		Shaders.LightCompositeCS = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.Data = (void*)HLSL_light_composite_cs;
		params.ComputeShader.Size = sizeof(HLSL_light_composite_cs);
		params.ComputeShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.LightCompositeCS->CreateShadersFromParams(params);
	}

	{
		Shaders.LightGBuffer = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_light_gbuffer_ps;
		params.PixelShader.Size = sizeof(HLSL_light_gbuffer_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.LightGBuffer->CreateShadersFromParams(params);
	}

	{
		Shaders.LightComposite = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.Data = (void*)HLSL_fullscreen_quad_vs;
		params.VertexShader.Size = sizeof(HLSL_fullscreen_quad_vs);
		params.VertexShader.LoadMethod = EShaderLoadMethod::Precompiled;

		params.PixelShader.Data = (void*)HLSL_light_composite_ps;
		params.PixelShader.Size = sizeof(HLSL_light_composite_ps);
		params.PixelShader.LoadMethod = EShaderLoadMethod::Precompiled;

		Shaders.LightComposite->CreateShadersFromParams(params);
	}

}

#endif
#endif
