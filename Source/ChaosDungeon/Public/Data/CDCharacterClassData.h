#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/CDTypes.h"
#include "CDCharacterClassData.generated.h"

class UCDSkillData;
class UCDDodgeAbility;

/** 기본 스킬 배치 1칸 */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDSkillSlot
{
	GENERATED_BODY()

	/** 슬롯 입력 태그 (Input.Skill.1 ~ 4) */
	UPROPERTY(EditAnywhere, meta = (Categories = "Input.Skill"))
	FGameplayTag inputTag;

	/** 배치할 스킬 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCDSkillData> skill;
};

/**
 * 직업 1개 = 에셋 1개. 2번째 직업은 에셋 + 메시BP 추가로 확장한다.
 */
UCLASS(BlueprintType)
class CHAOSDUNGEON_API UCDCharacterClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 직업 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText className;

	/** 기본 스탯 */
	UPROPERTY(EditDefaultsOnly)
	FCDBaseStats stats;

	/** 기본 QWER 배치 (게임 중 드래그 앤 드롭으로 교체 가능) */
	UPROPERTY(EditDefaultsOnly)
	TArray<FCDSkillSlot> defaultSkills;

	/** 회피 어빌리티 (직업별 몽타주/수치는 BP 자식) */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCDDodgeAbility> dodgeAbility;
};
