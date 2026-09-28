#include "Character/CDMonsterBase.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Core/CDMessageSubsystem.h"
#include "Data/CDMonsterData.h"
#include "TimerManager.h"

ACDMonsterBase::ACDMonsterBase()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 커서 호버 시에만 CustomDepth 를 켠다 (UCDCursorHighlightComponent)
	GetMesh()->SetCustomDepthStencilValue(CDStencil::Monster);
}

void ACDMonsterBase::ActivateFromPool(const UCDMonsterData* data, const FCDDifficultySettings& difficulty, const FVector& location)
{
	// 풀은 데이터별로 나뉘므로 어빌리티/면역 태그는 최초 1회만 부여
	const bool bFirstActivation = (monsterData == nullptr);
	monsterData = data;

	SetActorLocation(location + FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	InitializeAbilitySystem(data->stats, difficulty.healthMultiplier, difficulty.attackMultiplier);
	Revive(0.f);

	if (bFirstActivation)
	{
		for (UCDSkillData* skill : data->attackSkills)
		{
			abilitySystem->GrantSkill(skill);
		}
		abilitySystem->AddLooseGameplayTags(GetImmunityTags(data->grade));
	}

	SetPoolActive(true);

	if (data->grade == ECDMonsterGrade::Boss)
	{
		FCDActorMessage message;
		message.actor = this;
		UCDMessageSubsystem::Get(this).Broadcast(CDTags::Msg_Combat_BossAppeared, message);
	}
}

void ACDMonsterBase::DeactivateToPool()
{
	SetPoolActive(false);
	onReleased.Broadcast(this);
}

FVector ACDMonsterBase::GetAimLocation() const
{
	const AAIController* aiController = GetController<AAIController>();
	const AActor* target = aiController ? aiController->GetFocusActor() : nullptr;
	if (!target)
	{
		return Super::GetAimLocation();
	}
	return target->GetActorLocation();
}

float ACDMonsterBase::GetKillContribution() const
{
	return monsterData ? monsterData->contribution : 0.f;
}

void ACDMonsterBase::HandleDeath()
{
	Super::HandleDeath();
	SpawnDrops();
	GetWorldTimerManager().SetTimer(releaseTimer, this, &ThisClass::DeactivateToPool, corpseLifetime, false);
}

void ACDMonsterBase::SetPoolActive(bool bActive)
{
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);
	SetActorTickEnabled(bActive);

	if (AController* monsterController = GetController())
	{
		monsterController->SetActorTickEnabled(bActive);
		monsterController->StopMovement();
	}
}

void ACDMonsterBase::SpawnDrops() const
{
	for (const FCDDropEntry& drop : monsterData->drops)
	{
		if (drop.actorClass && FMath::FRand() < drop.chance)
		{
			GetWorld()->SpawnActor<AActor>(drop.actorClass, GetActorLocation(), FRotator::ZeroRotator);
		}
	}
}

FGameplayTagContainer ACDMonsterBase::GetImmunityTags(ECDMonsterGrade grade)
{
	FGameplayTagContainer tags;
	switch (grade)
	{
	case ECDMonsterGrade::Boss:
	{
		tags.AddTag(CDTags::State_Immune_Stagger);
		tags.AddTag(CDTags::State_Immune_Knockback);
		break;
	}
	case ECDMonsterGrade::Elite:
	{
		tags.AddTag(CDTags::State_Immune_Knockback);
		break;
	}
	default:
	{
		break;
	}
	}
	return tags;
}
