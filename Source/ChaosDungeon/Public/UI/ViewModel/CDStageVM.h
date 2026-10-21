#pragma once

#include "MVVMViewModelBase.h"
#include "Core/CDMessageSubsystem.h"
#include "CDStageVM.generated.h"

class AActor;
struct FCDStageMessage;
struct FCDRunMessage;
struct FCDActorMessage;
struct FCDDeathMessage;

/** 좌상단 스테이지 정보 + 우상단 지역명 + 보스 HP 바 표시 여부 · 이름 · 줄 수. 값은 전부 메시지로만 받는다 */
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

	/** 우측 상단 지역명 (스테이지 데이터의 displayName, 없으면 "스테이지 N") */
	FText GetStageName() const
	{
		return stageName;
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

	/** 보스 이름 */
	FText GetBossName() const
	{
		return bossName;
	}

	/** 보스 HP 줄 수 */
	int32 GetBossHpLineCount() const
	{
		return bossHpLineCount;
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

	/** Combat.Death → 현재 보스가 죽으면 보스 HP 바 숨김 */
	void HandleDeath(const FCDDeathMessage& message);

	/** 보스 이름 갱신 + 알림 (FText 는 UE_MVVM_SET_PROPERTY_VALUE 비교가 안 돼 직접 처리) */
	void SetBossName(const FText& value);

	/** 지역명 갱신 + 알림 (FText 라 직접 비교) */
	void SetStageName(const FText& value);

private:
	/** 스테이지 진행 중 (허브에서는 false → 스테이지 정보 숨김) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIsInStage, meta = (AllowPrivateAccess = true))
	bool bIsInStage = false;

	/** 스테이지 번호 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetStageNumber, meta = (AllowPrivateAccess = true))
	int32 stageNumber = 0;

	/** 우측 상단 지역명 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetStageName, meta = (AllowPrivateAccess = true))
	FText stageName;

	/** 처치 게이지 (0 ~ 1) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetProgress, meta = (AllowPrivateAccess = true))
	float progress = 0.f;

	/** 남은 데스카운트. -1 = 무제한 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetRemainingDeaths, meta = (AllowPrivateAccess = true))
	int32 remainingDeaths = -1;

	/** 보스 HP 바 표시 여부 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIsBossVisible, meta = (AllowPrivateAccess = true))
	bool bIsBossVisible = false;

	/** 보스 이름 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetBossName, meta = (AllowPrivateAccess = true))
	FText bossName;

	/** 보스 HP 줄 수 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetBossHpLineCount, meta = (AllowPrivateAccess = true))
	int32 bossHpLineCount = 1;

	/** 현재 보스 (사망 메시지가 이 보스의 것인지 확인용) */
	TWeakObjectPtr<AActor> currentBoss;

	/** 메시지 구독 핸들 */
	TArray<FCDListenerHandle> handles;
};
