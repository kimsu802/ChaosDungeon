#pragma once

#include "GameFramework/Actor.h"
#include "CDPickupOrb.generated.h"

class USphereComponent;
class UGameplayEffect;

/**
 * 회복/버프 구슬. 플레이어가 가까이 오면 날아와서 자동 흡수 → GameplayEffect 적용
 * 회복 구슬과 버프 구슬은 클래스가 아니라 effect/pickupText 값만 다른 BP
 * 흡수 텍스트는 UI.FloatingText 메시지로 발행 (UI 를 모름)
 */
UCLASS()
class CHAOSDUNGEON_API ACDPickupOrb : public AActor
{
	GENERATED_BODY()

public:
	/** 자석 범위 생성 */
	ACDPickupOrb();

	// AActor::Tick()
	virtual void Tick(float deltaSeconds) override;

private:
	/** 플레이어가 자석 범위에 들어옴 */
	UFUNCTION()
	void OnMagnetOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
		int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	/** 효과 적용 + 텍스트 발행 + 파괴 */
	void Absorb();

protected:
	/** 자석 범위 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> magnetSphere;

	/** 흡수 시 적용할 효과 */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TSubclassOf<UGameplayEffect> effect;

	/** 흡수 시 표시 (예: "공격력 증가"). 비우면 표시 안 함 */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	FText pickupText;

	/** 날아오는 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float flySpeed = 1500.f;

	/** 이 거리 안이면 흡수 */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float absorbDistance = 60.f;

private:
	/** 흡수 대상 */
	TWeakObjectPtr<APawn> target;
};
