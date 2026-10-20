// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CDBossPhaseComponent.generated.h"


/* 보스의 페이즈 열거형 구분 */
UENUM(BluePrintType)
enum class ECDBossPhase : uint8
{
	Phase1,
	Phase2
};

/* 2페이즈 상태 구분 */
UENUM(BluePrintType)
enum class ECDBossPhaseState : uint8
{
	Inactive,
	Combat,
	Special,
	Dead,
	Transitioning
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHAOSDUNGEON_API UCDBossPhaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCDBossPhaseComponent();

	/* 보스 활성화 시 1페이즈 전투 상태로 초기화 */
	void ResetPhase();

	/* 체력을 확인해 페이즈 전환 시작 여부 판단 */
	virtual void EvaluateHealth();

private:
	void OnPhaseMontageEnded(UAnimMontage* montage, bool bInterrupted);

protected:
	/* 페이즈 전환 애님 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Anim)
	UAnimMontage* PhaseChange;

private:
	/* 현재 보스 페이즈 */
	ECDBossPhase currentPhase = ECDBossPhase::Phase1;

	/* 현재 페이즈의 진행 상태 */
	ECDBossPhaseState phaseState = ECDBossPhaseState::Inactive;

};
