#include "Character/CDCharacterBase.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AbilitySystem/CDAttributeSet.h"
#include "AbilitySystem/CDHitReactAbility.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Core/CDMessageSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

ACDCharacterBase::ACDCharacterBase()
{
	abilitySystem = CreateDefaultSubobject<UCDAbilitySystemComponent>(TEXT("AbilitySystem"));
	attributes = CreateDefaultSubobject<UCDAttributeSet>(TEXT("Attributes"));
	hitReactAbilityClass = UCDHitReactAbility::StaticClass();

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
}

UAbilitySystemComponent* ACDCharacterBase::GetAbilitySystemComponent() const
{
	return abilitySystem;
}

FVector ACDCharacterBase::GetAimLocation() const
{
	return GetActorLocation() + GetActorForwardVector() * 100.f;
}

bool ACDCharacterBase::IsHostileTo(const AActor* other) const
{
	const ACDCharacterBase* otherCharacter = Cast<ACDCharacterBase>(other);
	return otherCharacter && !otherCharacter->IsDead() && otherCharacter->IsPlayerControlled() != IsPlayerControlled();
}

bool ACDCharacterBase::IsDead() const
{
	return abilitySystem->HasMatchingGameplayTag(CDTags::State_Dead);
}

void ACDCharacterBase::GrantInvincibility(float duration)
{
	// 루즈 태그는 카운트 방식이라 회피 무적과 부활 무적이 겹쳐도 안전
	abilitySystem->AddLooseGameplayTag(CDTags::State_Invincible);

	auto RemoveInvincible = [this]()
	{
		abilitySystem->RemoveLooseGameplayTag(CDTags::State_Invincible);
	};
	FTimerHandle timerHandle;
	GetWorldTimerManager().SetTimer(timerHandle, FTimerDelegate::CreateWeakLambda(this, RemoveInvincible), duration, false);
}

void ACDCharacterBase::Revive(float invincibleTime)
{
	abilitySystem->SetLooseGameplayTagCount(CDTags::State_Dead, 0);
	abilitySystem->SetNumericAttributeBase(UCDAttributeSet::GetHealthAttribute(), attributes->GetMaxHealth());
	StopAnimMontage();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (invincibleTime > 0.f)
	{
		GrantInvincibility(invincibleTime);
	}
}

void ACDCharacterBase::InitializeAbilitySystem(const FCDBaseStats& stats, float healthMultiplier, float attackMultiplier)
{
	abilitySystem->InitAbilityActorInfo(this, this);
	attributes->InitFromStats(stats, healthMultiplier, attackMultiplier);

	if (!bAbilitySystemInitialized)
	{
		bAbilitySystemInitialized = true;
		abilitySystem->GrantAbility(hitReactAbilityClass);
		attributes->onDamageTaken.AddUObject(this, &ThisClass::HandleDamageTaken);
		attributes->onOutOfHealth.AddUObject(this, &ThisClass::HandleDeath);
	}
}

float ACDCharacterBase::GetKillContribution() const
{
	return 0.f;
}

void ACDCharacterBase::HandleDamageTaken(AActor* damageInstigator, float damage, bool bCritical)
{
	FCDDamageMessage message;
	message.instigator = damageInstigator;
	message.target = this;
	message.amount = damage;
	message.bCritical = bCritical;
	message.bTargetIsPlayer = IsPlayerControlled();
	UCDMessageSubsystem::Get(this).Broadcast(CDTags::Msg_Combat_Damage, message);
}

void ACDCharacterBase::HandleDeath()
{
	abilitySystem->AddLooseGameplayTag(CDTags::State_Dead);
	abilitySystem->CancelAllAbilities();
	GetCharacterMovement()->DisableMovement();
	PlayAnimMontage(deathMontage);

	FCDDeathMessage message;
	message.victim = this;
	message.bIsPlayer = IsPlayerControlled();
	message.contribution = GetKillContribution();
	UCDMessageSubsystem::Get(this).Broadcast(CDTags::Msg_Combat_Death, message);
}
