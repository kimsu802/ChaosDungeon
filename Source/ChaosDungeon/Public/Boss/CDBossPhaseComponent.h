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
	Enrage,
	Dead
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHAOSDUNGEON_API UCDBossPhaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCDBossPhaseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
