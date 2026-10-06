#include "Core/CDRunSubsystem.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDLevelTransitionSubsystem.h"
#include "Core/CDMessages.h"
#include "Core/CDMessageSubsystem.h"
#include "Data/CDDungeonData.h"
#include "Data/CDStageData.h"
#include "Settings/CDDeveloperSettings.h"

void UCDRunSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);
	collection.InitializeDependency<UCDLevelTransitionSubsystem>();
	collection.InitializeDependency<UCDMessageSubsystem>();

	//dungeonData = dungeonDataAsset.LoadSynchronous();
	//ensureMsgf(dungeonData, TEXT("DefaultGame.ini 에 dungeonDataAsset 을 지정하세요."));

	const UCDDeveloperSettings* developerSettings = GetDefault<UCDDeveloperSettings>();
	dungeonData = developerSettings->dungeonData.LoadSynchronous();
}

void UCDRunSubsystem::StartRun(ECDDifficulty difficulty)
{
	// 던전 설정과 스테이지 목록을 확인한다.
	if (!dungeonData || dungeonData->stages.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Dungeon Data or Stages is missing."));
		return;
	}

	// 모든 스테이지에 목적지 맵이 지정되어 있어야 한다.
	for (const TObjectPtr<UCDStageData>& stage : dungeonData->stages)
	{
		if (!stage || stage->level.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("Stage Data or Level is missing."));
			return;
		}
	}

	// 원본 데이터는 유지하고 이번 런에서 사용할 목록만 복사한다.
	runStages = dungeonData->stages;

	// 뒤에서부터 임의의 항목과 교환하여 방문 순서를 섞는다.
	for (int32 i = runStages.Num() - 1; i > 0; --i)
	{
		const int32 randomIndex = FMath::RandRange(0, i);
		runStages.Swap(i, randomIndex);
	}

	// 새 런의 기록과 현재 위치를 초기화한다.
	record = FCDRunRecord();
	record.difficulty = difficulty;
	stageIndex = 0;
	bRunFinished = false;
	bNewBestRecord = false;
	bHasPlayedRun = true;

	TravelToCurrentStage();
}

void UCDRunSubsystem::RetryRun()
{
	StartRun(record.difficulty);
}

void UCDRunSubsystem::ReturnToHub()
{
	GetGameInstance()->GetSubsystem<UCDLevelTransitionSubsystem>()->TravelTo(dungeonData->hubLevel);
}

void UCDRunSubsystem::AdvanceStage()
{
	if (bRunFinished || !runStages.IsValidIndex(stageIndex + 1))
	{
		return;
	}

	++stageIndex;
	TravelToCurrentStage();
}

void UCDRunSubsystem::FinishRun(bool bSuccess)
{
	if (bRunFinished)
	{
		return;
	}
	bRunFinished = true;
	record.bSuccess = bSuccess;
	UpdateBestRecord();
	BroadcastRunMessage(CDTags::Msg_Run_Finished);
}

bool UCDRunSubsystem::RegisterDeath()
{
	++record.deaths;
	BroadcastRunMessage(CDTags::Msg_Run_DeathCount);

	const int32 maxDeaths = GetDifficultySettings().maxDeaths;
	return maxDeaths < 0 || record.deaths <= maxDeaths;
}

const UCDStageData* UCDRunSubsystem::GetCurrentStage() const
{
	if (!runStages.IsValidIndex(stageIndex))
	{
		return nullptr;
	}

	return runStages[stageIndex];
}

bool UCDRunSubsystem::IsLastStage() const
{
	return runStages.IsValidIndex(stageIndex) && stageIndex == runStages.Num() - 1;
}

const FCDDifficultySettings& UCDRunSubsystem::GetDifficultySettings() const
{
	return dungeonData->GetDifficultySettings(record.difficulty);
}

int32 UCDRunSubsystem::GetRemainingDeaths() const
{
	const int32 maxDeaths = GetDifficultySettings().maxDeaths;
	if (maxDeaths < 0)
	{
		return -1;
	}
	return FMath::Max(0, maxDeaths - record.deaths);
}

void UCDRunSubsystem::TravelToCurrentStage() const
{
	if (const UCDStageData* stage = GetCurrentStage())
	{
		GetGameInstance()->GetSubsystem<UCDLevelTransitionSubsystem>()->TravelTo(stage->level);
	}
}

void UCDRunSubsystem::UpdateBestRecord()
{
	// 더 많은 스테이지를 깼거나, 같은 스테이지를 더 빨리 깼으면 갱신
	const FCDRunRecord* best = bestRecords.Find(record.difficulty);
	bNewBestRecord = !best
		|| record.clearedStages > best->clearedStages
		|| (record.clearedStages == best->clearedStages && record.elapsedTime < best->elapsedTime);

	if (bNewBestRecord)
	{
		bestRecords.Add(record.difficulty, record);
	}
}

void UCDRunSubsystem::BroadcastRunMessage(FGameplayTag channel) const
{
	FCDRunMessage message;
	message.record = record;
	message.remainingDeaths = GetRemainingDeaths();
	message.bNewRecord = bNewBestRecord;
	GetGameInstance()->GetSubsystem<UCDMessageSubsystem>()->Broadcast(channel, message);
}
