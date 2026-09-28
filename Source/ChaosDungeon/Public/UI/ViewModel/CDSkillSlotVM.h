#pragma once

#include "MVVMViewModelBase.h"
#include "GameplayTagContainer.h"
#include "Containers/Ticker.h"
#include "CDSkillSlotVM.generated.h"

class UTexture2D;
class UCDAbilitySystemComponent;

/**
 * 스킬/회피 슬롯 1칸. inputTag 하나로 "이 칸에 무엇이 있는지"를 ASC 에 물어본다.
 * 쿨다운은 0.1초 간격으로 확인하고, 값이 바뀔 때만 View 에 알린다.
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

private:
	/** 슬롯 스킬의 아이콘 갱신 */
	void RefreshIcon();

	/** 쿨다운 확인 (FTSTicker) */
	bool TickCooldown(float deltaTime);

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

	/** 연결된 ASC */
	TWeakObjectPtr<UCDAbilitySystemComponent> boundAbilitySystem;

	/** 슬롯 입력 태그 */
	FGameplayTag inputTag;

	/** 쿨다운 티커 핸들 */
	FTSTicker::FDelegateHandle tickerHandle;
};
