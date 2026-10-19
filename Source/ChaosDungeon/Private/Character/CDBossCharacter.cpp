// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CDBossCharacter.h"
#include <Boss/CDBossPhaseComponent.h>
#include <AI/CDBossAIController.h>
#include <Components/CapsuleComponent.h>
#include <Data/CDMonsterData.h>

ACDBossCharacter::ACDBossCharacter()
{
    phaseComponent = CreateDefaultSubobject<UCDBossPhaseComponent>(TEXT("PhaseComponent"));
}

void ACDBossCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 배치용 데이터가 지정되지 않았다면
    // 기존 스폰 매니저의 초기화를 기다린다.
    if (!IsValid(placedBossData.Get()))
    {
        return;
    }

    // 이미 초기화된 보스는 다시 초기화하지 않는다.
    if (IsValid(GetMonsterData()))
    {
        return;
    }

    // 기본 난이도: 체력과 공격력 배율 모두 1
    const FCDDifficultySettings difficulty{};

    // ActivateFromPool은 바닥 위치를 받아
    // 캡슐 절반 높이를 더하므로, 여기서는 먼저 빼준다.
    const FVector groundLocation =
        GetActorLocation() -
        FVector(
            0.f,
            0.f,
            GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
        );

    ActivateFromPool(
        placedBossData.Get(),
        difficulty,
        groundLocation
    );
}

void ACDBossCharacter::ActivateFromPool(
    const UCDMonsterData* data,
    const FCDDifficultySettings& difficulty,
    const FVector& location)
{
    if (!IsValid(data))
    {
        return;
    }

    bossData = Cast<UCDBossData>(data);
    if (!IsValid(bossData.Get()))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Boss initialization requires UCDBossData."));
        return;
    }

    if (ACDBossAIController* ai = GetController<ACDBossAIController>())
    {
        ai->StopBossLogic();
    }

    /* 데이터(능력치스킬 부여)와 활성화 */
    Super::ActivateFromPool(data, difficulty, location);

    phaseComponent->ResetPhase();

    /* 초기화 완료 후 AI 시작 */
    if (ACDBossAIController* ai = GetController<ACDBossAIController>())
    {
        ai->StartBossLogic();
    }
}

void ACDBossCharacter::DeactivateToPool()
{
    if (ACDBossAIController* ai = GetController<ACDBossAIController>())
    {
        ai->StopBossLogic();
    }

    Super::DeactivateToPool();
}
void ACDBossCharacter::HandleDeath()
{
    Super::HandleDeath();

    if (ACDBossAIController* ai = GetController<ACDBossAIController>())
    {
        ai->StopBossLogic();
    }
}

void ACDBossCharacter::HandleDamageTaken(AActor* damageInstigator, float damage, bool bCritical)
{
    Super::HandleDamageTaken(damageInstigator, damage, bCritical);

    phaseComponent->EvaluateHealth();
}


