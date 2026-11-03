#pragma once

#include "GameFramework/Actor.h"
#include "CDGhostTrailActor.generated.h"

class UPoseableMeshComponent;
class USkeletalMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * 잔영 1개: 원본 메시의 현재 포즈를 복사해 멈춘 채로 두고, lifeTime 동안 투명도를 내린 뒤 스스로 파괴한다.
 * UCDAnimNotifyState_GhostTrail 이 일정 간격으로 스폰한다.
 * 머티리얼: Translucent(또는 Additive) + 스칼라 파라미터 opacityParamName(기본 Opacity)
 */
UCLASS()
class CHAOSDUNGEON_API ACDGhostTrailActor : public AActor
{
	GENERATED_BODY()

public:
	/** 포즈 메시 생성 (충돌 · 그림자 · 데칼 끔) */
	ACDGhostTrailActor();

	/** 원본 메시 포즈 복사 + 머티리얼 교체 + 수명 설정 (스폰 직후 1회) */
	void Init(USkeletalMeshComponent* source, UMaterialInterface* ghostMaterial, float inLifeTime);

	// AActor::Tick()
	virtual void Tick(float deltaTime) override;

protected:
	/** 포즈를 고정해 두는 메시 */
	UPROPERTY(VisibleAnywhere, Category = "Ghost")
	TObjectPtr<UPoseableMeshComponent> poseMesh;

	/** 페이드할 머티리얼 스칼라 파라미터 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Ghost")
	FName opacityParamName = TEXT("Opacity");

	/** 생성 순간의 불투명도 */
	UPROPERTY(EditDefaultsOnly, Category = "Ghost", meta = (ClampMin = 0, ClampMax = 1))
	float startOpacity = 1.f;

	/** true: 실제 시간으로 사라짐 (슬로우 무시) / false: 월드 시간 (슬로우 중엔 오래 남음) */
	UPROPERTY(EditDefaultsOnly, Category = "Ghost")
	bool bUseRealTime = false;

private:
	/** 모든 머티리얼 슬롯이 공유하는 동적 머티리얼 (슬롯마다 만들 필요 없음) */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ghostMaterialInstance;

	/** 사라지기까지 걸리는 시간 */
	float lifeTime = 0.35f;

	/** 생성 후 경과 시간 */
	float elapsed = 0.f;
};
