#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/TimerHandle.h"
#include "CDPhantomComponent.generated.h"

class ACDGhostTrailActor;
class UMaterialInterface;
class UAbilitySystemComponent;

/**
 * 환영 버프 (phantomTag = State.Buff.Phantom) 가 붙어 있는 동안
 *  1) 노란 잔영을 trailInterval 마다 계속 남긴다
 *  2) 스킬 · 평타가 맞으면 그 순간 포즈로 잔영을 남기고, echoDelay 뒤 잔영이 한 번 더 때린다 (어빌리티가 QueueEcho 로 요청)
 * 버프 GE 가 태그를 붙이고 떼는 것만 보고 켜고 끈다 → 지속 시간 · 해제는 GAS 가 관리
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAOSDUNGEON_API UCDPhantomComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 기본 태그 · 잔영 클래스 지정 */
	UCDPhantomComponent();

	/** 환영 버프 중인가 */
	UFUNCTION(BlueprintPure, Category = "Phantom")
	bool IsPhantomActive() const
	{
		return bPhantomActive;
	}

	/**
	 * 잔영 추가 타격 예약: 지금 포즈로 잔영을 하나 남기고 echoDelay 뒤 echo 실행.
	 * context(요청한 어빌리티)가 사라졌으면 실행하지 않는다. 버프 중이 아니면 아무것도 안 하고 false
	 */
	bool QueueEcho(const UObject* context, TFunction<void()> echo);

protected:
	// UActorComponent::BeginPlay()
	virtual void BeginPlay() override;

	// UActorComponent::EndPlay()
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

private:
	/** phantomTag 개수 변화 (0 ↔ 1 이상) */
	void HandlePhantomTagChanged(const FGameplayTag tag, int32 newCount);

	/** 지속 잔영 1개 (타이머) */
	void SpawnTrailGhost();

	/** 현재 포즈로 잔영 1개 */
	void SpawnGhost(float lifeTime) const;

protected:
	/** 이 태그가 붙어 있는 동안 환영 상태 */
	UPROPERTY(EditDefaultsOnly, Category = "Phantom")
	FGameplayTag phantomTag;

	/** 잔영 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Phantom")
	TSubclassOf<ACDGhostTrailActor> ghostClass;

	/** 잔영 머티리얼 (노란색 MI_GhostTrail_Yellow 등) */
	UPROPERTY(EditDefaultsOnly, Category = "Phantom")
	TObjectPtr<UMaterialInterface> ghostMaterial;

	/** 지속 잔영 생성 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Phantom|Trail", meta = (ClampMin = 0.02))
	float trailInterval = 0.1f;

	/** 지속 잔영 수명 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Phantom|Trail", meta = (ClampMin = 0.01))
	float trailLifeTime = 0.4f;

	/** 원래 타격 → 잔영 추가 타격까지 지연 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Phantom|Echo", meta = (ClampMin = 0.01))
	float echoDelay = 0.15f;

	/** 추가 타격 잔영 수명 (echoDelay 보다 길어야 때리는 순간에 보인다) */
	UPROPERTY(EditDefaultsOnly, Category = "Phantom|Echo", meta = (ClampMin = 0.01))
	float echoGhostLifeTime = 0.45f;

private:
	/** 소유 캐릭터의 ASC (태그 이벤트 구독) */
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> abilitySystem;

	/** 태그 이벤트 구독 핸들 */
	FDelegateHandle tagChangedHandle;

	/** 지속 잔영 타이머 */
	FTimerHandle trailTimer;

	/** 환영 버프 중 */
	bool bPhantomActive = false;
};
