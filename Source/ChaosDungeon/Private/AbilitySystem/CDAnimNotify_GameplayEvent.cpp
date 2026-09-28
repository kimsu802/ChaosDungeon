#include "AbilitySystem/CDAnimNotify_GameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UCDAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* meshComp, UAnimSequenceBase* animation, const FAnimNotifyEventReference& eventReference)
{
	Super::Notify(meshComp, animation, eventReference);
	if (meshComp && meshComp->GetOwner())
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(meshComp->GetOwner(), eventTag, FGameplayEventData());
	}
}

FString UCDAnimNotify_GameplayEvent::GetNotifyName_Implementation() const
{
	if (eventTag.IsValid())
	{
		return eventTag.ToString();
	}
	return Super::GetNotifyName_Implementation();
}
