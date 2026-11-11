// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Data/CDProjectileData.h>
#include "GameplayTagContainer.h"
#include "CDProjectileBase.generated.h"


/*
* 투사체 한 개를 관리하는 액터
* 기본적인 Projectile 정보가 들어감
* Mesh, 판정, 데미지 적용, 수명
* PMC 적용
*/

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class ACDReticleActor;
class UCDProjectilePoolSubSystem;

UCLASS()
class CHAOSDUNGEON_API ACDProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ACDProjectileBase();

	virtual void Tick(float deltaSeconds) override;

	// ProjectileBaseData 초기화
	void InitializeProjectile(const FCDProjectileBaseData& inData);

	bool IsProjectileActive() const { return bProjectileActive; }

	/** 이 투사체의 착탄 표시 등록 */
	void SetLandingReticle(ACDReticleActor* reticle);
	ACDReticleActor* GetLandingReticle() const { return landingReticle.Get(); }

	// 공중에서 이동과 캐릭터 명중 검사 중지
	void PauseFlight();

	// 지정 위치로 방향을 확정하고 직선 발사
	void LaunchAtLocation(const FVector& location, float speed, const FGameplayTag& launchCueTag);

protected:
	// 투사체 움직임 
	virtual bool ConfigureMovement();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	virtual void LifeSpanExpired() override;

	/** 풀링 여부에 따라 반환하거나 파괴하는 공통 종료 경로 */
	void FinishProjectile();

	// 이전 위치부터 현재 위치까지 명중 검사
	void TraceHit(const FVector& start, const FVector& end);

	// 맞혔을 때 실행
	virtual void OnTargetHit(const FHitResult& hit);

	void ApplyImpactDamage();

	// 비행 중 캐릭터 명중 검사
	void SetFlightHitEnabled(bool bEnabled)
	{
		previousLocation = GetActorLocation();
		bTraceEnabled = bEnabled;
	}

protected:
	// 충돌로 투사체 이동이 멈췄을 때 실행
	UFUNCTION()
	virtual void OnProjectileStop(const FHitResult& hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Component")
	TObjectPtr<USphereComponent> sphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Component")
	TObjectPtr<UProjectileMovementComponent> projectileMove;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Component")
	TObjectPtr<UStaticMeshComponent> meshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile | Component")
	TObjectPtr<UNiagaraComponent> niagaraComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile | Component")
	TObjectPtr<UNiagaraSystem> hitComp;

	UPROPERTY(Transient)
	FCDProjectileBaseData projectileData;

	UPROPERTY(EditAnywhere, Category  = "Projectile | Impact", meta = (Categories = "GameplayCue"))
	FGameplayTag impactCueTag;

private:
	friend class UCDProjectilePoolSubSystem;

	void InitializePool(UCDProjectilePoolSubSystem* pool);
	void DeactivateToPool();
	void ClearLandingReticle();

	UPROPERTY(Transient)
	TWeakObjectPtr<UCDProjectilePoolSubSystem> projectilePool;

	bool bProjectileActive = false;

	/** 투사체가 사라질 때 함께 제거할 착탄 표시 */
	UPROPERTY(Transient)
	TObjectPtr<ACDReticleActor> landingReticle = nullptr;

	// 이전 프레임의 투사체 위치
	FVector previousLocation = FVector::ZeroVector;

	// 초기화 완료 후 검사 시작, 최초 명중 후 검사 중지
	bool bTraceEnabled = false;
};
