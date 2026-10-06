#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Core/CDTypes.h"
#include "CDRunSubsystem.generated.h"

class UCDDungeonData;
class UCDStageData;

/**
 * 한 번의 던전 도전(런)을 관리한다.
 * 스테이지마다 레벨이 바뀌므로, 레벨이 바뀌어도 유지되어야 하는 데이터(난이도, 현재 스테이지, 기록)만 가진다.
 * 스테이지 안의 규칙은 ACDDungeonGameMode 가 담당한다.
 * 상태 변화는 메시지(Run.DeathCount, Run.Finished)로만 알린다. → UI 를 모름
 *
 * DefaultGame.ini
 *   [/Script/ChaosDungeon.CDRunSubsystem]
 *   dungeonDataAsset=/Game/Data/DA_Dungeon.DA_Dungeon
 */
UCLASS(Config = Game)
class CHAOSDUNGEON_API UCDRunSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem::Initialize()
	virtual void Initialize(FSubsystemCollectionBase& collection) override;

	/** 새 런 시작 (기록 초기화 후 1스테이지로 이동) */
	UFUNCTION(BlueprintCallable)
	void StartRun(ECDDifficulty difficulty);

	/** 같은 난이도로 재도전 */
	UFUNCTION(BlueprintCallable)
	void RetryRun();

	/** 허브로 돌아가기 */
	UFUNCTION(BlueprintCallable)
	void ReturnToHub();

	/** 다음 스테이지로 이동 */
	void AdvanceStage();

	/** 런 종료 (최고 기록 갱신 + Run.Finished 발행) */
	void FinishRun(bool bSuccess);

	/** 사망 1회 기록. 데스카운트를 초과하면 false */
	bool RegisterDeath();

	/** 기록 수정용 (GameMode 가 집계) */
	FORCEINLINE FCDRunRecord& GetMutableRecord()
	{
		return record;
	}

	/** 현재 스테이지 데이터 */
	const UCDStageData* GetCurrentStage() const;

	/** 현재 스테이지 인덱스 (0부터) */
	FORCEINLINE int32 GetStageIndex() const
	{
		return stageIndex;
	}

	/** 마지막 스테이지인가 */
	bool IsLastStage() const;

	/** 현재 난이도 설정 */
	const FCDDifficultySettings& GetDifficultySettings() const;

	/** 현재 런 기록 */
	UFUNCTION(BlueprintPure)
	FORCEINLINE FCDRunRecord GetRecord() const
	{
		return record;
	}

	/** 남은 데스카운트. -1 = 무제한 */
	UFUNCTION(BlueprintPure)
	int32 GetRemainingDeaths() const;

	/** 이번 런에서 최고 기록을 갱신했는가 */
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsNewBestRecord() const
	{
		return bNewBestRecord;
	}

	/** 한 번이라도 런을 시작했는가 (허브 복귀 시 타이틀 생략용) */
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool HasPlayedRun() const
	{
		return bHasPlayedRun;
	}

	/** 난이도별 최고 기록 (없으면 null) */
	FORCEINLINE const FCDRunRecord* FindBestRecord(ECDDifficulty difficulty) const
	{
		return bestRecords.Find(difficulty);
	}

private:
	/** 현재 스테이지 레벨로 이동 */
	void TravelToCurrentStage() const;

	/** 최고 기록 비교/갱신 */
	void UpdateBestRecord();

	/** Run.* 메시지 발행 */
	void BroadcastRunMessage(FGameplayTag channel) const;

private:
	/** 던전 구성 에셋 경로 (ini) */
	UPROPERTY(Config)
	TSoftObjectPtr<UCDDungeonData> dungeonDataAsset;

	/** 로드된 던전 구성 */
	UPROPERTY(Transient)
	TObjectPtr<UCDDungeonData> dungeonData;

	// 이번 런에서 방문할 스테이지를 무작위 순서로 보관한다.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCDStageData>> runStages;

	/** 현재 런 기록 */
	FCDRunRecord record;

	/** 난이도별 최고 기록 (저장은 추후 USaveGame) */
	TMap<ECDDifficulty, FCDRunRecord> bestRecords;

	/** 현재 스테이지 인덱스 */
	int32 stageIndex = 0;

	/** FinishRun 중복 호출 방지 */
	bool bRunFinished = false;

	/** 최고 기록 갱신 여부 */
	bool bNewBestRecord = false;

	/** 런을 시작한 적이 있는가 */
	bool bHasPlayedRun = false;
};
