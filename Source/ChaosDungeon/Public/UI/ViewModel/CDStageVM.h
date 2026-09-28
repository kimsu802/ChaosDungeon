#pragma once

#include "MVVMViewModelBase.h"
#include "Core/CDMessageSubsystem.h"
#include "CDStageVM.generated.h"

struct FCDStageMessage;
struct FCDRunMessage;
struct FCDActorMessage;

/** 좌상단 스테이지 정보 + 보스 HP 바 표시 여부. 값은 전부 메시지로만 받는다 */
UCLASS()
class CHAOSDUNGEON_API UCDStageVM : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** 메시지 구독 시작 */
	void StartListening(UCDMessageSubsystem& messages);

	/** 메시지 구독 해제 */
	void StopListening(UCDMessageSubsystem& messages);

	/** 스테이지 진행 중인가 (허브에서는 false) */
	bool GetIsInStage() const
	{
		return bIsInStage;
	}

	/** 스테이지 번호 */
	int32 GetStageNumber() const
	{
		return stageNumber;
	}

	/** 처치 게이지 (0 ~ 1) */
	float GetProgress() const
	{
		return progress;
	}

	/** 남은 데스카운트. -1 = 무제한 */
	int32 GetRemainingDeaths() const
	{
		return remainingDeaths;
	}

	/** 보스 HP 바 표시 여부 */
	bool GetIsBossVisible() const
	{
		return bIsBossVisible;
	}

private:
	/** Stage.Started */
	void HandleStageStarted(const FCDStageMessage& message);

	/** Stage.Progress */
	void HandleStageProgress(const FCDStageMessage& message);

	/** Run.DeathCount */
	void HandleDeathCount(const FCDRunMessage& message);

	/** Combat.BossAppeared */
	void HandleBossAppeared(const FCDActorMessage& message);

private:
	/** 스테이지 진행 중 (허브에서는 false → 스테이지 정보 숨김) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIsInStage, meta = (AllowPrivateAccess = true))
	bool bIsInStage = false;

	/** 스테이지 번호 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetStageNumber, meta = (AllowPrivateAccess = true))
	int32 stageNumber = 0;

	/** 처치 게이지 (0 ~ 1) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetProgress, meta = (AllowPrivateAccess = true))
	float progress = 0.f;

	/** 남은 데스카운트. -1 = 무제한 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetRemainingDeaths, meta = (AllowPrivateAccess = true))
	int32 remainingDeaths = -1;

	/** 보스 HP 바 표시 여부 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIsBossVisible, meta = (AllowPrivateAccess = true))
	bool bIsBossVisible = false;

	/** 메시지 구독 핸들 */
	TArray<FCDListenerHandle> handles;
};
