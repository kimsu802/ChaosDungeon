#pragma once

#include "AbilitySystemComponent.h"
#include "CDAbilitySystemComponent.generated.h"

class UCDSkillData;

/**
 * 입력 태그 기반 어빌리티 관리.
 * - 어빌리티 Spec 의 DynamicSpecSourceTags 에 입력 태그(Input.Skill.1 등)를 달아 둔다.
 * - 입력 → 태그 → Spec 검색 → 발동. 슬롯 교체(드래그 앤 드롭)는 태그만 맞바꾸면 끝.
 */
UCLASS()
class CHAOSDUNGEON_API UCDAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/** 어빌리티 부여 (입력 태그/소스 오브젝트 선택) */
	FGameplayAbilitySpecHandle GrantAbility(TSubclassOf<UGameplayAbility> abilityClass, FGameplayTag inputTag = FGameplayTag(), UObject* sourceObject = nullptr);

	/** 스킬 데이터를 SourceObject 로 가진 어빌리티를 부여한다 */
	FGameplayAbilitySpecHandle GrantSkill(UCDSkillData* skill, FGameplayTag inputTag = FGameplayTag());

	/** 입력 태그에 연결된 어빌리티 발동 */
	void AbilityInputPressed(FGameplayTag inputTag);

	/** AI 용: 특정 스킬 발동 */
	bool TryActivateSkill(const UCDSkillData* skill);

	/** 스킬 슬롯 드래그 앤 드롭: 두 슬롯의 입력 태그를 맞바꾼다 */
	void SwapInputTags(FGameplayTag slotA, FGameplayTag slotB);

	/** UI 조회: 슬롯에 있는 스킬 */
	const UCDSkillData* GetSkillByInputTag(FGameplayTag inputTag) const;

	/** UI 조회: 슬롯 쿨다운 (없으면 false) */
	bool GetCooldownByInputTag(FGameplayTag inputTag, float& outRemaining, float& outDuration) const;

private:
	/** 입력 태그로 Spec 검색 */
	const FGameplayAbilitySpec* FindSpecByInputTag(FGameplayTag inputTag) const;

	/** 입력 태그로 Spec 검색 (수정용) */
	FGameplayAbilitySpec* FindSpecByInputTag(FGameplayTag inputTag);
};
