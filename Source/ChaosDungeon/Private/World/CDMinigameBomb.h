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


protected:
	// AActor::BeginPlay()
	virtual void BeginPlay() override;

	// AActor::EndPlay()
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

private:
	/* 경고 원 생성 - yoonsang */
	void CreateWarning();

	/* 데미지 적용 - yoonsang */
	void ApplyExplosionDamage(const FVector& impactLocation);

public:
	/* 충돌 위치에서 폭발 효과와 피해 적용 - yoonsang*/
	UFUNCTION(BlueprintCallable, Category = "Bomb")
	void Explode(FVector impactLocation);

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

	/* 원 반지름 - yoonsang */
	UPROPERTY(EditDefaultsOnly, Category = "BombWarning", meta = (ClampMin = "1.0"))
	float warningRadius = 200.0f;

	/* 원 색상 - yoonsang */
	UPROPERTY(EditDefaultsOnly, Category = "BombWarning")
	FLinearColor warningColor = FLinearColor::Red;
	
	/* 검사 거리 - yoonsang */
	UPROPERTY(EditDefaultsOnly, Category = "BombWarning", meta = (ClampMin = "1.0"))
	float groundTraceDistance = 5000.0f;

private:
	/* 경고 원 정리 - yoonsang */
	UPROPERTY(Transient)
	TObjectPtr<UDecalComponent> warningDecal;

	/* 중복 처리 방지 - yoonsang */
	bool bHasExploded = false;
};
