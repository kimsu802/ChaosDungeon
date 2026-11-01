// Fill out your copyright notice in the Description page of Project Settings.


#include "World/CDMinigameBomb.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "Core/CDGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystemComponent.h"

// Called when the game starts or when spawned
void ACDMinigameBomb::BeginPlay()
{
	Super::BeginPlay();

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
	}

	UWorld* world = GetWorld();

	/* 생성 직 후 아래 방향으로 검사 -yoonsang */
	const FVector start = GetActorLocation();
	const FVector end = start - FVector(0.0f, 0.0f, groundTraceDistance);

	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	APawn* player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (IsValid(player))
	{
		queryParams.AddIgnoredActor(player);
	}

	FHitResult hitResult;

	const bool hitGround = world->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, queryParams);

	warningDecal = UGameplayStatics::SpawnDecalAtLocation(this, warningMeaterial, FVector(50.0f, warningRadius, warningRadius), 
		hitResult.ImpactPoint, FRotator(90.0f, 0.0f, 0.0f));

	UMaterialInstanceDynamic* materialInstance =
		warningDecal->CreateDynamicMaterialInstance();

	if (materialInstance)
	{
		materialInstance->SetVectorParameterValue(
			TEXT("BaseColor"),
			warningColor);
	}
}

void ACDMinigameBomb::ApplyExplosionDamage(const FVector& impactLocation)
{
	UWorld* world = GetWorld();

	TArray<FOverlapResult> overlapResults;

	FCollisionObjectQueryParams objectParams;
	objectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	world->OverlapMultiByObjectType(
		overlapResults,
		impactLocation,
		FQuat::Identity,
		objectParams,
		FCollisionShape::MakeSphere(warningRadius),
		queryParams);

	TSet<AActor*> damagedActors;

	for (const FOverlapResult& overlap : overlapResults)
	{
		APawn* player = Cast<APawn>(overlap.GetActor());

		if (!IsValid(player)
			|| !player->IsPlayerControlled()
			|| damagedActors.Contains(player))
		{
			continue;
		}

		damagedActors.Add(player);

		UAbilitySystemComponent* abilitySystem =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
				player);

		if (!abilitySystem
			|| abilitySystem->HasMatchingGameplayTag(
				CDTags::State_Invincible))
		{
			continue;
		}

		FGameplayEffectContextHandle effectContext =
			abilitySystem->MakeEffectContext();

		effectContext.AddInstigator(this, this);

		FGameplayEffectSpecHandle effectSpec =
			abilitySystem->MakeOutgoingSpec(
				damageEffectClass,
				1.0f,
				effectContext);

		if (effectSpec.IsValid())
		{
			abilitySystem->ApplyGameplayEffectSpecToSelf(
				*effectSpec.Data.Get());
		}
	}
}

void ACDMinigameBomb::Explode(FVector impactLocation)
{
	if (bHasExploded)
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
		ApplyExplosionDamage(impactLocation);
	}

	Destroy();
}

