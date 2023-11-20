#if !_SERVER

#include "menu_option.h"

bool KMenuOption::CanBeSelected()
{	
	return bActive && bSelectable && (!RequiredCheckBox || *(RequiredCheckBox->Value) == bRequiredCheckBoxSetting);
}

void KMenuOption_Slider::ClampToRange()
{
	*Value = KClamp(*Value, SliderMin, SliderMax);
}

void KMenuOption_Slider::RoundToSnap()
{
	*Value = RoundNearest(*Value, Step);
}

bool KMenuOption_Slider::IsOnSnap()
{
	f32 v = *Value;
	v = RoundNearest(v, Step);
	return abs(*Value - v) < .0001;
}

void KMenuOption_Slider::OnRightArrow(i32 jump)
{
	AdjustValue(Step * jump);
}

void KMenuOption_Slider::OnLeftArrow(i32 jump)
{
	AdjustValue(-Step * jump);
}

void KMenuOption_Slider::OnHome()
{
	Mutex.lock();
	f32 preVal = *Value;
	*Value = SliderMin;
	bPendingValueLayoutUpdate = preVal != *Value;
	Mutex.unlock();
	ExecuteOption();
}

void KMenuOption_Slider::OnEnd()
{
	Mutex.lock();
	f32 preVal = *Value;
	*Value = SliderMax;
	bPendingValueLayoutUpdate = preVal != *Value;
	Mutex.unlock();
	ExecuteOption();
}

void KMenuOption_Slider::AdjustValue(f32 amount)
{
	Mutex.lock();
	f32 preVal = *Value;

	if (!IsOnSnap())
	{
		f32 preVal = *Value;
		RoundToSnap();
		if (*Value < preVal && amount > 0)
			*Value += Step;
		else if (*Value > preVal && amount < 0)
			*Value -= Step;
	}
	else
	{
		*Value += amount;
		RoundToSnap();
	}
	ClampToRange();
	if (bForceInt) *Value = std::round(*Value);
	bPendingValueLayoutUpdate = preVal != *Value;
	Mutex.unlock();
	ExecuteOption();
}

void KMenuOption_Slider::SetValue(f32 val, bool force /*= false*/)
{
	Mutex.lock();
	f32 preVal = *Value;
	*Value = val;
	if (bForceSnap || force) RoundToSnap();
	if (bForceClampOnSlide || force) ClampToRange();
	if (bForceInt) *Value = std::round(*Value);
	bPendingValueLayoutUpdate = preVal != *Value;
	Mutex.unlock();
	ExecuteOption();
}

#endif