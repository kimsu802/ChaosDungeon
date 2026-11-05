// Fill out your copyright notice in the Description page of Project Settings.


#include "World/CDMinigameBomb.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "Core/CDGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "Materials/MaterialInstanceDynamic.h"

void ACDMinigameBomb::InitializePattern(const FVector& newCenter, const FVector& newDirection, float newRadius, bool newISConePattern)
{
	patternCenter = newCenter;

	patternDirection = newDirection.GetSafeNormal2D();

	if (patternDirection.IsNearlyZero())
	{
		patternDirection = FVector::ForwardVector;
	}

	warningRadius = FMath::Max(newRadius, 1.f);

	bISConePattern = newISConePattern;
	
	/* 패턴 형태 초기화 - yoonsang */
	bIsPatternInitialized = true;
}

// Called when the game starts or when spawned
void ACDMinigameBomb::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bIsPatternInitialized)
	{
		return;
	}

	CreateWarning();
	
}

void ACDMinigameBomb::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	if (IsValid(warningDecal))
	{
		warningDecal->DestroyComponent();
		warningDecal = nullptr;
	}

	Super::EndPlay(endPlayReason);
}

void ACDMinigameBomb::CreateWarning()
{
	if (!warningMeaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("Setting Warning Material."));

		return;
	}

	const float directionYaw = patternDirection.Rotation().Yaw;

	const FRotator decalRotation(90.f, directionYaw + warningYawOffset, 0.f);

	warningDecal = UGameplayStatics::SpawnDecalAtLocation(this, warningMeaterial, FVector(50.f, warningRadius, warningRadius), patternCenter, decalRotation);

	if (!IsValid(warningDecal))
	{
		return;
	}

	UMaterialInstanceDynamic* materialInstance = warningDecal->CreateDynamicMaterialInstance();

	materialInstance->SetScalarParameterValue(TEXT("ConeAngle"), bISConePattern ? 90.f : 360.f);

	materialInstance->SetScalarParameterValue(TEXT("InnerRadiusRatio"), 0.f);

	materialInstance->SetVectorParameterValue(TEXT("BaseColor"), warningColor);
}

void ACDMinigameBomb::ApplyExplosionDamage()
{
	UWorld* world = GetWorld();

	TArray<FOverlapResult> overlapResults;

	FCollisionObjectQueryParams objectParams;
	objectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	world->OverlapMultiByObjectType(overlapResults, patternCenter, FQuat::Identity, objectParams, FCollisionShape::MakeSphere(warningRadius), queryParams);

	TSet<AActor*> damagedActors;

	for (const FOverlapResult& overlap : overlapResults)
	{
		APawn* player = Cast<APawn>(overlap.GetActor());

		if (!IsValid(player) || !player->IsPlayerControlled() || damagedActors.Contains(player))
		{
			continue;
		}

		damagedActors.Add(player);

		if (!IsInsidePattern(player->GetActorLocation()))
		{
			continue;
		}

		UAbilitySystemComponent* abilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(player);

		if (!abilitySystem || abilitySystem->HasMatchingGameplayTag(CDTags::State_Invincible))
		{
			continue;
		}

		FGameplayEffectContextHandle effectContext = abilitySystem->MakeEffectContext();

		effectContext.AddInstigator(this, this);

		FGameplayEffectSpecHandle effectSpec = abilitySystem->MakeOutgoingSpec(damageEffectClass, 1.0f, effectContext);

		if (effectSpec.IsValid())
		{
			abilitySystem->ApplyGameplayEffectSpecToSelf(*effectSpec.Data.Get());
		}
	}
}

bool ACDMinigameBomb::IsInsidePattern(const FVector& location) const
{
	FVector offset = location - patternCenter;
	offset.Z = 0.f;

	const float distanceSquared = offset.SizeSquared();

	/* 바깥 반지름을 벗어나면 피해 X */
	if (distanceSquared > FMath::Square(warningRadius))
	{
		return false;
	}

	/* 원형이면 거리 검사만 통과 */
	if (!bISConePattern)
	{
		return true;
	}

	/* 내적, 외적 각도 판정 코드 */
	const FVector directionToPlayer = offset.GetSafeNormal2D();

	const float forwardDot = FVector::DotProduct(patternDirection, directionToPlayer);

	const float crossZ = FVector::CrossProduct(patternDirection, directionToPlayer).Z;

	const float angle = FMath::RadiansToDegrees(FMath::Atan2(crossZ, forwardDot));

	return angle >= -45.f && angle < 45.f;
}

void ACDMinigameBomb::Explode(FVector impactLocation)
{
	if (bHasExploded || !bIsPatternInitialized)
	{
		return;
	}

	bHasExploded = true;

	SetActorEnableCollision(false);
	
	if (explosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, explosionEffect, impactLocation);
	}

	if (HasAuthority())
	{
		ApplyExplosionDamage();
	}

	Destroy();
}

