// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/CDBossAIController.h"
#include <Character/CDCharacterBase.h>
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include <BehaviorTree/BlackboardComponent.h>
#include <Character/CDBossCharacter.h>
#include <Data/CDMonsterData.h>
#include <Data/CDBossSkillData.h>
#include <AbilitySystem/CDAbilitySystemComponent.h>
#include <Kismet/GameplayStatics.h>

void ACDBossAIController::StartBossLogic()
{
    if (bBossLogicRunning || bIsPatternRunning)
    {
        return;
    }

    const ACDBossCharacter* boss = GetPawn<ACDBossCharacter>();

    /* 스폰 데이터가 적용된 뒤에만 시작 */
    if (!IsValid(boss) || boss->IsDead() ||
        !IsValid(boss->GetMonsterData()))
    {
        return;
    }

    if (!bossBehaviorTree || !bossBehaviorTree->BlackboardAsset)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Boss Behavior Tree or Blackboard is missing."));
        return;
    }

    UBlackboardComponent* bb = nullptr;

    if (!UseBlackboard(bossBehaviorTree->BlackboardAsset, bb))
    {
        return;
    }

    bBossLogicRunning = true;
    bPatternSucceed = false;

    RefreshTarget();
    RefreshBlackBoard();

    if (!RunBehaviorTree(bossBehaviorTree.Get()))
    {
        StopBossLogic();
    }
}

void ACDBossAIController::StopBossLogic()
{
    /* 종료 콜백이 들어와도 새 공격을 선택하지 않도록 먼저 해제 */
    bBossLogicRunning = false;

    if (UBehaviorTreeComponent* tree = Cast<UBehaviorTreeComponent>(GetBrainComponent()))
    {
        tree->StopTree(EBTStopMode::Forced);
    }

    CancelPattern();
    StopMovement();

    targetActor.Reset();
    ClearFocus(EAIFocusPriority::Gameplay);
    bPatternSucceed = false;

    RefreshBlackBoard();
}

bool ACDBossAIController::HasReadyPatternInRange() const
{
    return ReadyPattern() != nullptr;
}

bool ACDBossAIController::StartPattern()
{
    if (!bBossLogicRunning || bIsPatternRunning)
    {
        return false;
    }

    ACDBossCharacter* boss = GetPawn<ACDBossCharacter>();
    const UCDBossSkillData* skill = ReadyPattern();

    UCDAbilitySystemComponent* abilitySys = IsValid(boss) ? boss->GetCDAbilitySystemComponent() : nullptr;

    bPatternSucceed = false;

    if (!IsValid(abilitySys) || !IsValid(skill))
    {
        return false;
    }
    /*  같은 Ability 클래스를 사용하는 스킬들도
     SourceObject로 각각 구분한다.*/
    FGameplayAbilitySpecHandle selectedHandle;

        for (const FGameplayAbilitySpec& spec : abilitySys->GetActivatableAbilities())
        {
            if (spec.SourceObject.Get() == skill)
            {
                selectedHandle = spec.Handle;
                break;
            }
        }

    if (!selectedHandle.IsValid())
    {
        return false;
    }

    patternHandle = selectedHandle;
    activeAbilitySystem = abilitySys;
    bIsPatternRunning = true;

    /* 실행 중 즉시 종료되는 경우도 받을 수 있도록 먼저 연결 */
    patternEnd = abilitySys->OnAbilityEnded.AddUObject(this, &ACDBossAIController::OnPatternFinished);

    StopMovement();
    RefreshBlackBoard();

    if (!abilitySys->TryActivateSkill(skill))
    {
        FinishPattern(false);
        return false;
    }
    return true;
}

void ACDBossAIController::CancelPattern()
{
    if (!bIsPatternRunning)
    {
        return;
    }

    UCDAbilitySystemComponent* abilitySys = activeAbilitySystem.Get();

    if (!IsValid(abilitySys) || !patternHandle.IsValid())
    {
        FinishPattern(false);
        return;
    }

    /* 현재 보스 공격만 취소.
     실제 종료 상태 정리는 OnPatternFinished에서 처리.*/
    abilitySys->CancelAbilityHandle(patternHandle);
}

void ACDBossAIController::OnPossess(APawn* inPawn)
{
    Super::OnPossess(inPawn);
    StartBossLogic();
}

void ACDBossAIController::OnUnPossess()
{
    StopBossLogic();
    Super::OnUnPossess();
}

const UCDBossSkillData* ACDBossAIController::ReadyPattern() const
{
    const ACDBossCharacter* boss = GetPawn<ACDBossCharacter>();
    const ACDCharacterBase* target = Cast<ACDCharacterBase>(targetActor.Get());

    if (!bBossLogicRunning || !IsValid(boss) || 
        boss->IsDead() || !IsValid(target) ||
        target->IsDead() || bIsPatternRunning)
    {
        return nullptr;
    }

    const UCDMonsterData* data = boss->GetMonsterData();
    const UCDAbilitySystemComponent* abilitySys = boss->GetCDAbilitySystemComponent();

    if (!IsValid(data) || !IsValid(abilitySys))
    {
        return nullptr;
    }

    const float distance = FVector::Dist2D(
        boss->GetActorLocation(),
        target->GetActorLocation());

    const UCDBossSkillData* selected = nullptr;

    for (const UCDSkillData* baseSkill : data->attackSkills)
    {
        const UCDBossSkillData* skill = Cast<UCDBossSkillData>(baseSkill);

        if (!IsValid(skill) || !IsValid(skill->montage.Get()))
        {
            continue;
        }

        if (distance < skill->bossMinRange ||
            distance > skill->bossMaxRange)
        {
            continue;
        }

        if (!abilitySys->CanActivateSkill(skill))
        {
            continue;
        }

        if (!selected || skill->Priority > selected->Priority)
        {
            selected = skill;
        }
    }
    return selected;
}

void ACDBossAIController::OnPatternFinished(const FAbilityEndedData& endedData)
{
    /* 다른 Ability의 종료 알림은 무시 */
    if (!bIsPatternRunning ||
        endedData.AbilitySpecHandle != patternHandle)
    {
        return;
    }

    FinishPattern(!endedData.bWasCancelled);
}

void ACDBossAIController::FinishPattern(bool bSucceeded)
{
    if (UCDAbilitySystemComponent* asc = activeAbilitySystem.Get())
    {
        asc->OnAbilityEnded.Remove(patternEnd);
    }

    patternEnd.Reset();
    patternHandle = FGameplayAbilitySpecHandle();
    activeAbilitySystem.Reset();

    bIsPatternRunning = false;
    bPatternSucceed = bSucceeded;

    RefreshBlackBoard();
}

void ACDBossAIController::RefreshTarget()
{
	ACDCharacterBase* Player = Cast<ACDCharacterBase>(UGameplayStatics::GetPlayerPawn(this, 0));

	/* 플레이어가 존재하지 않거나 죽으면 타겟 리셋 */
	if (!IsValid(Player) || Player->IsDead())
	{
		targetActor.Reset();
		ClearFocus(EAIFocusPriority::Gameplay);
		StopMovement();
		return;
	}

	targetActor = Player;
	SetFocus(Player);
}

void ACDBossAIController::RefreshBlackBoard()
{
    UBlackboardComponent* bb = GetBlackboardComponent();
    if (!bb)
    {
        return;
    }

    const ACDBossCharacter* boss = GetPawn<ACDBossCharacter>();
    AActor* target = targetActor.Get();

    const UCDMonsterData* data = IsValid(boss) ? boss->GetMonsterData() : nullptr;

    bb->SetValueAsBool(TEXT("IsPatternRunning"), bIsPatternRunning);

    /* 대상이나 초기화된 보스 데이터가 없으면 행동 조건 해제*/
    if (!bBossLogicRunning || !IsValid(boss) ||
        boss->IsDead() || !IsValid(target) || !data)
    {
        bb->ClearValue(TEXT("TargetActor"));
        bb->SetValueAsFloat(TEXT("DistanceToTarget"), 0.0f);
        bb->SetValueAsBool(TEXT("InMeleeRange"), false);
        bb->SetValueAsBool(TEXT("HasReadyPattern"), false);
        return;
    }

    const float distance = FVector::Dist2D(
        boss->GetActorLocation(),
        target->GetActorLocation());

    bb->SetValueAsObject(TEXT("TargetActor"), target);
    bb->SetValueAsFloat(TEXT("DistanceToTarget"), distance);
    bb->SetValueAsBool(TEXT("InMeleeRange"), distance <= data->attackRange);
    bb->SetValueAsBool(TEXT("HasReadyPattern"),!bIsPatternRunning && HasReadyPatternInRange());
}
