#include "World/CDInteractable.h"
#include "Core/CDGameplayTags.h"
#include "UI/CDUIManagerSubsystem.h"

ACDInteractable::ACDInteractable()
{
	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ACDInteractable::Interact(APlayerController* interactor) const
{
	if (UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(interactor))
	{
		uiManager->PushScreen(CDTags::UI_Layer_Modal, screenClass);
	}
}
