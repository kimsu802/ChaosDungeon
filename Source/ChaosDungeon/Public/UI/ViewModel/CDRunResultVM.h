#pragma once

#include "MVVMViewModelBase.h"
#include "Core/CDMessageSubsystem.h"
#include "Core/CDTypes.h"
#include "CDRunResultVM.generated.h"

struct FCDRunMessage;

/**
 * 결과 화면. Run.Finished 메시지 한 번으로 모든 값이 채워진다.
 * View 에서는 record.kills 처럼 구조체 멤버 경로로 바인딩
 */
UCLASS()
class CHAOSDUNGEON_API UCDRunResultVM : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** 메시지 구독 시작 */
	void StartListening(UCDMessageSubsystem& messages);

	/** 메시지 구독 해제 */
	void StopListening(UCDMessageSubsystem& messages);

	/** 런 기록 */
	FCDRunRecord GetRecord() const
	{
		return record;
	}

	/** 최고 기록 갱신 여부 */
	bool GetIsNewRecord() const
	{
		return bIsNewRecord;
	}

	/** 결과 화면 표시용 텍스트: 걸린 시간 "05:32" */
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetElapsedTimeText() const;

	/** 결과 화면 표시용 텍스트: 처치 수 "1,234" */
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetKillsText() const;

	/** 결과 화면 표시용 텍스트: 사망 수 "2" */
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetDeathsText() const;

	/** 결과 화면 표시용 텍스트: 준 피해 "1,204,880" */
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetDamageDealtText() const;

	/** 결과 화면 표시용 텍스트: 받은 피해 "8,412" */
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetDamageTakenText() const;

	/** 결과 화면 표시용 텍스트: 클리어 스테이지 "3" */
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetClearedStagesText() const;

	/** 결과 화면 표시용 텍스트: 난이도 이름 "보통" */
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetDifficultyText() const;

	/** 결과 화면 표시용: 난이도 색 (배지 틴트) */
	UFUNCTION(BlueprintPure, FieldNotify)
	FLinearColor GetDifficultyColor() const;

	/** 초 → "MM:SS" (난이도 카드의 최고 기록 표시에서도 사용) */
	static FText FormatRunTime(float seconds);

	/** 난이도 → 표시 이름 */
	static FText GetDifficultyDisplayName(ECDDifficulty difficulty);

	/** 난이도 → 대표 색 (쉬움 초록 · 보통 금 · 어려움 빨강) */
	static FLinearColor GetDifficultyAccentColor(ECDDifficulty difficulty);

private:
	/** Run.Finished */
	void HandleRunFinished(const FCDRunMessage& message);

private:
	/** 런 기록 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetRecord, meta = (AllowPrivateAccess = true))
	FCDRunRecord record;

	/** 최고 기록 갱신 여부 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIsNewRecord, meta = (AllowPrivateAccess = true))
	bool bIsNewRecord = false;

	/** Run.Finished 구독 핸들 */
	FCDListenerHandle finishedHandle;
};
