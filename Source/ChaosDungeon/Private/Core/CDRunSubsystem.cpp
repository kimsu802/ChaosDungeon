#include "Core/CDRunSubsystem.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDLevelTransitionSubsystem.h"
#include "Core/CDMessages.h"
#include "Core/CDMessageSubsystem.h"
#include "Data/CDDungeonData.h"
#include "Data/CDStageData.h"

void UCDRunSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);
	collection.InitializeDependency<UCDLevelTransitionSubsystem>();
	collection.InitializeDependency<UCDMessageSubsystem>();

	dungeonData = dungeonDataAsset.LoadSynchronous();
	ensureMsgf(dungeonData, TEXT("DefaultGame.ini 에 dungeonDataAsset 을 지정하세요."));
}

void UCDRunSubsystem::StartRun(ECDDifficulty difficulty)
{
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
	if (!dungeonData || !dungeonData->stages.IsValidIndex(stageIndex))
	{
		return nullptr;
	}
	return dungeonData->stages[stageIndex];
}

bool UCDRunSubsystem::IsLastStage() const
{
	return dungeonData && stageIndex >= dungeonData->stages.Num() - 1;
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
