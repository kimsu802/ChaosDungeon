#pragma once

#include "AIController.h"
#include "CDMonsterAIController.generated.h"

/** 잡몹 FSM 상태 */
UENUM(BlueprintType)
enum class ECDMonsterState : uint8
{
	Idle,
	Chase,
	Attack,
	HitReact,
	Dead
};

/**
 * 잡몹 FSM: 추적 → 공격 → 피격 → 사망
 * - 상태는 GAS 태그(사망/피격/시전)와 거리로 매 틱 판정 → 상태 전이 코드가 한 곳에 모인다
 * - 타겟은 SetFocus 로 공유 → 몬스터는 이 클래스를 몰라도 GetFocusActor() 로 조준
 * - 몰림 회피: DetourCrowd (CrowdFollowingComponent)
 * - 보스 AI(P1)는 이 클래스를 상속하거나 StateTree 로 별도 구현
 */
UCLASS()
class CHAOSDUNGEON_API ACDMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	/** CrowdFollowingComponent 사용, 틱 간격 설정 */
	ACDMonsterAIController(const FObjectInitializer& objectInitializer);

	// AActor::Tick()
	virtual void Tick(float deltaSeconds) override;

	/** 현재 상태 */
	FORCEINLINE ECDMonsterState GetState() const
	{
		return state;
	}

protected:
	/** 태그와 거리로 다음 상태 결정 */
	virtual ECDMonsterState EvaluateState(const AActor* target) const;

	/** 상태 진입 처리 */
	virtual void EnterState(ECDMonsterState newState);

	/** 공격 스킬을 우선순위대로 시도 */
	void TryAttack() const;

private:
	/** 살아있는 플레이어 */
	AActor* FindTarget() const;

protected:
	/** 추적 도착 허용 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float acceptanceRadius = 50.f;

private:
	/** 현재 상태 */
	ECDMonsterState state = ECDMonsterState::Idle;
};
