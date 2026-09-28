#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "CDDamageExecution.generated.h"

/**
 * 데미지 공식은 여기 한 곳에만 둔다.
 *   공격력 x 스킬계수 x (1 - 방어력) [x 치명타 배율]
 * 난이도 배율은 몬스터 스폰 시 공격력/체력에 미리 곱해져 있다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	/** 캡처할 속성 등록 */
	UCDDamageExecution();

	// UGameplayEffectExecutionCalculation::Execute()
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& executionParams,
		FGameplayEffectCustomExecutionOutput& outExecutionOutput) const override;
};
