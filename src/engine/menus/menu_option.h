#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "../math/math.h"
#include <mutex>
#include "../render/font_usage.h"

struct KMenuOption
{
	KString Text;
	bool bActive = true;
	bool bSelectable = true;
	EFontUsage FontUsage = MenuButtonLarge;
	std::function<void()> RunFunction;
	KString ConsoleCommand;

	class KMenuOption_CheckBox* RequiredCheckBox = nullptr;
	u8 bRequiredCheckBoxSetting = true;

	virtual void ExecuteOption() 
	{
		if (RunFunction) RunFunction();
	}

	virtual void OnRightArrow(i32 jump = 1) {}
	virtual void OnLeftArrow(i32 jump = 1) {}
	virtual void OnHome() {}
	virtual void OnEnd() {}

	bool CanBeSelected();
};

struct KMenuOption_CheckBox : public KMenuOption
{
	u8* Value = nullptr;

	KString Enabled = "Enabled";
	KString Disabled = "Disabled";
	u32 EnabledLayout = 0;
	u32 DisabledLayout = 0;

	void ExecuteOption() override
	{
		*Value = *Value == 0 ? 1 : 0;
		RunFunction();
	}

	void OnRightArrow(i32 jump = 1) override { ExecuteOption(); }
	void OnLeftArrow(i32 jump = 1) override { ExecuteOption(); }
};

struct KMenuOption_RadioButton : public KMenuOption
{
	TVector<KString> Options;
};

struct KMenuOption_Slider : public KMenuOption
{
	std::mutex Mutex; // lock for render

	u32 ValueLayout = 0; // hud text
	bool bPendingValueLayoutUpdate = true; // hud needs to render new layout
	f32 SliderMin = 0, SliderMax = 1, Step = .1f;
	f32 ActualMin = 0, ActualMax = 1;
	f32* Value = nullptr;
	f32 LastRenderedValue = -99999;
	bool bForceSnap = false, bForceClampOnSlide = false;
	bool bForceClampMax = false, bForceClampMin = false;
	bool bForceInt = false;

	void ClampToRange();
	void RoundToSnap();
	bool IsOnSnap();
	void OnRightArrow(i32 jump = 1);
	void OnLeftArrow(i32 jump = 1);
	void OnHome();
	void OnEnd();
	void AdjustValue(f32 amount);
	void SetValue(f32 val, bool force = false);
};

#endif