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

 // 포탈의 사용 목적.
UENUM(BlueprintType)
enum class ECDPortalType : uint8
{
	DungeonEntrance UMETA(DisplayName = "Dungeon Entrance"),
	NextStage UMETA(DisplayName = "Next Stage")
};

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

	/* 문 외형 유지, 진입 감지만 bCanEnter로 감지 */
	void SetOpen(bool bOpen);

protected:
	/** 진입 감지 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> trigger;

	// 맵에 배치한 포탈의 사용 목적.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	ECDPortalType portalType = ECDPortalType::NextStage;

private:
	/** Stage.Cleared 구독 핸들 */
	FCDListenerHandle clearedHandle;

	/** 중복 진입 방지 */
	bool bEntered = false;

	/* 플레이어의 포탈 진입을 허용 여부 판별 */
	bool bCanEnter = false;
};
