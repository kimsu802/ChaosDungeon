// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDMinigameBomb.generated.h"

class UDecalComponent;
class UMaterialInterface;
class UNiagaraSystem;
class UGameplayEffect;


UCLASS()
class ACDMinigameBomb : public AActor
{
	GENERATED_BODY()
	
public:	
	/* 생성 전에 패턴 정보 입력 - yoonsang */
	void InitializePattern(const FVector& newCenter, const FVector& newDirection, float newRadius, bool newISConePattern);

	/* 충돌 위치에서 폭발 효과와 피해 적용 - yoonsang*/
	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void Explode(FVector impactLocation);


protected:
	// AActor::BeginPlay()
	virtual void BeginPlay() override;

	// AActor::EndPlay()
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

private:
	/* 경고 원 생성 - yoonsang */
	void CreateWarning();

	/* 데미지 적용 - yoonsang */
	void ApplyExplosionDamage();

	/* 해당 위치가 피해 범위 안인지 확인 - yoonsang */
	bool IsInsidePattern(const FVector& location) const;

protected:
	/* 폭발 재생 나이아가라 - yoonsang */
	UPROPERTY(EditDefaultsOnly, Category = "Bomb")
	TObjectPtr<UNiagaraSystem> explosionEffect;

	/* 폭발 데미지 적용 - yoonsang */
	UPROPERTY(EditDefaultsOnly, Category = "Bomb")
	TSubclassOf<UGameplayEffect> damageEffectClass;

	/* 원형 범위 표시 머티리얼 - yoonsang*/
	UPROPERTY(EditDefaultsOnly, Category = "BombWarning")
	TObjectPtr<UMaterialInterface> warningMeaterial;

	/* 원 반지름 GameMode에서 전달받음 - yoonsang */
	UPROPERTY(VisibleInstanceOnly, Category = "BombWarning")
	float warningRadius = 0.f;

	/* 원 색상 - yoonsang */
	UPROPERTY(EditDefaultsOnly, Category = "BombWarning")
	FLinearColor warningColor = FLinearColor::Red;

	/* 데칼 방향 지정 - yoonsang */
	UPROPERTY(EditDefaultsOnly, Category = "BombWarning")
	float warningYawOffset = 0.f;

private:
	/* 경고 원 정리 - yoonsang */
	UPROPERTY(Transient)
	TObjectPtr<UDecalComponent> warningDecal;

	/* 중복 처리 방지 - yoonsang */
	bool bHasExploded = false;

	/* 기준 위치 - yoonsang */
	FVector patternCenter = FVector::ZeroVector;

	/* 콘 폭탄 방향 - yoonsang */
	FVector patternDirection = FVector::ForwardVector;

	/* 콘 범위 변경 - yoonsang */
	bool bISConePattern = false;

	/* 패턴 초기화 여부 - yoonsang*/
	bool bIsPatternInitialized = false;

};
