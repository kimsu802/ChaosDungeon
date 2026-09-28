#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CDAttributeSet.generated.h"

struct FCDBaseStats;

/**
 * 속성 접근자 생성 매크로.
 * 엔진 기본 매크로(ATTRIBUTE_ACCESSORS)는 변수 이름이 그대로 함수 이름이 되므로,
 * 변수(camelCase)와 함수(PascalCase) 이름을 따로 받는다.
 *   CD_ATTRIBUTE_ACCESSORS(UCDAttributeSet, health, Health)
 *   → GetHealthAttribute(), GetHealth(), SetHealth(), InitHealth()
 */
#define CD_ATTRIBUTE_ACCESSORS(ClassName, PropertyName, FuncName) \
	static FGameplayAttribute Get##FuncName##Attribute() \
	{ \
		static FProperty* property = FindFieldChecked<FProperty>(ClassName::StaticClass(), GET_MEMBER_NAME_CHECKED(ClassName, PropertyName)); \
		return property; \
	} \
	FORCEINLINE float Get##FuncName() const \
	{ \
		return PropertyName.GetCurrentValue(); \
	} \
	FORCEINLINE void Set##FuncName(float newValue) \
	{ \
		UAbilitySystemComponent* abilitySystem = GetOwningAbilitySystemComponent(); \
		if (ensure(abilitySystem)) \
		{ \
			abilitySystem->SetNumericAttributeBase(Get##FuncName##Attribute(), newValue); \
		} \
	} \
	FORCEINLINE void Init##FuncName(float newValue) \
	{ \
		PropertyName.SetBaseValue(newValue); \
		PropertyName.SetCurrentValue(newValue); \
	}

DECLARE_MULTICAST_DELEGATE_ThreeParams(FCDDamageTakenDelegate, AActor* /*instigator*/, float /*damage*/, bool /*bCritical*/);

/**
 * 플레이어/몬스터 공용 속성.
 * 데미지는 메타 속성(incomingDamage)으로 받아 여기서만 health 에 반영한다. → 체력이 깎이는 경로가 한 곳
 */
UCLASS()
class CHAOSDUNGEON_API UCDAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	CD_ATTRIBUTE_ACCESSORS(UCDAttributeSet, health, Health)
	CD_ATTRIBUTE_ACCESSORS(UCDAttributeSet, maxHealth, MaxHealth)
	CD_ATTRIBUTE_ACCESSORS(UCDAttributeSet, attackPower, AttackPower)
	CD_ATTRIBUTE_ACCESSORS(UCDAttributeSet, defense, Defense)
	CD_ATTRIBUTE_ACCESSORS(UCDAttributeSet, critChance, CritChance)
	CD_ATTRIBUTE_ACCESSORS(UCDAttributeSet, critDamage, CritDamage)
	CD_ATTRIBUTE_ACCESSORS(UCDAttributeSet, incomingDamage, IncomingDamage)

	/** 스탯 초기화. 난이도 배율은 몬스터에만 적용 */
	void InitFromStats(const FCDBaseStats& stats, float healthMultiplier = 1.f, float attackMultiplier = 1.f);

protected:
	// UAttributeSet::PreAttributeChange()
	virtual void PreAttributeChange(const FGameplayAttribute& attribute, float& newValue) override;

	// UAttributeSet::PostGameplayEffectExecute()
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& data) override;

public:
	/** 피해를 받았을 때 (가해자, 피해량, 치명타) */
	FCDDamageTakenDelegate onDamageTaken;

	/** 체력이 0 이 되었을 때 */
	FSimpleMulticastDelegate onOutOfHealth;

protected:
	/** 현재 체력 */
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData health;

	/** 최대 체력 */
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData maxHealth;

	/** 공격력 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData attackPower;

	/** 받는 피해 감소 비율 (0 ~ 0.9) */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData defense;

	/** 치명타 확률 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData critChance;

	/** 치명타 피해 배율 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData critDamage;

	/** 메타 속성: 데미지 계산 결과를 잠깐 담는 용도 */
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData incomingDamage;
};
