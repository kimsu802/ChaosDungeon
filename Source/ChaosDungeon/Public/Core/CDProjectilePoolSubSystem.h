#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CDProjectilePoolSubSystem.generated.h"

class ACDProjectileBase;
class ACDReticleActor;
class UCDSkillData;

USTRUCT()
struct FCDProjectilePoolBucket
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACDProjectileBase>> actors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACDProjectileBase>> available;

	UPROPERTY(Transient)
	TSubclassOf<ACDReticleActor> reticleClass;

	UPROPERTY(Transient)
	TObjectPtr<const UCDSkillData> skill;

	int32 targetCount = 0;
};

/** Scatter가 명시적으로 선택하는 투사체, 착탄 표시 묶음 풀. */
UCLASS()
class CHAOSDUNGEON_API UCDProjectilePoolSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 프레임당 한 묶음씩 미리 준비 이미 확보한 수량은 다시 생성 X.
	void Prewarm(TSubclassOf<ACDProjectileBase> projectileClass, int32 targetCount,
		TSubclassOf<ACDReticleActor> reticleClass = nullptr, const UCDSkillData* skill = nullptr);

	bool IsPooledClass(TSubclassOf<ACDProjectileBase> projectileClass) const;

	ACDProjectileBase* Acquire(TSubclassOf<ACDProjectileBase> projectileClass);

	void Release(ACDProjectileBase* projectile);

	virtual void Deinitialize() override;

private:
	void TickPrewarm();
	ACDProjectileBase* CreateProjectile(TSubclassOf<ACDProjectileBase> projectileClass);

	UPROPERTY(Transient)
	TMap<TSubclassOf<ACDProjectileBase>, FCDProjectilePoolBucket> pools;

	FTimerHandle prewarmTimer;
	bool bShuttingDown = false;
};
