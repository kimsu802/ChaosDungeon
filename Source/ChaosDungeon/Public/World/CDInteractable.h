#pragma once

#include "GameFramework/Actor.h"
#include "Core/CDTypes.h"
#include "CDInteractable.generated.h"

class UCommonActivatableWidget;
class APlayerController;

/**
 * 허브의 클릭 상호작용 대상 (NPC: 난이도 선택/입장, 기록 게시판)
 * 둘 다 "가까이 가서 클릭 → 화면 열기" 이므로 인터페이스/자식 클래스 없이 screenClass 값만 다르게 쓴다.
 * 메시는 BP 에서 추가하고 CustomDepthStencilValue = CDStencil::Npc 로 설정.
 * 현재는 문 기능도 포함하게 변경 - yoonsang
 */

UCLASS()
class CHAOSDUNGEON_API ACDInteractable : public AActor
{
	GENERATED_BODY()

public:
	/** 루트 생성 */
	ACDInteractable();

	/** 화면 열기 */
	void Interact(APlayerController* interactor);

	/** 상호작용 가능 거리 */
	FORCEINLINE float GetInteractRange() const
	{
		return interactRange;
	}

	/* 포탈타입 셋 */
	FORCEINLINE void SetPortalType(ECDPortalType newPortalType)
	{
		bUsePortalInteraction = true;
		portalType = newPortalType;
	}

protected:
	/** 열 화면 (Modal 레이어) */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TSoftClassPtr<UCommonActivatableWidget> screenClass;

	/** 상호작용 가능 거리 */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float interactRange = 250.f;

	/* 문 상호 작용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	bool bUsePortalInteraction = false;

	/* 맵에 배치한 포탈의 사용 목적 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	ECDPortalType portalType = ECDPortalType::NextStage;

	/* 확인창 중복 표시 방지 목적 - yoonsang */
	bool bPortalRequested = false;
};
