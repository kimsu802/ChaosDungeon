#pragma once

#include "MVVMViewModelBase.h"
#include "CDHealthVM.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

/** HP 바. 플레이어 HP, 보스 HP 가 같은 클래스의 인스턴스 2개를 쓴다 */
UCLASS()
class CHAOSDUNGEON_API UCDHealthVM : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** ASC 의 체력 변경을 구독 */
	void BindTo(UAbilitySystemComponent* abilitySystem);

	/** 구독 해제 */
	void Unbind();

	/** 현재 체력 */
	float GetCurrentHealth() const
	{
		return currentHealth;
	}

	/** 최대 체력 */
	float GetMaxHealth() const
	{
		return maxHealth;
	}

	/** 체력 비율 (0 ~ 1) */
	UFUNCTION(BlueprintPure, FieldNotify)
	float GetHealthPercent() const
	{
		return maxHealth > 0.f ? currentHealth / maxHealth : 0.f;
	}

private:
	/** 현재 체력 갱신 + 비율 알림 */
	void SetCurrentHealth(float value);

	/** 최대 체력 갱신 + 비율 알림 */
	void SetMaxHealth(float value);

	/** Health 속성 변경 콜백 */
	void HandleHealthChanged(const FOnAttributeChangeData& data);

	/** MaxHealth 속성 변경 콜백 */
	void HandleMaxHealthChanged(const FOnAttributeChangeData& data);

private:
	/** 현재 체력 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetCurrentHealth, meta = (AllowPrivateAccess = true))
	float currentHealth = 0.f;

	/** 최대 체력 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetMaxHealth, meta = (AllowPrivateAccess = true))
	float maxHealth = 0.f;

	/** 구독 중인 ASC */
	TWeakObjectPtr<UAbilitySystemComponent> boundAbilitySystem;

	/** Health 구독 핸들 */
	FDelegateHandle healthHandle;

	/** MaxHealth 구독 핸들 */
	FDelegateHandle maxHealthHandle;
};
