#include "AbilitySystem/CDGameplayAbility.h"
#include "AbilitySystem/CDGameplayEffects.h"
#include "AbilitySystem/CDProjectileBase.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "AbilitySystemComponent.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"
#include "Character/CDPlayerCharacter.h"
#include "Character/CDMonsterBase.h"
#include "Components/CapsuleComponent.h"
#include "Data/CDProjectileData.h"
#include "Data/CDMonsterData.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"


TAutoConsoleVariable<bool> CVarDebugHitShape(TEXT("CD.DebugHitShape"), false, TEXT("일반 공격 및 스킬 판정 범위를 그린다"));

UCDGameplayAbility::UCDGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CooldownGameplayEffectClass = UCDCooldownEffect::StaticClass();
	ActivationBlockedTags.AddTag(CDTags::State_Dead);
}

const FGameplayTagContainer* UCDGameplayAbility::GetCooldownTags() const
{
	cooldownTagsCache.Reset();
	const FGameplayTag cooldownTagConst = GetCooldownTag();
	if (cooldownTagConst.IsValid())
	{
		cooldownTagsCache.AddTag(cooldownTagConst);
	}
	return &cooldownTagsCache;
}

void UCDGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo) const
{
	const float duration = GetCooldownDuration();
	if (duration <= 0.f)
	{
		return;
	}
	const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(handle, actorInfo, activationInfo, CooldownGameplayEffectClass, GetAbilityLevel(handle, actorInfo));
	if (specHandle.IsValid())
	{
		specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Cooldown, duration);
		specHandle.Data->DynamicGrantedTags.AddTag(GetCooldownTag());
		ApplyGameplayEffectSpecToOwner(handle, actorInfo, activationInfo, specHandle);
	}
}

float UCDGameplayAbility::GetCooldownDuration() const
{
	return cooldownDuration;
}

FGameplayTag UCDGameplayAbility::GetCooldownTag() const
{
	return cooldownTag;
}

ACDCharacterBase* UCDGameplayAbility::GetCDCharacter() const
{
	return Cast<ACDCharacterBase>(GetAvatarActorFromActorInfo());
}

void UCDGameplayAbility::BeginCastTowards(const FVector& location)
{
	ACDCharacterBase* character = GetCDCharacter();
	
	if (AController* controller = character->GetController())
	{
		controller->StopMovement();
	}

	// 10.29 남은 속도 + 이번 프레임에 쌓인 이동 입력까지 제거
	// (남아 있으면 OrientRotationToMovement 가 다음 Tick 에 원래 이동 방향으로 되돌린다. 각성기는 플레이어 ×5 라 더 크게 보임)
	if (UCharacterMovementComponent* movement = character->GetCharacterMovement())
	{
		movement->StopMovementImmediately();
		movement->ConsumeInputVector();
	}

	const FVector toTarget = location - character->GetActorLocation();

	if (!toTarget.IsNearlyZero())
	{
		character->SetActorRotation(FRotator(0.f, toTarget.Rotation().Yaw, 0.f));
	}
}

void UCDGameplayAbility::OnDashFinished()
{
	ACDPlayerCharacter* playerCharacter = Cast<ACDPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (playerCharacter && playerCharacter->GetCapsuleComponent())
	{
		playerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}	
}

void UCDGameplayAbility::StartDash(const FVector& direction, float distance, float duration)
{
	ACDPlayerCharacter* playerCharacter = Cast<ACDPlayerCharacter>(GetAvatarActorFromActorInfo());

	if (playerCharacter)
	{
		playerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
	
	UAbilityTask_ApplyRootMotionConstantForce* task = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this, NAME_None, direction.GetSafeNormal2D(), distance / duration, duration,
		false, nullptr, ERootMotionFinishVelocityMode::SetVelocity, FVector::ZeroVector, 0.f, false);

	task->OnFinish.AddDynamic(this, &ThisClass::OnDashFinished);
	task->ReadyForActivation();
}

ACDProjectileBase* UCDGameplayAbility::SpawnProjectile(const FCDProjectileBaseData& inData, float damageCoefficient) const
{
    if (!GetCurrentActorInfo())
    {
        return nullptr;
    }

    UWorld* world = GetWorld();
    AActor* sourceActor = GetAvatarActorFromActorInfo();
    UAbilitySystemComponent* sourceASC = GetAbilitySystemComponentFromActorInfo();

    if (!world || !IsValid(sourceActor) || !IsValid(sourceASC) || !inData.projectileClass)
    {
        return nullptr;
    }

    // 원본 설정을 복사하고 이번 발사의 정보 입력
    FCDProjectileBaseData launchData = inData;
    launchData.sourceActor = sourceActor;
    launchData.sourceASC = sourceASC;

    // 현재 Ability의 발사자를 기준으로 데미지 Spec 생성
    launchData.damageSpec = MakeOutgoingGameplayEffectSpec( UCDDamageEffect::StaticClass(), GetAbilityLevel());

    if (!launchData.damageSpec.IsValid())
    {
        return nullptr;
    }

    launchData.damageSpec.Data->SetSetByCallerMagnitude( CDTags::SetByCaller_Coefficient, damageCoefficient);

    FActorSpawnParameters spawnParams;
    spawnParams.Owner = sourceActor;
    spawnParams.Instigator = Cast<APawn>(sourceActor);
    spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    const FRotator launchRotation = (launchData.targetPoint - launchData.launchLocation).Rotation();

    ACDProjectileBase* projectile = world->SpawnActor<ACDProjectileBase>(launchData.projectileClass, launchData.launchLocation, launchRotation, spawnParams);

    if (!IsValid(projectile))
    {
        return nullptr;
    }

    // BP 구성까지 완료된 Actor에 발사 데이터 전달
    projectile->InitializeProjectile(launchData);

    // 이동 설정 실패로 초기화 중 제거된 경우
    if (!IsValid(projectile) || projectile->IsActorBeingDestroyed())
    {
        return nullptr;
    }

    return projectile;
}
