#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "CDAnimNotify_GameplayEvent.generated.h"

/**
 * 몽타주 타이밍 → 어빌리티 전달용 노티파이 하나로 통일
 * (Event.Montage.Hit = 타격 시점, Event.Montage.Recovery = 후딜 시작)
 */
UCLASS(meta = (DisplayName = "Send Gameplay Event"))
class CHAOSDUNGEON_API UCDAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	// UAnimNotify::Notify()
	virtual void Notify(USkeletalMeshComponent* meshComp, UAnimSequenceBase* animation, const FAnimNotifyEventReference& eventReference) override;

	// UAnimNotify::GetNotifyName()
	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** 보낼 이벤트 태그 */
	UPROPERTY(EditAnywhere, meta = (Categories = "Event.Montage"))
	FGameplayTag eventTag;
};
