#pragma once

#include "Components/ActorComponent.h"
#include "CDCursorHighlightComponent.generated.h"

/**
 * 커서 아래 액터 외곽선 (PlayerController 에 부착)
 * 대상별 색은 각 메시의 CustomDepthStencilValue 로 구분되므로 여기서는 켜고 끄기만 한다.
 * 스텐실 값이 0 인 액터(바닥, 벽 등)는 무시.
 */

class APlayerController;

UCLASS(ClassGroup = (CD), meta = (BlueprintSpawnableComponent))
class CHAOSDUNGEON_API UCDCursorHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 틱 설정 */
	UCDCursorHighlightComponent();

	// UActorComponent::TickComponent()
	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

private:
	/** 액터의 CustomDepth 켜기/끄기 */
	void SetHighlight(AActor* actor, bool bHighlight) const;

	/* 액터의 외곽선 처리를 모두 해제하고 참조 비우기*/
	void ClearHighlight();
	
	/* 현재 입력 모드가 Game으로 되어있는지 판단 (UI가 띄워져있을때는 인게임 내 액터 외곽선 처리 방지하기 위해)*/
	bool IsGameInputActive(const APlayerController* playerController) const;

private:
	/** 현재 커서 아래 액터 */
	TWeakObjectPtr<AActor> hoveredActor;
};
