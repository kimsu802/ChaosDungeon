#pragma once

#include "Character/CDCharacterBase.h"
#include "Core/CDTypes.h"
#include "GameplayTagContainer.h"
#include "CDMonsterBase.generated.h"

class ACDMonsterBase;
class UCDMonsterData;

DECLARE_MULTICAST_DELEGATE_OneParam(FCDMonsterEvent, ACDMonsterBase*);

/**
 * 모든 몬스터(일반/정예/보스)의 단일 클래스. 차이는 UCDMonsterData 로만 표현한다.
 * 오브젝트 풀링 전제: Destroy 대신 ActivateFromPool / DeactivateToPool
 * AI 컨트롤러 클래스는 BP 에서 지정한다 (몬스터는 AI 클래스를 모름, AI → 몬스터 단방향)
 */
UCLASS()
class CHAOSDUNGEON_API ACDMonsterBase : public ACDCharacterBase
{
	GENERATED_BODY()

public:
	/** 풀링/스텐실 기본 설정 */
	ACDMonsterBase();

	/** 풀에서 꺼내 데이터/난이도로 초기화 */
	void ActivateFromPool(const UCDMonsterData* data, const FCDDifficultySettings& difficulty, const FVector& location);

	/** 사망 연출 후 자동 호출. onReleased 로 스폰 매니저에 반환 */
	void DeactivateToPool();

	/** 몬스터 데이터 */
	FORCEINLINE const UCDMonsterData* GetMonsterData() const
	{
		return monsterData;
	}

	// ACDCharacterBase::GetAimLocation()
	virtual FVector GetAimLocation() const override;

protected:
	// ACDCharacterBase::GetKillContribution()
	virtual float GetKillContribution() const override;

	// ACDCharacterBase::HandleDeath()
	virtual void HandleDeath() override;

private:
	/** 풀 활성/비활성 (표시, 충돌, 틱, 컨트롤러) */
	void SetPoolActive(bool bActive);

	/** 드랍 테이블에 따라 액터 생성 */
	void SpawnDrops() const;

	/** 경직/넉백 규칙을 한 곳에서 결정: 일반 = 모두 적용, 정예 = 넉백 면역, 보스 = 모두 면역 */
	static FGameplayTagContainer GetImmunityTags(ECDMonsterGrade grade);

public:
	/** 풀로 반환될 때 */
	FCDMonsterEvent onReleased;

protected:
	/** 사망 후 풀로 돌아가기까지 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Pool")
	float corpseLifetime = 2.f;

private:
	/** 현재 몬스터 데이터 */
	UPROPERTY(Transient)
	TObjectPtr<const UCDMonsterData> monsterData;

	/** 풀 반환 타이머 */
	FTimerHandle releaseTimer;
};
