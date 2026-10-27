// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/CDBossSpawnAbility.h"

#include <Character/CDBossCharacter.h>
#include <Components/CapsuleComponent.h>
#include <Data/CDBossSkillData.h>
#include <Data/CDMonsterData.h>
#include <Engine/World.h>
#include <GameFramework/GameModeBase.h>
#include <World/CDSpawnManagerComponent.h>
#include "NavigationSystem.h"

void UCDBossSpawnAbility::ApplyHit()
{
    const UCDBossSkillData* data = Cast<UCDBossSkillData>(GetSkillData());

    ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetCDCharacter());

    UWorld* world = GetWorld();

    if (!world ||
        !IsValid(data) ||
        !IsValid(boss) ||
        boss->IsDead() ||
        !IsValid(data->summonMonsterData.Get()))
    {
        return;
    }

    // 기존 SpawnManager를 가져온다.
    AGameModeBase* gameMode = world->GetAuthGameMode();

    UCDSpawnManagerComponent* spawnManager =
        gameMode ? gameMode->FindComponentByClass<UCDSpawnManagerComponent>() : nullptr;

    const UNavigationSystemV1* navSystem = UNavigationSystemV1::GetCurrent(world);

    if (!IsValid(spawnManager) || !navSystem)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Boss summon failed: SpawnManager or NavigationSystem missing."));
        return;
    }

    // 노티파이가 발생한 순간의 보스 방향을 기준
    const FVector forward = boss->GetActorForwardVector().GetSafeNormal2D();

    const FVector right = boss->GetActorRightVector().GetSafeNormal2D();

    const FVector bossGroundLocation = boss->GetActorLocation() -
        FVector(0.f, 0.f, boss->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

    const FVector center = bossGroundLocation + forward * data->summonForwardDistance;

    const int32 count = FMath::Max(1, data->summonCount);
    int32 spawnedCount = 0;

    for (int32 index = 0; index < count; ++index)
    {
        const float sideOffset = (index - (count - 1) * 0.5f) * data->summonSpacing;

        const FVector requestedLocation = center + right * sideOffset;

        FNavLocation navLocation;

        if (!navSystem->ProjectPointToNavigation(requestedLocation, navLocation, FVector(100.f, 100.f, 200.f)))
        {
            continue;
        }

        if (spawnManager->SpawnMonsterAt(
            data->summonMonsterData.Get(),
            navLocation.Location))
        {
            ++spawnedCount;
        }
    }

    UE_LOG(LogTemp, Log,
        TEXT("Boss summon: requested %d, spawned %d"),
        count, spawnedCount);
}
