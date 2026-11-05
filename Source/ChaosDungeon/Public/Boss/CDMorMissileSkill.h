#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CDMonsterSkillAbility.h"
#include "CDMorMissileSkill.generated.h"

UCLASS()
class CHAOSDUNGEON_API UCDMorMissileSkill : public UCDMonsterSkillAbility
{
	GENERATED_BODY()

protected:
	// UCDMonsterSkillAbility::ApplyHit()
	virtual void ApplyHit() override;

	// UCDMonsterSkillAbility::ShowReticle()
	virtual void ShowReticle() override;

protected:
	// 보스 중심에서 위쪽 생성 높이
	UPROPERTY(EditDefaultsOnly, Category = "Missile")
	float spawnHeight = 100.f;

	// 투사체 사이의 좌우 간격
	UPROPERTY(EditDefaultsOnly, Category = "Missile",
		meta = (ClampMin = "0"))
	float missileSpacing = 80.f;

	// 위로 상승하는 속도
	UPROPERTY(EditDefaultsOnly, Category = "Missile",
		meta = (ClampMin = "1"))
	float riseSpeed = 600.f;

	// 플레이어 방향으로 발사될 때 실행할 GCN
	UPROPERTY(EditDefaultsOnly, Category = "Missile",
		meta = (Categories = "GameplayCue"))
	FGameplayTag launchCueTag;
};