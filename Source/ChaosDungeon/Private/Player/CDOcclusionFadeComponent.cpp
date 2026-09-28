#include "Player/CDOcclusionFadeComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UCDOcclusionFadeComponent::UCDOcclusionFadeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UCDOcclusionFadeComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

	const APawn* ownerPawn = GetOwner<APawn>();
	const APlayerController* playerController = ownerPawn ? ownerPawn->GetController<APlayerController>() : nullptr;
	if (!playerController)
	{
		return;
	}

	TArray<FHitResult> hits;
	const FCollisionQueryParams queryParams(SCENE_QUERY_STAT(OcclusionFade), false, ownerPawn);
	GetWorld()->SweepMultiByChannel(hits, playerController->PlayerCameraManager->GetCameraLocation(), ownerPawn->GetActorLocation(),
		FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(probeRadius), queryParams);

	TArray<TWeakObjectPtr<UPrimitiveComponent>> occluders;
	for (const FHitResult& hit : hits)
	{
		if (UStaticMeshComponent* mesh = Cast<UStaticMeshComponent>(hit.GetComponent()))
		{
			occluders.AddUnique(mesh);
		}
	}

	for (const TWeakObjectPtr<UPrimitiveComponent>& previous : fadedPrimitives)
	{
		if (previous.IsValid() && !occluders.Contains(previous))
		{
			SetFaded(previous.Get(), false);
		}
	}
	for (const TWeakObjectPtr<UPrimitiveComponent>& occluder : occluders)
	{
		SetFaded(occluder.Get(), true);
	}
	fadedPrimitives = MoveTemp(occluders);
}

void UCDOcclusionFadeComponent::SetFaded(UPrimitiveComponent* primitive, bool bFaded) const
{
	primitive->SetScalarParameterForCustomPrimitiveData(fadeParameterName, bFaded ? 1.f : 0.f);
}
