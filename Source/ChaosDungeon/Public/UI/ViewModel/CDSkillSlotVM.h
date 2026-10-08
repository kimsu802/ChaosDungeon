#pragma once

#include "MVVMViewModelBase.h"
#include "GameplayTagContainer.h"
#include "Containers/Ticker.h"
#include "CDSkillSlotVM.generated.h"

class UTexture2D;
class UCDAbilitySystemComponent;
class UEnhancedInputUserSettings;

/**
 * 스킬/회피 슬롯 1칸. inputTag 하나로 "이 칸에 무엇이 있는지"를 ASC 에 물어본다.
 * 쿨다운은 매 프레임 확인하고(채우기 연출이 끊기지 않도록), 값이 바뀔 때만 View 에 알린다.
 * 키 표시(keyText)는 키 매핑 설정이 바뀔 때마다 갱신된다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDSkillSlotVM : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// UObject::BeginDestroy()
	virtual void BeginDestroy() override;

	/** ASC 와 슬롯 입력 태그 연결 */
	void BindTo(UCDAbilitySystemComponent* abilitySystem, FGameplayTag inInputTag);

	/** 연결 해제 */
	void Unbind();

	/** 키 매핑 연결: mappingName 은 이 슬롯 입력 액션의 Player Mappable Key Settings 이름 */
	void BindKey(UEnhancedInputUserSettings* userSettings, FName mappingName);

	/** 키 매핑 연결 해제 */
	void UnbindKey();

	/** 슬롯 클릭: 키 입력과 같은 경로로 이 슬롯의 스킬 발동 */
	UFUNCTION(BlueprintCallable)
	void Activate();

	/** 드래그 앤 드롭: 두 슬롯의 스킬 교체 (Input.Skill.* 끼리만) */
	UFUNCTION(BlueprintCallable)
	void SwapWith(UCDSkillSlotVM* other);

	/** 슬롯 입력 태그 */
	FGameplayTag GetInputTag() const
	{
		return inputTag;
	}

	/** 아이콘 */
	UTexture2D* GetIcon() const
	{
		return icon;
	}

	/** 쿨다운 비율 (1 → 0) */
	float GetCooldownPercent() const
	{
		return cooldownPercent;
	}

	/** 남은 쿨다운 텍스트 */
	FText GetCooldownText() const
	{
		return cooldownText;
	}

	/** 현재 바인딩된 키 표시 */
	FText GetKeyText() const
	{
		return keyText;
	}

private:
	/** 슬롯 스킬의 아이콘 갱신 */
	void RefreshIcon();

	/** 쿨다운 확인 (FTSTicker) */
	bool TickCooldown(float deltaTime);

	/** 현재 매핑된 키로 keyText 갱신 */
	void RefreshKeyText();

	/** 키 매핑 설정 변경 알림 (UEnhancedInputUserSettings::OnSettingsChanged) */
	UFUNCTION()
	void HandleKeySettingsChanged(UEnhancedInputUserSettings* settings);

private:
	/** 회피 슬롯처럼 스킬 데이터가 없으면 null → View 가 기본 아이콘 사용 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIcon, meta = (AllowPrivateAccess = true))
	TObjectPtr<UTexture2D> icon;

	/** 쿨다운 비율 (1 → 0) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetCooldownPercent, meta = (AllowPrivateAccess = true))
	float cooldownPercent = 0.f;

	/** 쿨다운이 아니면 빈 텍스트 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetCooldownText, meta = (AllowPrivateAccess = true))
	FText cooldownText;

	/** 바인딩된 키 이름 (예: Q). 매핑이 없으면 빈 텍스트 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetKeyText, meta = (AllowPrivateAccess = true))
	FText keyText;

	/** 연결된 ASC */
	TWeakObjectPtr<UCDAbilitySystemComponent> boundAbilitySystem;

	/** 슬롯 입력 태그 */
	FGameplayTag inputTag;

	/** 쿨다운 티커 핸들 */
	FTSTicker::FDelegateHandle tickerHandle;

	/** 키 매핑 설정 */
	TWeakObjectPtr<UEnhancedInputUserSettings> boundKeySettings;

	/** 이 슬롯 입력 액션의 키 매핑 이름 */
	FName keyMappingName;
};
