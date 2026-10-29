#pragma once

#include "MVVMViewModelBase.h"
#include "CDAwakenGaugeVM.generated.h"

class UAbilitySystemComponent;
class UEnhancedInputUserSettings;
struct FOnAttributeChangeData;

/**
 * 각성 게이지. ASC 의 AwakenGauge / MaxAwakenGauge 속성 변경을 구독해 View 에 알린다.
 * - Global Viewmodel Collection 이름: AwakenGauge
 * - 준비 여부(IsReady)는 각성기 발동 조건(CDPlayerSkillAbility: awakenGauge == maxAwakenGauge)과 같은 기준
 * - 키 표시(keyText)는 Input.Skill.Awakening 의 키 매핑 설정이 바뀔 때마다 갱신
 */
UCLASS()
class CHAOSDUNGEON_API UCDAwakenGaugeVM : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// UObject::BeginDestroy()
	virtual void BeginDestroy() override;

	/** ASC 의 각성 게이지 변경을 구독 */
	void BindTo(UAbilitySystemComponent* abilitySystem);

	/** 구독 해제 */
	void Unbind();

	/** 키 매핑 연결: mappingName 은 각성기 입력 액션의 Player Mappable Key Settings 이름 */
	void BindKey(UEnhancedInputUserSettings* userSettings, FName mappingName);

	/** 키 매핑 연결 해제 */
	void UnbindKey();

	/** 현재 게이지 */
	float GetCurrentGauge() const
	{
		return currentGauge;
	}

	/** 최대 게이지 */
	float GetMaxGauge() const
	{
		return maxGauge;
	}

	/** 게이지 비율 (0 ~ 1) */
	UFUNCTION(BlueprintPure, FieldNotify)
	float GetGaugePercent() const
	{
		return (maxGauge > 0.f) ? FMath::Clamp(currentGauge / maxGauge, 0.f, 1.f) : 0.f;
	}

	/** 각성기 사용 가능 (게이지 가득) */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsReady() const
	{
		return (maxGauge > 0.f) && (currentGauge >= maxGauge);
	}

	/** 현재 바인딩된 키 표시 */
	FText GetKeyText() const
	{
		return keyText;
	}

private:
	/** 현재 게이지 갱신 + 비율/준비 알림 */
	void SetCurrentGauge(float value);

	/** 최대 게이지 갱신 + 비율/준비 알림 */
	void SetMaxGauge(float value);

	/** 비율 · 준비 여부 알림 */
	void BroadcastDerived();

	/** AwakenGauge 속성 변경 콜백 */
	void HandleGaugeChanged(const FOnAttributeChangeData& data);

	/** MaxAwakenGauge 속성 변경 콜백 */
	void HandleMaxGaugeChanged(const FOnAttributeChangeData& data);

	/** 현재 매핑된 키로 keyText 갱신 */
	void RefreshKeyText();

	/** 키 매핑 설정 변경 알림 (UEnhancedInputUserSettings::OnSettingsChanged) */
	UFUNCTION()
	void HandleKeySettingsChanged(UEnhancedInputUserSettings* settings);

private:
	/** 현재 게이지 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetCurrentGauge, meta = (AllowPrivateAccess = true))
	float currentGauge = 0.f;

	/** 최대 게이지 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetMaxGauge, meta = (AllowPrivateAccess = true))
	float maxGauge = 0.f;

	/** 바인딩된 키 이름 (예: Z). 매핑이 없으면 빈 텍스트 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetKeyText, meta = (AllowPrivateAccess = true))
	FText keyText;

	/** 구독 중인 ASC */
	TWeakObjectPtr<UAbilitySystemComponent> boundAbilitySystem;

	/** AwakenGauge 구독 핸들 */
	FDelegateHandle gaugeHandle;

	/** MaxAwakenGauge 구독 핸들 */
	FDelegateHandle maxGaugeHandle;

	/** 키 매핑 설정 */
	TWeakObjectPtr<UEnhancedInputUserSettings> boundKeySettings;

	/** 각성기 입력 액션의 키 매핑 이름 */
	FName keyMappingName;
};
