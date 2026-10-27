// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ListData/CDOptionData_Scalar.h"

void UCDOptionData_Scalar::InitScalar(TFunction<float()>&& inGetter, TFunction<void(float)>&& inSetter, float inMin, float inMax, float inStep, float inDefault, bool bInDisplayAsPercent)
{
	getter = MoveTemp(inGetter);
	setter = MoveTemp(inSetter);
	minValue = inMin;
	maxValue = FMath::Max(inMax, inMin + KINDA_SMALL_NUMBER);
	stepSize = FMath::Max(inStep, 0.f);
	defaultValue = inDefault;
	bDisplayAsPercent = bInDisplayAsPercent;
}

float UCDOptionData_Scalar::GetValue() const
{
	return getter ? getter() : defaultValue;
}

void UCDOptionData_Scalar::SetValue(float newValue)
{
	newValue = FMath::Clamp(newValue, minValue, maxValue);
	if (stepSize > 0.f)
	{
		newValue = minValue + (FMath::RoundToFloat((newValue - minValue) / stepSize) * stepSize);
		newValue = FMath::Clamp(newValue, minValue, maxValue);
	}

	if (FMath::IsNearlyEqual(newValue, GetValue()))
	{
		return;
	}

	if (setter)
	{
		setter(newValue);
	}
	NotifyValueChanged();
}

float UCDOptionData_Scalar::GetNormalizedValue() const
{
	return (GetValue() - minValue) / (maxValue - minValue);
}

void UCDOptionData_Scalar::SetNormalizedValue(float normalized)
{
	SetValue(FMath::Lerp(minValue, maxValue, FMath::Clamp(normalized, 0.f, 1.f)));
}

float UCDOptionData_Scalar::GetNormalizedStep() const
{
	return stepSize / (maxValue - minValue);
}

FText UCDOptionData_Scalar::GetValueText() const
{
	const float value = GetValue();
	if (bDisplayAsPercent)
	{
		return FText::AsPercent(value);
	}

	FNumberFormattingOptions options;
	options.MaximumFractionalDigits = (stepSize >= 1.f) ? 0 : 2;
	return FText::AsNumber(value, &options);
}

void UCDOptionData_Scalar::ResetToDefault()
{
	SetValue(defaultValue);
}
