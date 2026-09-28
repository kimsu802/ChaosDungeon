#pragma once

#include "GameFramework/Actor.h"
#include "CDInteractable.generated.h"

class UCommonActivatableWidget;
class APlayerController;

/**
 * 허브의 클릭 상호작용 대상 (NPC: 난이도 선택/입장, 기록 게시판)
 * 둘 다 "가까이 가서 클릭 → 화면 열기" 이므로 인터페이스/자식 클래스 없이 screenClass 값만 다르게 쓴다.
 * 메시는 BP 에서 추가하고 CustomDepthStencilValue = CDStencil::Npc 로 설정.
 */
UCLASS()
class CHAOSDUNGEON_API ACDInteractable : public AActor
{
	GENERATED_BODY()

public:
	/** 루트 생성 */
	ACDInteractable();

	/** 화면 열기 */
	void Interact(APlayerController* interactor) const;

	/** 상호작용 가능 거리 */
	FORCEINLINE float GetInteractRange() const
	{
		return interactRange;
	}

protected:
	/** 열 화면 (Modal 레이어) */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TSoftClassPtr<UCommonActivatableWidget> screenClass;

	/** 상호작용 가능 거리 */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float interactRange = 250.f;
};
