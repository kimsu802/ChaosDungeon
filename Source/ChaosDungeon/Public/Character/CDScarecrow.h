#pragma once

#include "Character/CDMonsterBase.h"
#include "Engine/TimerHandle.h"
#include "CDScarecrow.generated.h"

class UCDMonsterData;
class UCDSpeechBubbleComponent;

/**
 * 스킬 · 타격 테스트용 허수아비 (Entrance 레벨 등에 직접 배치).
 * - 몬스터 데이터는 원래 스폰 매니저가 ActivateFromPool 로 넣어 준다 → 배치된 허수아비는 BeginPlay 에서 스스로 호출
 * - 죽지 않는다: 체력이 0 이 되면 즉시 가득 채움, 마지막 피격 후 healthResetDelay 가 지나도 가득 채움
 * - AI 없음 (움직이거나 공격하지 않음), 넉백 면역(옵션)
 * - 머리 위 대사: 가끔 랜덤 대사, 맞으면 피격 대사 (쿨다운)
 */
UCLASS()
class CHAOSDUNGEON_API ACDScarecrow : public ACDMonsterBase
{
	GENERATED_BODY()

public:
	/** AI 끄기 + 대사창 생성 + 기본 대사 */
	ACDScarecrow();

protected:
	// ACDMonsterBase::BeginPlay()
	virtual void BeginPlay() override;

	// AActor::EndPlay()
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

	// ACDCharacterBase::HandleDamageTaken()
	virtual void HandleDamageTaken(AActor* damageInstigator, float damage, bool bCritical) override;

	// ACDMonsterBase::HandleDeath() ← ACDCharacterBase::HandleDeath()
	virtual void HandleDeath() override;

private:
	/** 체력 가득 채우기 */
	void RefillHealth();

	/** 다음 랜덤 대사 예약 */
	void ScheduleIdleLine();

	/** 랜덤 대사 1회 (타이머) */
	void HandleIdleLine();

	/** 대사 표시 → lineDuration 뒤 내림 */
	void SayLine(const FText& text);

	/* 대사 표시가 가능한 지*/
	bool CanSpeak() const;

	/** 목록에서 무작위 1개 (직전과 다른 것 우선) */
	static const FText* PickLine(const TArray<FText>& lines, int32& lastIndex);

protected:
	/** 허수아비 스탯 (maxHealth 를 아주 크게, contribution 0, attackSkills 비움) */
	UPROPERTY(EditAnywhere, Category = "Scarecrow")
	TObjectPtr<UCDMonsterData> scarecrowData;

	/** 경직(피격 모션) 면역 */
	UPROPERTY(EditAnywhere, Category = "Scarecrow")
	bool bImmuneToStagger = false;

	/** 넉백 면역 (밀려나면 테스트하기 불편) */
	UPROPERTY(EditAnywhere, Category = "Scarecrow")
	bool bImmuneToKnockback = true;

	/** 마지막 피격 후 이 시간이 지나면 체력 회복 (초) */
	UPROPERTY(EditAnywhere, Category = "Scarecrow", meta = (ClampMin = 0.1))
	float healthResetDelay = 3.f;

	/** 머리 위 대사창 (Widget Class = WBP_SpeechBubble, 위치는 BP 에서 조절) */
	UPROPERTY(VisibleAnywhere, Category = "Scarecrow|Speech")
	TObjectPtr<UCDSpeechBubbleComponent> speechBubble;

	/** 가끔 혼자 하는 대사 */
	UPROPERTY(EditAnywhere, Category = "Scarecrow|Speech")
	TArray<FText> idleLines;

	/** 맞았을 때 대사 */
	UPROPERTY(EditAnywhere, Category = "Scarecrow|Speech")
	TArray<FText> hitLines;

	/** 랜덤 대사 간격 최소 (초) */
	UPROPERTY(EditAnywhere, Category = "Scarecrow|Speech", meta = (ClampMin = 0.5))
	float idleIntervalMin = 6.f;

	/** 랜덤 대사 간격 최대 (초) */
	UPROPERTY(EditAnywhere, Category = "Scarecrow|Speech", meta = (ClampMin = 0.5))
	float idleIntervalMax = 10.f;

	/** 대사가 떠 있는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Scarecrow|Speech", meta = (ClampMin = 0.5))
	float lineDuration = 2.5f;

	/** 피격 대사 최소 간격 (연타에 매번 말하지 않도록, 초) */
	UPROPERTY(EditAnywhere, Category = "Scarecrow|Speech", meta = (ClampMin = 0))
	float hitLineCooldown = 4.f;

private:
	/** 체력 회복 타이머 */
	FTimerHandle healthResetTimer;

	/** 랜덤 대사 타이머 */
	FTimerHandle idleLineTimer;

	/** 대사 내림 타이머 */
	FTimerHandle hideLineTimer;

	/** 마지막 피격 대사 시각 (월드 시간) */
	float lastHitLineTime = -1000.f;

	/** 직전 랜덤 대사 번호 */
	int32 lastIdleIndex = INDEX_NONE;

	/** 직전 피격 대사 번호 */
	int32 lastHitIndex = INDEX_NONE;
};
