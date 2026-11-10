// Fill out your copyright notice in the Description page of Project Settings.
#include "AbilitySystem/CDAttackAbility.h"
#include "AbilitySystem/CDGameplayEffects.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/CDProjectileBase.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDTypes.h"
#include "Core/CDRunSubsystem.h"
#include "Character/CDCharacterBase.h"
#include "Data/CDMonsterDataInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "Player/CDPhantomComponent.h"

UCDAttackAbility::UCDAttackAbility()
{
	SetAssetTags(FGameplayTagContainer(CDTags::Ability_Attack));
	ActivationOwnedTags.AddTag(CDTags::State_Casting);

	ActivationBlockedTags.AddTag(CDTags::State_HitReact);
}

void UCDAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actorInfo, const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData * triggerEventData)
{
	if (!CommitAbility(handle, actorInfo, activationInfo))
	{
		EndAbility(handle, actorInfo, activationInfo, true, true);
		return;
	}

	bCanCombo = false;
	currentCombo = 1;

	const ACDCharacterBase* character = GetCDCharacter();
	character->GetController()->StopMovement();

	// 10.26 - Jun6 마우스 방향으로 회전
	const FVector aimLocation = character->GetAimLocation();
	BeginCastTowards(aimLocation);

	UAbilityTask_PlayMontageAndWait* playAttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, montage, 1.0f);
	playAttackTask->OnCompleted.AddDynamic(this, &ThisClass::OnCompleteCallback);
	playAttackTask->OnBlendOut.AddDynamic(this, &ThisClass::OnCompleteCallback);
	playAttackTask->OnInterrupted.AddDynamic(this, &ThisClass::OnInterruptedCallback);
	playAttackTask->OnCancelled.AddDynamic(this, &ThisClass::OnInterruptedCallback);
	playAttackTask->ReadyForActivation();

	// 몽타주 내 공격 노티파이 수신 기능
	UAbilityTask_WaitGameplayEvent* event = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CDTags::Event_Montage, nullptr, false, false);
	event->EventReceived.AddDynamic(this, &ThisClass::OnAnimNotifyCallback);
	event->ReadyForActivation();
}

void UCDAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (bCanCombo)
	{
		bCanCombo = false;

		if (montage->GetNumSections() < currentCombo + 1)
		{
			return;
		}

		// 10.26 - Jun6 마우스 방향으로 회전
		const ACDCharacterBase* character = GetCDCharacter();
		const FVector aimLocation = character->GetAimLocation();
		BeginCastTowards(aimLocation);

		currentCombo += 1;
		FName nextSection = FName(*FString::Printf(TEXT("AM_Attack%d"), currentCombo));
		MontageJumpToSection(nextSection);
	}
}

void UCDAttackAbility::ApplyHit()
{
	const ACDCharacterBase* character = GetCDCharacter();
	const FVector forward = character->GetActorForwardVector();

	// 투사체 BP가 설정된 기본 공격
	if (projectileData.projectileClass)
	{
		const FVector direction = forward.GetSafeNormal2D();

		if (direction.IsNearlyZero())
		{
			return;
		}

		FCDProjectileBaseData launchData = projectileData;
		int muzzleIndex = ((currentCombo + 1) % 2) + 1;
		const FVector origin = character->GetMesh()->GetSocketLocation(FName(*FString::Printf(TEXT("Muzzle_0%d"), muzzleIndex)));

		launchData.launchLocation =	origin + direction * projectileSpawnDistance;

		// 직선형은 이 지점으로부터 진행 방향을 계산
		launchData.targetPoint = launchData.launchLocation + direction * 1000.f;

		SpawnProjectile(launchData, damageCoefficient);

		// 11.03 환영 버프: 잔영이 같은 자리 · 방향으로 한 발 더
		if (UCDPhantomComponent* phantom = character->FindComponentByClass<UCDPhantomComponent>())
		{
			phantom->QueueEcho(this, [this, launchData]()
				{
					SpawnProjectile(launchData, damageCoefficient);
				});
		}

		// 기존 즉발 타격 처리는 실행하지 않음
		return;
	}

	const FVector origin = character->GetActorLocation();
	TArray<AActor*> hitActors;
	hitShape.FindActors(GetWorld(), origin, forward, hitActors);

	// 11.03 환영 버프: 맞은 적을 모아 두었다가 잔영이 한 번 더 때림
	TArray<TWeakObjectPtr<AActor>> echoTargets;
	for (AActor* actor : hitActors)
	{
		if (character->IsHostileTo(actor))
		{
			echoTargets.Add(actor);
			ApplyDamageTo(actor);

			// 10.28 Jun6 - 각성기 게이지 관련 코드 추가
			const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(UCDAwakenEffect::StaticClass(), GetAbilityLevel());

			ICDMonsterDataInterface* interface = Cast<ICDMonsterDataInterface>(actor);
			UCDRunSubsystem* runSub = GetWorld()->GetGameInstance()->GetSubsystem<UCDRunSubsystem>();
			if (nullptr == interface || nullptr == runSub)
			{
				return;
			}

			ECDMonsterGrade grade = interface->GetMonsterGrade();
			int modifier = static_cast<int>(grade) + 1;

			FCDRunRecord rec = runSub->GetRecord();
			ECDDifficulty dif = rec.difficulty;
			int difficulty = 3 - static_cast<int>(dif);


			// 난이도/등급별 | 폭탄 | 일반 | 엘리트 | 보스
			//	   이지		   3	  6		 9      12
			//	   노말		   2	  4		 6		8
			//	   하드		   1	  2		 3		4
			// -> / 5.0f
			specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Awakening, static_cast<float>(modifier * difficulty) / 5.0f);
			GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(*specHandle.Data);

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(actor, hitNiagara, actor->GetActorLocation(), actor->GetActorRotation());
		}
	}

	QueueEchoHit(echoTargets);

	if (CVarDebugHitShape.GetValueOnGameThread())
	{
		hitShape.DrawDebug(GetWorld(), origin, forward, FColor::Red, 1.f);
	}
}

void UCDAttackAbility::QueueEchoHit(const TArray<TWeakObjectPtr<AActor>>& targets)
{
	if (targets.IsEmpty())
	{
		return;
	}

	const ACDCharacterBase* character = GetCDCharacter();
	UCDPhantomComponent* phantom = character ? character->FindComponentByClass<UCDPhantomComponent>() : nullptr;
	if (phantom == nullptr)
	{
		return;
	}

	// 버프 중이 아니면 QueueEcho 가 무시. 각성 게이지는 원래 타격에서만
	phantom->QueueEcho(this, [this, targets]()
		{
			const ACDCharacterBase* owner = GetCDCharacter();
			if (owner == nullptr)
			{
				return;
			}

			for (const TWeakObjectPtr<AActor>& weakTarget : targets)
			{
				AActor* target = weakTarget.Get();
				if ((target == nullptr) || (owner->IsHostileTo(target) == false))
				{
					continue;
				}

				ApplyDamageTo(target);
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(target, hitNiagara, target->GetActorLocation(), target->GetActorRotation());
			}
		});
}

void UCDAttackAbility::ApplyDamageTo(AActor* target) const
{
	UAbilitySystemComponent* targetAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(target);
	if (!targetAbilitySystem)
	{
		return;
	}

	const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(UCDDamageEffect::StaticClass(), GetAbilityLevel());
	specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Coefficient, damageCoefficient);
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*specHandle.Data, targetAbilitySystem);
}

float UCDAttackAbility::GetCooldownDuration() const
{
	return 0.0f;
}

FGameplayTag UCDAttackAbility::GetCooldownTag() const
{
	return FGameplayTag();
}

void UCDAttackAbility::OnCompleteCallback()
{
	bCanCombo = false;
	currentCombo = 1;
		
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UCDAttackAbility::OnInterruptedCallback()
{
	UE_LOG(LogTemp, Log, TEXT("OnInterruptedCallback"));
	bCanCombo = false;
	currentCombo = 1;

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

void UCDAttackAbility::OnAnimNotifyCallback(FGameplayEventData eventData)
{
	if (eventData.EventTag == CDTags::Event_Montage_Hit)
	{
		ApplyHit();
	}
	else if (eventData.EventTag == CDTags::Event_Montage_ComboStart)
	{
		bCanCombo = true;
	}
	else if (eventData.EventTag == CDTags::Event_Montage_ComboEnd)
	{
		bCanCombo = false;
	}
}
