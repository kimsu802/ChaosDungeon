#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/CDTypes.h"
#include "CDCharacterClassData.generated.h"

class UCDSkillData;
class UCDDodgeAbility;
class UCDAttackAbility;
class UTexture2D;
class USkeletalMesh;
class UAnimInstance;

/** 기본 스킬 배치 1칸 */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDSkillSlot
{
	GENERATED_BODY()

	/** 입력 태그 - 10.02 Jun6 입력 태그(Input.Skill)로 원복 */
	UPROPERTY(EditAnywhere, meta = (Categories = "Input.Skill"))
	FGameplayTag inputTag;

	/** 배치할 스킬 태그 - 10.02 Jun6 스킬 구조체가 아닌 스킬 구분자 Tag로 변경*/
	UPROPERTY(EditAnywhere)
	FGameplayTag skillTag;
	//TObjectPtr<UCDSkillData> skill;
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

	// 영어 직업 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText englishClassName;

	/** 직업 선택 화면 초상화 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> portrait;

	/** 직업 선택 화면 설명 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display|Tags")
	TArray<FText> classTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display|Percentage", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float mobilityPercentage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display|Percentage", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float attackPercentage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display|Percentage", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float defensePercentage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display|Percentage", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float difficultyPercentage = 0.f;

	/** 캐릭터 메시 (비우면 캐릭터 BP 의 메시 유지) */
	UPROPERTY(EditDefaultsOnly, Category = "Appearance")
	TObjectPtr<USkeletalMesh> skeletalMesh;

	/** 애님 블루프린트 (비우면 기본 ABP_CDPlayer) */
	UPROPERTY(EditDefaultsOnly, Category = "Appearance")
	TSubclassOf<UAnimInstance> animClass;

	/** 기본 스탯 */
	UPROPERTY(EditDefaultsOnly)
	FCDBaseStats stats;

	/*
		10.02 - Jun6 스킬 보유 구조 변경

		어떤 스킬을 어떤 입력과 연결 시킬 것인가? -> 기본 FCDSkillSlot 구조체에 정의
		어떤 스킬을 보유하고 있는가? -> TMap <Tag, USkillData> 추가

		Input.Skill = 게임 UI상 슬롯
		Skill - 실제 게임 인식 구분 Tag
	*/

	/** 기본 QWER 배치 (게임 중 드래그 앤 드롭으로 교체 가능) */
	UPROPERTY(EditDefaultsOnly)
	TArray<FCDSkillSlot> defaultSkillSlots;

	/** 기본 보유 스킬 */
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, TObjectPtr<UCDSkillData>> defaultSkills;

	/** 기본 공격 어빌리티 (직업별 몽타주/수치는 BP 자식) */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCDAttackAbility> attackAbility;

	/** 회피 어빌리티 (직업별 몽타주/수치는 BP 자식) */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCDDodgeAbility> dodgeAbility;
};
