#include "Player/CDCursorHighlightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"

UCDCursorHighlightComponent::UCDCursorHighlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

void UCDCursorHighlightComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

	const APlayerController* playerController = GetOwner<APlayerController>();
	FHitResult hit;
	playerController->GetHitResultUnderCursor(ECC_Visibility, false, hit);

	AActor* newHovered = hit.GetActor();
	if (newHovered == hoveredActor.Get())
	{
		return;
	}
	SetHighlight(hoveredActor.Get(), false);
	SetHighlight(newHovered, true);
	hoveredActor = newHovered;
}

void UCDCursorHighlightComponent::SetHighlight(AActor* actor, bool bHighlight) const
{
	// 플레이어는 벽 뒤 실루엣 때문에 항상 CustomDepth 를 켜 두므로 건드리지 않는다
	if (!actor || actor == GetOwner<APlayerController>()->GetPawn())
	{
		return;
	}
	auto ApplyHighlight = [bHighlight](UPrimitiveComponent* primitive)
	{
		if (primitive->CustomDepthStencilValue > 0)
		{
			primitive->SetRenderCustomDepth(bHighlight);
		}
	};
	actor->ForEachComponent<UPrimitiveComponent>(false, ApplyHighlight);
}
