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
	// 던전 레벨 확인
	if (dungeonData == nullptr || dungeonData->stages.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Dungeon Data or Stages is missing."));
		return;
	}

	// 보스 레벨 확인
	if (dungeonData->bossStage == nullptr || dungeonData->bossStage->level.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("Boss Stage or Level is missing"));
		return;
	}

	TSet<FSoftObjectPath> registeredLevels;

	for (const TObjectPtr<UCDStageData>& stage : dungeonData->stages)
	{
		if (stage == nullptr || stage->level.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("Stage Data or Level is missing"));
			return;
		}

		const FSoftObjectPath levelPath = stage->level.ToSoftObjectPath();

		if (registeredLevels.Contains(levelPath)
			|| levelPath == dungeonData->bossStage->level.ToSoftObjectPath())
		{
			UE_LOG(LogTemp, Error, TEXT("Duplicate stage level is registered."));
			return;
		}

		registeredLevels.Add(levelPath);
	}

	UCDLevelTransitionSubsystem* transition = GetGameInstance()->GetSubsystem<UCDLevelTransitionSubsystem>();

	if (transition == nullptr)
	{
		return;
	}

	TArray<TObjectPtr<UCDStageData>> nextStages = dungeonData->stages;

	// 뒤에서부터 임의의 항목과 교환하여 방문 순서를 섞는다.
	//for (int32 i = nextStages.Num() - 1; i > 0; --i)
	//{
	//	const int32 randomIndex = FMath::RandRange(0, i);
	//	nextStages.Swap(i, randomIndex);
	//}

	nextStages.Add(dungeonData->bossStage);

	const TSoftObjectPtr<UWorld> firstLevel = nextStages[0]->level;

	const bool accepted = transition->TravelTo(firstLevel, [this, difficulty, nextStages = MoveTemp(nextStages)]() mutable
		{
			runStages = MoveTemp(nextStages);

			record = FCDRunRecord();
			record.difficulty = difficulty;

			stageIndex = 0;
			bRunFinished = false;
			bNewBestRecord = false;
			bHasPlayedRun = true;
		});

	if (!accepted)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartRun travel request was rejected."));
	}

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

bool UCDRunSubsystem::BeginTestRunForWorld(const UWorld* world, ECDDifficulty difficulty)
{
	// 정상적으로 StartRun 을 거쳐 들어온 경우
	if (GetCurrentStage() != nullptr)
	{
		return true;
	}

	if (world == nullptr || dungeonData == nullptr)
	{
		return false;
	}

	// StartRun 과 같은 순서: 일반 스테이지 → 보스 스테이지
	TArray<TObjectPtr<UCDStageData>> stages = dungeonData->stages;
	if (dungeonData->bossStage)
	{
		stages.Add(dungeonData->bossStage);
	}

	// PIE 월드는 패키지 이름 앞에 "UEDPIE_0_" 가 붙으므로 떼고 비교
	const FString worldPackage = UWorld::RemovePIEPrefix(world->GetPackage()->GetName());
	const int32 foundIndex = stages.IndexOfByPredicate([&worldPackage](const UCDStageData* stage)
		{
			return stage && (stage->level.ToSoftObjectPath().GetLongPackageName() == worldPackage);
		});

	if (foundIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("[TestRun] %s 레벨이 DA_Dungeon 의 stages / bossStage 에 없습니다."), *worldPackage);
		return false;
	}

	runStages = MoveTemp(stages);
	record = FCDRunRecord();
	record.difficulty = difficulty;
	stageIndex = foundIndex;
	bRunFinished = false;
	bNewBestRecord = false;
	bHasPlayedRun = true;

	UE_LOG(LogTemp, Warning, TEXT("[TestRun] 런 없이 시작한 레벨 → 테스트 런 생성 (스테이지 %d / %d)"), foundIndex + 1, runStages.Num());
	return true;
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
