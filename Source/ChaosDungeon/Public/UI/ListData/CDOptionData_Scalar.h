// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CDOptionData.h"
#include "CDOptionData_Scalar.generated.h"

/**
 * 슬라이더 항목 (볼륨 등). 값은 min ~ max, step 단위로 맞춘다.
 * 표시는 bDisplayAsPercent 면 "80%", 아니면 숫자 그대로.
 */
UCLASS()
class CHAOSDUNGEON_API UCDOptionData_Scalar : public UCDOptionData
{
	GENERATED_BODY()

public:
	/** 범위 · 기본값 · 실제 설정을 읽고 쓰는 함수 연결 */
	void InitScalar(TFunction<float()>&& inGetter, TFunction<void(float)>&& inSetter, float inMin, float inMax, float inStep, float inDefault, bool bInDisplayAsPercent = true);

	/** 현재 값 */
	float GetValue() const;

	/** 값 변경 (범위 · step 으로 맞춘 뒤 설정에 쓰고 알림) */
	void SetValue(float newValue);

	/** 0 ~ 1 로 바꾼 현재 값 (슬라이더용) */
	float GetNormalizedValue() const;

	/** 0 ~ 1 값으로 변경 (슬라이더용) */
	void SetNormalizedValue(float normalized);

	/** 0 ~ 1 기준 step (슬라이더 Step Size) */
	float GetNormalizedStep() const;

	/** 표시용 글자 ("80%") */
	FText GetValueText() const;

	// UCDOptionData::ResetToDefault()
	virtual void ResetToDefault() override;

private:
	/** 실제 설정 읽기 */
	TFunction<float()> getter;

	/** 실제 설정 쓰기 */
	TFunction<void(float)> setter;

	/** 최소값 */
	float minValue = 0.f;

	/** 최대값 */
	float maxValue = 1.f;

	/** 값 단위 */
	float stepSize = 0.05f;

	/** 기본값 */
	float defaultValue = 1.f;

	/** 퍼센트로 표시 */
	bool bDisplayAsPercent = true;
};
