#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Data/CDHitShape.h"
#include "CDSkillData.generated.h"

class UAnimMontage;
class UTexture2D;
class UCDSkillAbility;

/** 판정 기준점 */
UENUM(BlueprintType)
enum class ECDSkillOrigin : uint8
{
	Self,    // 시전자 기준 (원형/부채꼴/직선/돌진)
	Cursor   // 조준 위치 기준 (장판)
};

/**
 * 스킬 1개 = 에셋 1개. 플레이어/몬스터 공용.
 * 원형/부채꼴/직선/장판/돌진 모두 이 데이터 조합으로 표현한다 → 새 스킬은 코드 수정 없이 에셋 추가.
 * 데이터로 표현할 수 없는 특수 스킬만 abilityClass 를 UCDSkillAbility 자식으로 교체한다.
 */
UCLASS(BlueprintType)
class CHAOSDUNGEON_API UCDSkillData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 표시 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText displayName;

	/** 슬롯 아이콘 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> icon;

	/** 비워두면 UCDSkillAbility */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UCDSkillAbility> abilityClass;

	/** 타격 시점에 Event.Montage.Hit, 후딜 시작에 Event.Montage.Recovery 노티파이를 넣는다 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UAnimMontage> montage;

	/** 판정 기준점 */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	ECDSkillOrigin origin = ECDSkillOrigin::Self;

	/** 장판 최대 사거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting", meta = (EditCondition = "origin == ECDSkillOrigin::Cursor"))
	float maxRange = 1000.f;

	/** 판정 도형 */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	FCDHitShape hitShape;

	/** 0보다 크면 돌진 스킬 */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float dashDistance = 0.f;

	/** 돌진 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting", meta = (EditCondition = "dashDistance > 0"))
	float dashDuration = 0.2f;

	/** 스킬 계수 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float damageCoefficient = 1.f;

	/** 피격 대상에게 경직을 주는가 (일반 몬스터 기본 공격은 false) */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	bool bCausesHitReact = true;

	/** 넉백 세기 (0 = 없음) */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float knockbackStrength = 0.f;

	/** 쿨다운(초) */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float cooldown = 5.f;

	/** 스킬마다 고유해야 한다 (예: Cooldown.Skill.Whirlwind) */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown", meta = (Categories = "Cooldown"))
	FGameplayTag cooldownTag;

	/** 후딜을 다른 스킬로 캔슬할 수 있는가 (회피 캔슬은 항상 가능) */
	UPROPERTY(EditDefaultsOnly, Category = "Cancel")
	bool bRecoveryCancelable = false;
};
