#include "AbilitySystem/CDMonsterSkillAbility.h"
#include "AIController.h"
#include "AbilitySystem/CDProjectileBase.h"
#include "AbilitySystem/CDReticleActor.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Character/CDCharacterBase.h"
#include "Character/CDMonsterBase.h"
#include "Core/CDGameplayTags.h"
#include "Data/CDSkillData.h"
#include "Data/CDMonsterData.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

UCDMonsterSkillAbility::UCDMonsterSkillAbility()
{
	static ConstructorHelpers::FClassFinder<ACDReticleActor> reticleClassRef(
		TEXT("/Game/Character/Player/Abilities/BP_CDReticleActor.BP_CDReticleActor_C"));
	if (reticleClassRef.Succeeded())
	{
		reticleClass = reticleClassRef.Class;
	}
}

void UCDMonsterSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData)
{
	Super::ActivateAbility(handle, actorInfo, activationInfo, triggerEventData);

	const ACDCharacterBase* character = GetCDCharacter();
	if (!IsValid(skillData) || !IsValid(character) || !skillData->montage || !GetWorld()
		|| !GetAbilitySystemComponentFromActorInfo())
	{
		EndAbility(handle, actorInfo, activationInfo, true, true);
		return;
	}
	if (!CommitAbility(handle, actorInfo, activationInfo))
	{
		EndAbility(handle, actorInfo, activationInfo, true, true);
		return;
	}

	// AI의 현재 타겟 위치를 시전 시점에 고정. 
	skillCastLocation = character->GetActorLocation();
	const FVector aimLocation = character->GetAimLocation();
	targetLocation = skillCastLocation + (aimLocation - skillCastLocation).GetClampedToMaxSize2D(FMath::Max(0.f, skillData->maxRange));
	BeginCastTowards(aimLocation);
	SetCasting(true);

	if (skillData->dashDistance > 0.f && skillData->dashDuration > 0.f)
	{
		StartDash(character->GetActorForwardVector(), skillData->dashDistance, skillData->dashDuration);
	}
	ShowReticle();

	UAbilityTask_WaitGameplayEvent* hitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CDTags::Event_Montage, nullptr, false, false);
	hitTask->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);
	hitTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* vfxTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CDTags::GameplayCue_Skill, nullptr, false, false);
	vfxTask->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);
	vfxTask->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* montageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, skillData->montage, 1.f, NAME_None, true, 1.f, 0.f, true);
	montageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	montageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	montageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	montageTask->ReadyForActivation();
}

void UCDMonsterSkillAbility::EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(handle, actorInfo))
	{
		return;
	}
	ClearReticle();
	SetUntargetable(false);
	Super::EndAbility(handle, actorInfo, activationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UCDMonsterSkillAbility::GetSkillOrigin() const
{
	if (!skillData)
	{
		return FVector::ZeroVector;
	}
	switch (skillData->origin)
	{
	case ECDSkillOrigin::TargetLocation:
		return targetLocation;
	case ECDSkillOrigin::CastStartLocation:
		return skillCastLocation;
	default:
		return GetCDCharacter() ? GetCDCharacter()->GetActorLocation() : FVector::ZeroVector;
	}
}

void UCDMonsterSkillAbility::ApplyHit()
{
	ACDCharacterBase* character = GetCDCharacter();

	if (!IsValid(skillData) || !IsValid(character) || !GetWorld())
	{
		return;
	}

	/* Temp */
	const ACDMonsterBase* monster = Cast<ACDMonsterBase>(character);

	const UCDMonsterData* monsterData = monster ? monster->GetMonsterData() : nullptr;

	const bool bIsBombMonster = monsterData && monsterData->grade == ECDMonsterGrade::Bomb;

	if (bIsBombMonster)
	{
		UE_LOG(LogTemp, Log, TEXT("Bomb"));
	}

	// 투사체 스킬
	if (skillData->projectileData.projectileClass)
	{
		FireProjectile();
		return;
	}

	// 기존 범위 공격
	const FVector origin = GetSkillOrigin();
	const FVector forward = character->GetActorForwardVector();

	TArray<AActor*> hitActors;
	skillData->hitShape.FindActors(GetWorld(), origin, forward, hitActors
	);

	for (AActor* actor : hitActors)
	{
		if (character->IsHostileTo(actor))
		{
			ApplyDamageTo(actor);
		}
	}

	if (CVarDebugHitShape.GetValueOnGameThread())
	{
		skillData->hitShape.DrawDebug(GetWorld(), origin, forward, FColor::Red, 1.f
		);
	}

	if (bIsBombMonster)
	{
		/* 폭발 끝 - yoonsang */
		EndBombExplosion();
		UE_LOG(LogTemp, Log, TEXT("Bomb Finished"));
	}
}

void UCDMonsterSkillAbility::OnEventReceived(FGameplayEventData payload)
{
	ACDCharacterBase* character = GetCDCharacter();
	if (!IsActive() || !IsValid(skillData) || !IsValid(character))
	{
		return;
	}

	const FGameplayTag tag = payload.EventTag;
	if (tag == CDTags::Event_Montage_Hit)
	{
		ClearReticle();
		ApplyHit();
	}
	else if (tag == CDTags::Event_Montage_ReticleOn)
	{
		ShowReticle();
	}
	else if (tag == CDTags::Event_Montage_Untargetable)
	{
		SetUntargetable(true);
	}
	else if (tag == CDTags::Event_Montage_Targetable)
	{
		SetUntargetable(false);
	}
	else if (tag == CDTags::Event_Montage_Land)
	{
		const float airOffset = character->GetActorLocation().Z - skillCastLocation.Z;
		const float destinationZ = targetLocation.Z + character->GetSimpleCollisionHalfHeight() + airOffset;
		character->SetActorLocation(FVector(targetLocation.X, targetLocation.Y, destinationZ));
	}
	else if (tag.MatchesTag(CDTags::GameplayCue_Skill))
	{
		for (const FCDSkillVFX& vfx : skillData->skillEffects)
		{
			if (tag == vfx.VFXCueTag)
			{
				FGameplayCueParameters params;
				params.Location = GetSkillOrigin();
				GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(tag, params);
				break;
			}
		}
	}
}

void UCDMonsterSkillAbility::ShowReticle()
{
	ClearReticle();
	ACDCharacterBase* character = GetCDCharacter();
	UWorld* world = GetWorld();

	if (!skillData || !IsValid(character) || !GetWorld() || !reticleClass
		|| skillData->reticleConfig.displayMode == ECDReticleDisplayMode::None)
	{
		return;
	}

	FVector location = GetSkillOrigin();
	if (skillData->hitShape.type == ECDHitShapeType::Line)
	{
		location += character->GetActorForwardVector() * (skillData->hitShape.length * 0.5f);
	}
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = character;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACDReticleActor* reticle = world->SpawnActor<ACDReticleActor>(reticleClass, location, character->GetActorRotation(), spawnParams);
	if (!IsValid(reticle))
	{
		return;
	}

	spawnedReticle.Add(reticle);

	reticle->SetReticleConfig(*skillData);

	if (skillData->reticleConfig.displayMode == ECDReticleDisplayMode::Fill)
	{
		reticle->StartFill(skillData->reticleConfig.fillDuration);
	}

	if (skillData->origin == ECDSkillOrigin::Self)
	{
		reticle->AttachToActor(character, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void UCDMonsterSkillAbility::ClearReticle()
{
	for (const auto& reticle : spawnedReticle)
	{
		if (IsValid(reticle.Get()))
		{
			reticle->Destroy();
		}
	}
		spawnedReticle.Reset();
}

void UCDMonsterSkillAbility::BeginCastTowards(const FVector& location)
{
	ACDCharacterBase* character = GetCDCharacter();
	const ACDMonsterBase* monster = Cast<ACDMonsterBase>(character);

	/* 몬스터일때만 몬스터 데이터 가져오기 - yoonsang */
	const UCDMonsterData* monsterData = monster ? monster->GetMonsterData() : nullptr;


	/* 원거리 몬스터 SetActorRotation 시 끊기는 현상 개선 - yoonsang */
	if (monsterData && monsterData->useSmoothAttackRotation)
	{
		if (UCharacterMovementComponent* movement = character->GetCharacterMovement())
		{
			movement->bOrientRotationToMovement = false;
			movement->bUseControllerDesiredRotation = true;

			movement->RotationRate = FRotator(0.f, 180.f, 0.f);
			return;
		}
	}
	Super::BeginCastTowards(location);
}

void UCDMonsterSkillAbility::FireProjectile()
{
	if (!IsValid(skillData))
	{
		return;
	}

	const FCDBossProjectileData& data = skillData->projectileData.bossData;
	const int32 count = data.projectileCount;
	const float baseAngle = count > 1 ? -data.spreadAngle * 0.5f : 0.f;
	const float angleStep = count > 1 ? data.spreadAngle / (count - 1) : 0.f;

	for (int32 index = 0; index < count; ++index)
	{
		FireProjectileWithYaw(baseAngle + angleStep * index);
	}
}

void UCDMonsterSkillAbility::FireProjectileWithYaw(float yawOffset)
{
	const ACDCharacterBase* character = GetCDCharacter();

	if (!IsValid(skillData) || !IsValid(character))
	{
		return;
	}

	// 에셋의 설정을 이번 발사용 데이터로 복사
	FCDProjectileBaseData launchData = skillData->projectileData;

	// 피격 설정 전달
	launchData.bCausesHitReact = skillData->bCausesHitReact;
	launchData.knockbackStrength = skillData->knockbackStrength;

	// 발사 위치와 시전 시점에 확정한 조준점
	launchData.launchLocation = character->GetActorLocation();
	launchData.targetPoint = targetLocation;
	if (yawOffset != 0.f)
	{
		const FVector aimOffset = targetLocation - launchData.launchLocation;
		launchData.targetPoint = launchData.launchLocation + aimOffset.RotateAngleAxis(yawOffset, FVector::UpVector);
	}

	// 이후 방향 전환 패턴에서 사용할 대상 Actor
	const AAIController* aiController = character->GetController<AAIController>();

	launchData.targetActor = aiController ? aiController->GetFocusActor() : nullptr;

	// 발사자 ASC와 데미지 Spec은 공통 함수에서 설정
	SpawnProjectile(launchData, skillData->damageCoefficient);
}

void UCDMonsterSkillAbility::SetUntargetable(bool bNewUntargetable)
{
	UAbilitySystemComponent* abilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (!abilitySystem || bOwnsUntargetableTag == bNewUntargetable)
	{
		return;
	}
	bOwnsUntargetableTag = bNewUntargetable;
	if (bNewUntargetable)
	{
		abilitySystem->AddLooseGameplayTag(CDTags::State_Untargetable);
	}
	else
	{
		abilitySystem->RemoveLooseGameplayTag(CDTags::State_Untargetable);
	}
}

void UCDMonsterSkillAbility::EndBombExplosion()
{
	ACDCharacterBase* character = GetCDCharacter();

	ACDMonsterBase* monster = Cast<ACDMonsterBase>(character);

	monster->DeactivateToPool();

	UE_LOG(LogTemp, Log, TEXT("Bomb InBombExplosion"));
}
