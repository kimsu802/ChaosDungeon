#pragma once

#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "CDPlayerController.generated.h"

class UCDInputConfig;
class UCDCursorHighlightComponent;
class ACDInteractable;

/**
 * 입력 → 명령 변환만 담당한다.
 * - 우클릭: 짧게 = 길찾기 이동, 누르고 있으면 커서 방향으로 계속 이동
 * - 스킬/회피: 입력 태그를 ASC 에 전달 (무슨 스킬인지 모름)
 * - Esc/F1: UIManager 에 위임
 */
UCLASS()
class CHAOSDUNGEON_API ACDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 커서 표시, 외곽선 컴포넌트 생성 */
	ACDPlayerController();

protected:
	// AActor::BeginPlay()
	virtual void BeginPlay() override;

	// APlayerController::SetupInputComponent()
	virtual void SetupInputComponent() override;

	// APlayerController::PlayerTick()
	virtual void PlayerTick(float deltaTime) override;

private:
	/** 우클릭 시작: 이동 초기화 */
	void OnMoveStarted();

	/** 우클릭 유지: 커서 방향으로 이동 */
	void OnMoveTriggered();

	/** 우클릭 해제: 짧게 눌렀으면 길찾기 이동 */
	void OnMoveReleased();

	/** 스킬/회피 입력 */
	void OnAbilityInput(FGameplayTag inputTag);

	/** Esc: 일시정지 메뉴 */
	void OnPause();

	/** F1: 조작법 안내 */
	void OnGuide();

	/** 시전/피격/사망 중에는 이동 불가 */
	bool CanMove() const;

	/** 클릭한 상호작용 대상에 도착했으면 상호작용 */
	void UpdatePendingInteraction();

	/* 목적지 갱신*/
	bool UpdateMoveDestination();

protected:
	/** 입력 설정 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UCDInputConfig> inputConfig;

	/** 이 시간 이하로 누르면 짧은 클릭(길찾기 이동) */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float shortPressThreshold = 0.5f;

	/** 커서 호버 외곽선 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCDCursorHighlightComponent> cursorHighlight;

private:
	/** 우클릭 누른 시간 */
	float movePressedTime = 0.f;

	/** 마지막 이동 목표 */
	FVector moveDestination = FVector::ZeroVector;

	/** 클릭해서 이동 중인 상호작용 대상 */
	TWeakObjectPtr<ACDInteractable> pendingInteraction;
};
