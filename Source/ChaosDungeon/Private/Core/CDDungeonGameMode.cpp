#include "Core/CDDungeonGameMode.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Core/CDRunSubsystem.h"
#include "Data/CDStageData.h"
#include "Player/CDPlayerController.h"
#include "World/CDSpawnManagerComponent.h"
#include "World/CDDoorSpawnPoint.h"
#include "World/CDInteractable.h"
#include "World/CDSpawnPoint.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UI/CDUIManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"


ACDDungeonGameMode::ACDDungeonGameMode()
{
	PlayerControllerClass = ACDPlayerController::StaticClass();
	spawnManager = CreateDefaultSubobject<UCDSpawnManagerComponent>(TEXT("SpawnManager"));
}

void ACDDungeonGameMode::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	UCDUIManagerSubsystem::Get(playerController)->ShowGameHud();

	UCDMessageSubsystem& messages = UCDMessageSubsystem::Get(this);
	listenerHandles.Add(messages.Listen(CDTags::Msg_Combat_Damage, this, &ThisClass::HandleDamage));
	listenerHandles.Add(messages.Listen(CDTags::Msg_Combat_Death, this, &ThisClass::HandleDeath));
	listenerHandles.Add(messages.Listen(CDTags::Msg_Stage_PortalEntered, this, &ThisClass::HandlePortalEntered));

	if (GetRun()->BeginTestRunForWorld(GetWorld()) == false)
	{
		return;
	}

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
	const UCDStageData* stage = GetRun()->GetCurrentStage();

	stageStartTime = GetWorld()->GetTimeSeconds();
	BroadcastStage(CDTags::Msg_Stage_Started);

	/* 현재 실행중인 게임 타입이 미닉 게임이면 스타트 - yoonsang */
	if (stage->gameModeType == ECDGameModeType::MiniGameMode)
	{
		StartBombGame();
	}
	else
	{
		spawnManager->StartSpawning();
	}
}

void ACDDungeonGameMode::ClearStage()
{
	if (bIsBombGameActive)
	{
		StopBombGame();
	}

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

	for (TActorIterator<ACDDoorSpawnPoint> it(GetWorld()); it; ++it)
	{
		ACDInteractable* door = GetWorld()->SpawnActor<ACDInteractable>(
			exitDoorClass,
			it->GetActorLocation(),
			it->GetActorRotation());

		if (door)
		{
			door->SetPortalType(ECDPortalType::NextStage);
		}

		break;
	}
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
	if (const UCDStageData* stage = GetRun()->GetCurrentStage())
	{
		message.stageName = stage->displayName;
	}
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

void ACDDungeonGameMode::StartBombGame()
{
	const UCDStageData* stage = GetRun()->GetCurrentStage();

	if (bIsBombGameActive)
	{
		return;
	}

	bIsBombGameActive = true;

	GetWorldTimerManager().SetTimer(
        bombSpawnTimer,
        this,
        &ThisClass::SpawnBomb,
        stage->bombSpawnInterval,
        true);

    GetWorldTimerManager().SetTimer(
        bombCompleteTimer,
        this,
        &ThisClass::CompleteBombGame,
        static_cast<float>(stage->durationTime),
        false);
}

void ACDDungeonGameMode::SpawnBomb()
{
	if (!bIsBombGameActive)
	{
		return;
	}

	const UCDStageData* stage = GetRun()->GetCurrentStage();
	APawn* player = UGameplayStatics::GetPlayerPawn(this, 0);

	FVector spawnLocation = player->GetActorLocation();
	spawnLocation.Z += 1000.0f;

	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* bomb = GetWorld()->SpawnActor<AActor>(
		stage->bombClass,
		spawnLocation,
		FRotator::ZeroRotator,
		spawnParameters);

	if (IsValid(bomb))
	{
		spawnedBombs.Add(bomb);
	}
}

void ACDDungeonGameMode::CompleteBombGame()
{
	ClearStage();
}

void ACDDungeonGameMode::StopBombGame()
{
	bIsBombGameActive = false;


	GetWorldTimerManager().ClearTimer(bombSpawnTimer);
	GetWorldTimerManager().ClearTimer(bombCompleteTimer);

	for (AActor* bomb : spawnedBombs)
	{
		if (IsValid(bomb))
		{
			bomb->Destroy();
		}
	}

	spawnedBombs.Empty();
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
