#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CDInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/** 어빌리티 입력 1개 */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDAbilityInput
{
	GENERATED_BODY()

	/** 입력 액션 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<const UInputAction> action;

	/** 연결할 입력 태그 */
	UPROPERTY(EditAnywhere, meta = (Categories = "Input"))
	FGameplayTag inputTag;
};

/**
 * 입력 설정. 어빌리티 입력은 InputAction → GameplayTag 로만 연결한다.
 * 슬롯이 늘어나도(ASDF) 코드 수정 없이 배열에 추가하면 된다.
 * 키 매핑: InputAction 에 Player Mappable Key Settings 설정 + Project Settings > Enhanced Input > Enable User Settings
 */
UCLASS()
class CHAOSDUNGEON_API UCDInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 슬롯 입력 태그에 연결된 입력 액션 (없으면 nullptr) */
	const UInputAction* FindAbilityAction(FGameplayTag inputTag) const;

	/** 슬롯 입력 태그의 키 매핑 이름 (입력 액션의 Player Mappable Key Settings > Name). 없으면 NAME_None */
	FName FindMappingName(FGameplayTag inputTag) const;

public:
	/** 기본 매핑 컨텍스트 */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> mappingContext;

	/** 이동 (우클릭) */
	UPROPERTY(EditDefaultsOnly, Category = "Native")
	TObjectPtr<UInputAction> moveAction;

	/** 일시정지 (Esc, Trigger When Paused 체크) */
	UPROPERTY(EditDefaultsOnly, Category = "Native")
	TObjectPtr<UInputAction> pauseAction;

	/** 조작법 안내 (F1) */
	UPROPERTY(EditDefaultsOnly, Category = "Native")
	TObjectPtr<UInputAction> guideAction;

	/** 스킬 QWER, 회피 Space */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<FCDAbilityInput> abilityInputs;
};
