#include "Player/CDCursorHighlightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "Character/CDPlayerCharacter.h"
#include "Input/CommonUIActionRouterBase.h"


UCDCursorHighlightComponent::UCDCursorHighlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

void UCDCursorHighlightComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

	const APlayerController* playerController = GetOwner<APlayerController>();

	if (!playerController)
	{
		return;
	}
	
	if (!IsGameInputActive(playerController))
	{
		ClearHighlight();
		return;
	}

	FHitResult hit;
	playerController->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, hit);

	AActor* newHovered = hit.GetActor();
	//UE_LOG(LogTemp, Log, TEXT("Hover: %s / %s"), *GetNameSafe(hit.GetActor()), *GetNameSafe(hit.GetComponent()));

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
	if (!actor)
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

void UCDCursorHighlightComponent::ClearHighlight()
{
	SetHighlight(hoveredActor.Get(), false);
	hoveredActor = nullptr;
}

bool UCDCursorHighlightComponent::IsGameInputActive(const APlayerController* playerController) const
{
	const UCommonUIActionRouterBase* actionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(playerController->GetLocalPlayer());
	if (!actionRouter)
	{
		return true;
	}

	return actionRouter->GetActiveInputMode(ECommonInputMode::Game) != ECommonInputMode::Menu;
}
