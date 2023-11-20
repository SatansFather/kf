#if !_SERVER && !_COMPILER

#include "hud_damage_numbers.h"
#include "../interface/render_interface.h"
#include "../color.h"

KHudDamageNumbers::KHudDamageNumbers()
{

}

void KHudDamageNumbers::Draw()
{
	KRenderInterface* iface = GetRenderInterface();
	
	f32 scaledY = GetScaledY();

	for (const KDamageNumber& num : iface->LiveDamageNumbers)
	{
		i32 val = num.Damage;
		if (val == 0) continue;
		bool heal = val < 0;
		val = abs(val);

		u32 layout = 0;
		if (!DamageToLayoutMap.contains(val))
		{
			layout = iface->HUD_CreateTextLayout(KString(val), EFontUsage::DamageNumber, 1000, 1000);
			DamageToLayoutMap[val] = layout;
		}
		else
		{
			layout = DamageToLayoutMap[val];
		}
		const f32 w = iface->HUD_GetTextWidth(layout);
		const f32 h = iface->HUD_GetTextHeight(layout);
		const f32 age = iface->GetTotalRenderTime() - num.RenderTimeCreated;
		const f32 xRand = (f32(num.RandomSeed) - 128) / 128.f;
		const f32 yRand = MapRange(f32(num.RandomSeed * 2) / 255.f, 0, 1, .7, 1);
		const f32 xMove = xRand * (scaledY * 45);
		const f32 yMove = yRand * (scaledY * 45);
		const f32 lifeAlpha = age / num.Lifespan;
		const glm::vec2 pos = iface->ProjectWorldToScreen((num.Position + ((f32)num.RandomSeed / 32.f)).ToGLM());

		f32 opacity = KSaturate(f32(num.FramesRemaining) / 20.f) * 180;
		opacity *= pow(KSaturate(age * 6), 2);

		const f32 sizeAlpha = KSaturate(sin(2.f * PI<f32>() * KSaturate(age * 3) + PI<f32>() / 2.f) / 2.f + .5);
		const f32 fontSize = KLerp(28, 20, sizeAlpha);
		iface->HUD_SetTextLayoutFontSize(layout, fontSize * scaledY);

		const f32 fontScale = fontSize / 20;
		const f32 wDiff = fontScale * w - w;
		const f32 hDiff = fontScale * h - h;

		KHudPointF p;
		p.x = pos.x + xMove * sqrt(age) - (w / 2);
		p.y = pos.y - yMove * sqrt(age) - (h / 2);
		p.x -= wDiff / 2;
		p.y -= hDiff / 2;
		DrawTextShadowed(layout, p, scaledY * 2, FColor8(255, 255, 255, opacity), FColor8(40, 40, 40, opacity));
	}
}

void KHudDamageNumbers::OnWindowResize()
{

}

#endif