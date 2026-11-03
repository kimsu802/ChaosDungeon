#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CDAnimNotifyState_GhostTrail.generated.h"

class ACDGhostTrailActor;
class UMaterialInterface;

/**
 * 노티파이 구간 동안 spawnInterval 마다 잔영(ACDGhostTrailActor)을 남긴다. (회피 · 돌진 · 각성기 Release 등)
 * 노티파이 객체는 몽타주 에셋에 하나만 있고 그 몽타주를 쓰는 모든 캐릭터가 공유한다.
 * → 캐릭터별 누적 시간은 메시 컴포넌트를 키로 따로 보관 (멤버 float 하나로 두면 캐릭터끼리 섞임)
 */
UCLASS(meta = (DisplayName = "Ghost Trail"))
class CHAOSDUNGEON_API UCDAnimNotifyState_GhostTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/** 기본 잔영 클래스 지정 */
	UCDAnimNotifyState_GhostTrail();

	// UAnimNotifyState::NotifyBegin()
	virtual void NotifyBegin(USkeletalMeshComponent* meshComp, UAnimSequenceBase* animation, float totalDuration, const FAnimNotifyEventReference& eventReference) override;

	// UAnimNotifyState::NotifyTick()
	virtual void NotifyTick(USkeletalMeshComponent* meshComp, UAnimSequenceBase* animation, float frameDeltaTime, const FAnimNotifyEventReference& eventReference) override;

	// UAnimNotifyState::NotifyEnd()
	virtual void NotifyEnd(USkeletalMeshComponent* meshComp, UAnimSequenceBase* animation, const FAnimNotifyEventReference& eventReference) override;

	// UAnimNotifyState::GetNotifyName_Implementation()
	virtual FString GetNotifyName_Implementation() const override;

private:
	/** 현재 포즈로 잔영 1개 생성 */
	void SpawnGhost(USkeletalMeshComponent* meshComp) const;

protected:
	/** 잔영 액터 클래스 (BP 자식으로 바꿔 끼울 수 있음) */
	UPROPERTY(EditAnywhere, Category = "Ghost")
	TSubclassOf<ACDGhostTrailActor> ghostClass;

	/** 잔영 머티리얼 (Translucent, 스칼라 파라미터 Opacity 필수 → ACDGhostTrailActor::opacityParamName) */
	UPROPERTY(EditAnywhere, Category = "Ghost")
	TObjectPtr<UMaterialInterface> ghostMaterial;

	/** 잔영 생성 간격 (애니메이션 시간 기준 초) */
	UPROPERTY(EditAnywhere, Category = "Ghost", meta = (ClampMin = 0.01))
	float spawnInterval = 0.05f;

	/** 잔영 하나가 사라지기까지 걸리는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Ghost", meta = (ClampMin = 0.01))
	float ghostLifeTime = 0.35f;

	/** 구간 시작 프레임에 바로 1개 생성 */
	UPROPERTY(EditAnywhere, Category = "Ghost")
	bool bSpawnOnBegin = true;

private:
	/** 메시별 다음 잔영까지 남은 시간 (공유 노티파이라 캐릭터마다 분리) */
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, float> timeUntilNext;
};
