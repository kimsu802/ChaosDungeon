#pragma once

#include "GameFramework/Actor.h"
#include "Core/CDMessageSubsystem.h"
#include "CDPortal.generated.h"

class UBoxComponent;
struct FCDStageMessage;

/**
 * 스테이지 클리어 시 열리는 다음 단계 포탈. 레벨에 미리 배치(비활성)
 * 구독: Stage.Cleared → 열림 / 발행: Stage.PortalEntered (GameMode 가 다음 스테이지로 이동)
 */
UCLASS()
class CHAOSDUNGEON_API ACDPortal : public AActor
{
	GENERATED_BODY()

public:
	/** 트리거 생성 */
	ACDPortal();

protected:
	// AActor::BeginPlay()
	virtual void BeginPlay() override;

	// AActor::EndPlay()
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

	/** 포탈이 열릴 때 이펙트/사운드 (BP 구현) */
	UFUNCTION(BlueprintImplementableEvent)
	void OnOpened();

private:
	/** Stage.Cleared 수신 → 열기 */
	void HandleStageCleared(const FCDStageMessage& message);

	/** 플레이어 진입 → Stage.PortalEntered 발행 */
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
		int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	/** 표시/충돌 켜기/끄기 */
	void SetOpen(bool bOpen);

protected:
	/** 진입 감지 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> trigger;

private:
	/** Stage.Cleared 구독 핸들 */
	FCDListenerHandle clearedHandle;

	/** 중복 진입 방지 */
	bool bEntered = false;
};
