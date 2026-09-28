#include "AI/CDMonsterAIController.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "Character/CDMonsterBase.h"
#include "Core/CDGameplayTags.h"
#include "Data/CDMonsterData.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/CrowdFollowingComponent.h"

ACDMonsterAIController::ACDMonsterAIController(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	PrimaryActorTick.bCanEverTick = true;
	// 60마리 기준, 매 프레임 판정 불필요
	PrimaryActorTick.TickInterval = 0.1f;
}

void ACDMonsterAIController::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	AActor* target = FindTarget();
	if (target)
	{
		SetFocus(target);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
	}

	const ECDMonsterState newState = EvaluateState(target);
	if (newState != state)
	{
		EnterState(newState);
	}

	switch (state)
	{
	case ECDMonsterState::Chase:
	{
		// 풀에서 재사용되면 이동이 멈춘 채로 Chase 일 수 있으므로 Idle 이면 다시 이동
		if (GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			MoveToActor(target, acceptanceRadius);
		}
		break;
	}
	case ECDMonsterState::Attack:
	{
		// 쿨다운/시전 중이면 GAS 가 알아서 거절
		TryAttack();
		break;
	}
	default:
	{
		break;
	}
	}
}

AActor* ACDMonsterAIController::FindTarget() const
{
	ACDCharacterBase* player = Cast<ACDCharacterBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!player || player->IsDead())
	{
		return nullptr;
	}
	return player;
}

ECDMonsterState ACDMonsterAIController::EvaluateState(const AActor* target) const
{
	const ACDMonsterBase* monster = GetPawn<ACDMonsterBase>();
	if (!monster || monster->IsDead())
	{
		return ECDMonsterState::Dead;
	}

	const UCDAbilitySystemComponent* abilitySystem = monster->GetCDAbilitySystemComponent();
	if (abilitySystem->HasMatchingGameplayTag(CDTags::State_HitReact))
	{
		return ECDMonsterState::HitReact;
	}
	if (abilitySystem->HasMatchingGameplayTag(CDTags::State_Casting))
	{
		return ECDMonsterState::Attack;
	}
	if (!target)
	{
		return ECDMonsterState::Idle;
	}

	const float distance = FVector::Dist2D(monster->GetActorLocation(), target->GetActorLocation());
	if (distance <= monster->GetMonsterData()->attackRange)
	{
		return ECDMonsterState::Attack;
	}
	return ECDMonsterState::Chase;
}

void ACDMonsterAIController::EnterState(ECDMonsterState newState)
{
	state = newState;
	if (state != ECDMonsterState::Chase)
	{
		StopMovement();
	}
}

void ACDMonsterAIController::TryAttack() const
{
	const ACDMonsterBase* monster = GetPawn<ACDMonsterBase>();
	for (const UCDSkillData* skill : monster->GetMonsterData()->attackSkills)
	{
		if (monster->GetCDAbilitySystemComponent()->TryActivateSkill(skill))
		{
			return;
		}
	}
}
