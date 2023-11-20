#include "face_attribs.h"
#include "../entity.h"
#include "engine/system/terminal/terminal.h"
#include "compiler/compiler.h"

KFaceAttribs* KFaceAttribs::CreateNewAttribs(class KMapEntity* ent)
{
	if (ent->GetProperty("classname") == "face_attribs")
	{
		UPtr<KFaceAttribs> attribs;
		KFaceAttribs* att = nullptr;

		KString entID = ent->GetProperty("entity_id");
		{
			i32 id = 0;
			entID.ToI32Safe(id);
			if (id == 0 || entID.IsEmpty())
			{
				return nullptr;
			}
			else
			{
				attribs = std::make_unique<KFaceAttribs>();
				att = attribs.get();
				KMapCompiler::Get().FaceAttribMap[id] = std::move(attribs);
			}	
		}
		KString shader = ent->GetProperty("shader");
		{
			att->Shader = shader;
		}
		KString gi_fog_scale = ent->GetProperty("gi_fog_scale");
		{
			TVector<f32> arr = gi_fog_scale.ToFloatArray();
			if (arr.size() >= 3)
			{
				att->RedGIFogScale = arr[0];
				att->GreenGIFogScale = arr[1];
				att->BlueGIFogScale = arr[2];
			}
			if (arr.size() == 4)
			{
				att->AlphaGIFogScale = arr[3];
			}
		}
		KString gi_sky_scale = ent->GetProperty("gi_sky_scale");
		{
			TVector<f32> arr = gi_sky_scale.ToFloatArray();
			if (arr.size() >= 3)
			{
				att->RedGISkyScale = arr[0];
				att->GreenGISkyScale = arr[1];
				att->BlueGISkyScale = arr[2];
			}
			if (arr.size() == 4)
			{
				att->AlphaGISkyScale = arr[3];
			}
		}
		KString gi_mod_ids = ent->GetProperty("gi_mod_ids");
		{
			const auto IsNumeric_Int = [&](char c) -> bool
			{
				return
					c == '0' ||
					c == '1' ||
					c == '2' ||
					c == '3' ||
					c == '4' ||
					c == '5' ||
					c == '6' ||
					c == '7' ||
					c == '8' ||
					c == '9';
			};

			const auto IsNumeric = [&](char c) -> bool
			{
				return IsNumeric_Int(c) || c == '.' || c == '-';
			};

			enum class ParseState : u8
			{
				FaceID, FaceMod, None
			};

			ParseState parse_state = ParseState::None;
			bool parsing_color = false;
			i16 error = -1;
			i16 charindex = 0;

			KString current_face_str = "";
			KString current_mod_str = "";
			TMap<KString, KString> face_mod;

			for (char c : gi_mod_ids)
			{
				switch (parse_state)
				{
					case ParseState::None:
					{
						if (c == ' ') continue;

						// expect a face id
						if (IsNumeric_Int(c))
						{
							if (current_face_str != "")
							{
								face_mod[current_face_str] = current_mod_str;
								current_face_str = "";
								current_mod_str = "";
							}

							parse_state = ParseState::FaceID;
							current_face_str += c;
							continue;
						}
						else
						{
							error = charindex;
							break;
						}
					}
					case ParseState::FaceID:
					{
						if (c == '(')
						{
							i32 id_int;
							if (parse_state == ParseState::FaceID && current_face_str.ToI32Safe(id_int))
							{
								parse_state = ParseState::FaceMod;
								continue;
							}
							else
							{
								error = charindex;
								break;
							}
						}

						if (c == ' ')
						{
							parse_state = ParseState::None;
							continue; // will store face next loop
						}

						if (IsNumeric_Int(c))
						{
							current_face_str += c;
							continue;
						}

						error = charindex;
						break;
					}
					case ParseState::FaceMod:
					{
						// add everything to this string, parse it later
						if (c != ')')
						{
							if (IsNumeric(c) || c == '+' || c == '-')
							{
								current_mod_str += c;
								continue;
							}
							else
							{
								error = charindex;
								break;
							}
						}
						else
						{
							parse_state = ParseState::None;
							continue;
						}
					}

				}
				if (error) break;
				charindex++;
			}

			// store final face data if we have it
			if (current_face_str != "")
			{
				face_mod[current_face_str] = current_mod_str;
				current_face_str = "";
				current_mod_str = "";
			}

			if (error < 0)
			{
				for (auto& kv : face_mod)
				{
					i32 id = 0;
					if (kv.first.ToI32Safe(id))
					{
						KFaceAttribs::GI_Mod mod;
						KString	modstr = kv.second;

						if (modstr.Contains("+"))
						{
							mod.ModFlags |= KFaceAttribs::GI_Mod::Unlit;
							std::replace(modstr.GetMut().begin(), modstr.GetMut().end(), '+', ' ');
						}

						if (modstr.Contains("-"))
						{
							mod.ModFlags |= KFaceAttribs::GI_Mod::Ignore;
							std::replace(modstr.GetMut().begin(), modstr.GetMut().end(), '-', ' ');
						}

						TVector<f32> color = modstr.ToFloatArray();

						if (color.size() >= 3)
						{
							mod.R = color[0];
							mod.G = color[1];
							mod.B = color[2];
						}

						if (color.size() == 4)
						{
							mod.A = color[3];
						}

						att->GIMod[id] = mod;
					}
				}
			}
			else
			{
				SYSLOG_ERROR("face_attribs entity with invalid gi_mod_ids value. Error at character #" << KString(error));
				SYSLOG_ERROR("Value: " << gi_mod_ids);
			}
		}
		KString light_position_offset = ent->GetProperty("light_position_offset");
		if (!light_position_offset.IsEmpty())
		{
			light_position_offset.ToFloatSafe(att->LightPositionOffset);
		}
		KString ignore_from_ids = ent->GetProperty("ignore_from_ids");
		if (!ignore_from_ids.IsEmpty());
		{
			TVector<f32> arr = ignore_from_ids.ToFloatArray();
			for (f32 f : arr)
			{
				i32 i = f;
				att->IgnoreFrom.push_back(i);
			}
		}
		KString ignore_to_ids = ent->GetProperty("ignore_to_ids");
		if (!ignore_to_ids.IsEmpty())
		{
			TVector<f32> arr = ignore_to_ids.ToFloatArray();
			for (f32 f : arr)
			{
				i32 i = f;
				att->IgnoreTo.push_back(i);
			}
		}
		KString masked_shadows = ent->GetProperty("masked_shadows");
		if (!masked_shadows.IsEmpty())
		{
			i32 i = 0;
			masked_shadows.ToI32Safe(i);
			if (i != 0) att->Flags |= KFaceAttribs::MaskedShadows;
		}
		KString masked_shadow_coloring = ent->GetProperty("masked_shadow_coloring");
		if (!masked_shadow_coloring.IsEmpty())
		{
			i32 i = 0;
			masked_shadow_coloring.ToI32Safe(i);
			if (i != 0) att->Flags |= KFaceAttribs::MaskedShadowColoring;
		}
		KString rgb_pass = ent->GetProperty("rgb_pass");
		if (!rgb_pass.IsEmpty())
		{
			TVector<f32> arr = rgb_pass.ToFloatArray();
			if (arr.size() == 3)
			{
				att->RedPass = arr[0];
				att->GreenPass = arr[1];
				att->BluePass = arr[2];
			}
			else
			{
				SYSLOG_ERROR("rgb_pass value on face_attribs has invalid value: " + rgb_pass);
				SYSLOG_ERROR("Must have 3 valid numbers");
			}
		}
		KString rgb_scale = ent->GetProperty("rgb_scale");
		if (!rgb_scale.IsEmpty())
		{
			TVector<f32> arr = rgb_scale.ToFloatArray();
			if (arr.size() == 3)
			{
				att->RedIntensity = arr[0];
				att->GreenIntensity = arr[1];
				att->BlueIntensity = arr[2];
			}
			else
			{
				SYSLOG_ERROR("rgb_scale value on face_attribs has invalid value: " + rgb_scale);
				SYSLOG_ERROR("Must have 3 valid numbers");
			}
		}
		KString diffuse_shadow_strength = ent->GetProperty("diffuse_shadow_strength");
		if (!diffuse_shadow_strength.IsEmpty())
		{
			TVector<f32> arr = diffuse_shadow_strength.ToFloatArray();
			if (arr.size() == 3)
			{
				att->RedDiffuseShadow = arr[0];
				att->GreenDiffuseShadow = arr[1];
				att->BlueDiffuseShadow = arr[2];
			}
			else
			{
				SYSLOG_ERROR("diffuse_shadow_strength value on face_attribs has invalid value: " + diffuse_shadow_strength);
				SYSLOG_ERROR("Must have 3 valid numbers");
			}
		}
		KString extra_smoothing_groups = ent->GetProperty("extra_smoothing_groups");
		if (!extra_smoothing_groups.IsEmpty())
		{
			TVector<i32> arr = extra_smoothing_groups.ToNumArray<i32>();
			for (i32 i : arr)
			{
				att->ExtraSmoothingGroups.push_back(i);
			}
		}

		return att;
	}

	return nullptr;
}

