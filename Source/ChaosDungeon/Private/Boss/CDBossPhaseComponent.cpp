// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/CDBossPhaseComponent.h"
#include <AbilitySystem/CDAttributeSet.h>
#include <Animation/AnimMontage.h>
#include "Animation/AnimInstance.h"
#include <Character/CDBossCharacter.h>
#include <Components/SkeletalMeshComponent.h>
#include <AI/CDBossAIController.h>
#include <Data/CDBossData.h>

// Sets default values for this component's properties
UCDBossPhaseComponent::UCDBossPhaseComponent()
{
	// 체력 검사는 피격 시 호출하므로 매 프레임 갱신 X
	PrimaryComponentTick.bCanEverTick = false;
}

void UCDBossPhaseComponent::ResetPhase()
{
	currentPhase = ECDBossPhase::Phase1;
	phaseState = ECDBossPhaseState::Combat;

	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetOwner());
	USkeletalMeshComponent* mesh = boss->GetMesh();

	mesh->SetMorphTarget(FName(TEXT("geo_stage_2_Morph")), 0.f);
	mesh->SetMorphTarget(FName(TEXT("geo_stage_2_spine_Morph")), 0.f);
	mesh->SetMorphTarget(FName(TEXT("geo_stage_3_Morph")), 0.f);
}

void UCDBossPhaseComponent::EvaluateHealth()
{
	// 1페이즈 전투 중에만 검사
	if (currentPhase != ECDBossPhase::Phase1 ||
		phaseState != ECDBossPhaseState::Combat)
	{
		return;
	}

	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetOwner());
	if (!IsValid(boss) || boss->IsDead())
	{
		return;
	}

	const UCDAttributeSet* attributes = boss->GetAttributes();
	const UCDBossData* data = boss->GetBossData();
	if (!IsValid(attributes) || !IsValid(data))
	{
		return;
	}

	const float health = attributes->GetHealth();
	const float maxHealth = attributes->GetMaxHealth();

	// 체력이 0이면 페이즈 전환보다 사망 처리를 우선
	if (health <= 0.f || maxHealth <= 0.f)
	{
		return;
	}

	if (health > maxHealth * data->phaseHealthRatio)
	{
		return;
	}

	// 먼저 상태를 바꿔 이후 피격에서 중복 실행되는 것을 막는다.
	phaseState = ECDBossPhaseState::Transitioning;

	UE_LOG(LogTemp, Warning,
		TEXT("Boss phase transition started: HP %.1f / %.1f"),
		health, maxHealth);

	// 무적 적용, AI 정지, 변신 몽타주 재생.
	boss->GrantInvincibility(5.0f);
	
	// AI 정지
	if (ACDBossAIController* ai = Cast<ACDBossAIController>(boss->GetController<ACDBossAIController>()))
	{
		ai->StopBossLogic();

		// 전환 몽타주
		UAnimMontage* montage = data->phaseMontage.Get();
		UAnimInstance* animInstance = boss->GetMesh()->GetAnimInstance();

		if (!IsValid(montage) || !IsValid(animInstance))
		{
			OnPhaseMontageEnded(montage, true);
			return;
		}

		if (boss->PlayAnimMontage(montage) <= 0.f)
		{
			OnPhaseMontageEnded(montage, true);
			return;
		}

		FOnMontageEnded endDelegate;
		endDelegate.BindUObject(
			this, &UCDBossPhaseComponent::OnPhaseMontageEnded);

		animInstance->Montage_SetEndDelegate(endDelegate, montage);

	}
	
	
}

void UCDBossPhaseComponent::OnPhaseMontageEnded(
	UAnimMontage* montage, bool bInterrupted)
{
	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetOwner());

	if (!IsValid(boss) || boss->IsDead() ||
		phaseState != ECDBossPhaseState::Transitioning)
	{
		return;
	}

	if (bInterrupted)
	{
		// 재생 실패·중단 시 1페이즈로 복구
		ResetPhase();
	}
	else
	{
		currentPhase = ECDBossPhase::Phase2;
		phaseState = ECDBossPhaseState::Combat;
	}

	if (ACDBossAIController* ai =
		boss->GetController<ACDBossAIController>())
	{
		USkeletalMeshComponent* mesh = boss->GetMesh();

		ai->StartBossLogic();
	}
}
