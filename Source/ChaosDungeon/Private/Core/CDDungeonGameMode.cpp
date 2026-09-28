#include "Core/CDDungeonGameMode.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Core/CDRunSubsystem.h"
#include "Data/CDStageData.h"
#include "Player/CDPlayerController.h"
#include "World/CDSpawnManagerComponent.h"
#include "World/CDSpawnPoint.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ACDDungeonGameMode::ACDDungeonGameMode()
{
	PlayerControllerClass = ACDPlayerController::StaticClass();
	spawnManager = CreateDefaultSubobject<UCDSpawnManagerComponent>(TEXT("SpawnManager"));
}

void ACDDungeonGameMode::BeginPlay()
{
	Super::BeginPlay();

	UCDMessageSubsystem& messages = UCDMessageSubsystem::Get(this);
	listenerHandles.Add(messages.Listen(CDTags::Msg_Combat_Damage, this, &ThisClass::HandleDamage));
	listenerHandles.Add(messages.Listen(CDTags::Msg_Combat_Death, this, &ThisClass::HandleDeath));
	listenerHandles.Add(messages.Listen(CDTags::Msg_Stage_PortalEntered, this, &ThisClass::HandlePortalEntered));

	const UCDStageData* stage = GetRun()->GetCurrentStage();
	targetProgress = stage->targetProgress;
	spawnManager->Initialize(GatherSpawnPoints(), stage, GetRun()->GetDifficultySettings());

	FTimerHandle introTimer;
	GetWorldTimerManager().SetTimer(introTimer, this, &ThisClass::BeginStage, stageIntroDelay, false);
}

void ACDDungeonGameMode::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	UCDMessageSubsystem& messages = UCDMessageSubsystem::Get(this);
	for (FCDListenerHandle& handle : listenerHandles)
	{
		messages.Unlisten(handle);
	}
	Super::EndPlay(endPlayReason);
}

void ACDDungeonGameMode::AbandonRun()
{
	FailRun();
}

void ACDDungeonGameMode::BeginStage()
{
	stageStartTime = GetWorld()->GetTimeSeconds();
	BroadcastStage(CDTags::Msg_Stage_Started);
	spawnManager->StartSpawning();
}

void ACDDungeonGameMode::ClearStage()
{
	bStageCleared = true;
	spawnManager->StopSpawning();
	CommitStageTime();
	++GetRun()->GetMutableRecord().clearedStages;

	if (GetRun()->IsLastStage())
	{
		GetRun()->FinishRun(true);
		return;
	}

	// 포탈이 구독해서 열림
	BroadcastStage(CDTags::Msg_Stage_Cleared);
}

void ACDDungeonGameMode::FailRun()
{
	spawnManager->StopSpawning();
	CommitStageTime();
	GetRun()->FinishRun(false);
}

void ACDDungeonGameMode::CommitStageTime()
{
	const float now = GetWorld()->GetTimeSeconds();
	GetRun()->GetMutableRecord().elapsedTime += now - stageStartTime;
	stageStartTime = now;
}

void ACDDungeonGameMode::BroadcastStage(FGameplayTag channel) const
{
	FCDStageMessage message;
	message.stageNumber = GetRun()->GetStageIndex() + 1;
	message.progress = targetProgress > 0.f ? FMath::Min(progress / targetProgress, 1.f) : 0.f;
	message.remainingDeaths = GetRun()->GetRemainingDeaths();
	UCDMessageSubsystem::Get(this).Broadcast(channel, message);
}

void ACDDungeonGameMode::HandleDamage(const FCDDamageMessage& message)
{
	FCDRunRecord& record = GetRun()->GetMutableRecord();
	if (message.bTargetIsPlayer)
	{
		record.damageTaken += message.amount;
	}
	else
	{
		record.damageDealt += message.amount;
	}
}

void ACDDungeonGameMode::HandleDeath(const FCDDeathMessage& message)
{
	if (!message.bIsPlayer)
	{
		++GetRun()->GetMutableRecord().kills;
		if (bStageCleared)
		{
			return;
		}
		progress += message.contribution;
		BroadcastStage(CDTags::Msg_Stage_Progress);
		if (progress >= targetProgress)
		{
			ClearStage();
		}
		return;
	}

	if (!GetRun()->RegisterDeath())
	{
		FailRun();
		return;
	}

	// 제자리 부활
	TWeakObjectPtr<ACDCharacterBase> player = Cast<ACDCharacterBase>(message.victim);
	auto RevivePlayer = [this, player]()
	{
		if (player.IsValid())
		{
			player->Revive(reviveInvincibleTime);
		}
	};
	FTimerHandle reviveTimer;
	GetWorldTimerManager().SetTimer(reviveTimer, FTimerDelegate::CreateWeakLambda(this, RevivePlayer), reviveDelay, false);
}

void ACDDungeonGameMode::HandlePortalEntered(const FCDStageMessage& message)
{
	GetRun()->AdvanceStage();
}

TArray<FVector> ACDDungeonGameMode::GatherSpawnPoints() const
{
	TArray<FVector> points;
	for (TActorIterator<ACDSpawnPoint> it(GetWorld()); it; ++it)
	{
		points.Add(it->GetActorLocation());
	}
	return points;
}

UCDRunSubsystem* ACDDungeonGameMode::GetRun() const
{
	return GetGameInstance()->GetSubsystem<UCDRunSubsystem>();
}
