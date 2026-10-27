#include "Character/CDMonsterBase.h"
#include "AbilitySystem/CDHitReactAbility.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Core/CDMessageSubsystem.h"
#include "Data/CDMonsterData.h"
#include "TimerManager.h"
#include "Components/WidgetComponent.h"
#include "UI/CDMonsterHealthBarWidget.h"

ACDMonsterBase::ACDMonsterBase()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 커서 호버 시에만 CustomDepth 를 켠다 (UCDCursorHighlightComponent)
	GetMesh()->SetCustomDepthStencilValue(CDStencil::Monster);

	healthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	healthBarComponent->SetupAttachment(GetCapsuleComponent());
	healthBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	healthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	healthBarComponent->SetDrawAtDesiredSize(true);
	healthBarComponent->SetPivot(FVector2D(0.5f, 1.f));		// 바 아래 가운데가 Z+110 지점
	healthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	healthBarComponent->SetGenerateOverlapEvents(false);
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

	/* 디졸브로 인해 사라진 모습 복구 - yoonsang */
	OnResetVisual();

	SetPoolActive(true);

	// 보스는 화면 위 보스 바를 쓰므로 머리 위 바는 끔
	const bool bShowHealthBar = (data->grade != ECDMonsterGrade::Boss);
	healthBarComponent->SetVisibility(bShowHealthBar);
	if (UCDMonsterHealthBarWidget* healthBar = GetHealthBar())
	{
		if (bShowHealthBar)
		{
			// Revive 로 체력이 이미 최대 → BindTo 가 지연 연출 없이 가득 찬 상태로 맞춤
			healthBar->SetGrade(data->grade);
			healthBar->BindTo(abilitySystem);
		}
		else
		{
			healthBar->Unbind();
		}
	}

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

void ACDMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	abilitySystem->GrantAbility(hitReactAbilityClass);
}

float ACDMonsterBase::GetKillContribution() const
{
	return monsterData ? monsterData->contribution : 0.f;
}

void ACDMonsterBase::HandleDeath()
{
	Super::HandleDeath();

	SpawnDrops();

	/* Death 처리 시 충돌 끄기 
	*(밑에 SetActorEnableCollision을 false로 되는 시점이 pool로 돌아갈 때 여서 죽음 처리 후 충돌 처리) - yoonsang */
	SetActorEnableCollision(false);
	
	/* 죽음 처리 시 noiseTexture 재생 - yoonsang */ 
	OnDeathVisual();

	// 머리 위 HP 바: 빈 바를 잠깐 보여 준 뒤 사라짐 (hideAnim)
	if (UCDMonsterHealthBarWidget* healthBar = GetHealthBar())
	{
		healthBar->HideBar(true);
	}

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

	// 풀로 돌아갈 때 머리 위 바도 확실히 끔 (켜는 건 ActivateFromPool 에서 등급 확인 후)
	if (bActive == false)
	{
		healthBarComponent->SetVisibility(false);
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

UCDMonsterHealthBarWidget* ACDMonsterBase::GetHealthBar() const
{
	return Cast<UCDMonsterHealthBarWidget>(healthBarComponent->GetUserWidgetObject());
}
