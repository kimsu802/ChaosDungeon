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
#include "Engine/TargetPoint.h"
#include "World/CDMiniGameBomb.h"


ACDDungeonGameMode::ACDDungeonGameMode()
{
	PlayerControllerClass = ACDPlayerController::StaticClass();
	spawnManager = CreateDefaultSubobject<UCDSpawnManagerComponent>(TEXT("SpawnManager"));
}

void ACDDungeonGameMode::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	if (playerController)
	{
		if (ACDCharacterBase* character = Cast<ACDCharacterBase>(playerController->GetPawn()))
		{
			character->SetOutlineEnabled(true);
		}

		UCDUIManagerSubsystem::Get(playerController)->ShowGameHud();
	}


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

	/* 기존 방향의 변경으로 미니 게임에 몬스터도 스폰되도록 조정 - yoonsang */
	spawnManager->StartSpawning();
	
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
	StopBombGame();

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
	const UCDStageData* stage = GetRun()->GetCurrentStage();

	if (!message.bIsPlayer)
	{
		++GetRun()->GetMutableRecord().kills;
		if (bStageCleared)
		{
			return;
		}

		if (stage->gameModeType == ECDGameModeType::DungeonGameMode)
		{
			progress += message.contribution;
		}

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

	ATargetPoint* centerPoint = nullptr;
	ATargetPoint* upperLeftPoint = nullptr;
	ATargetPoint* upperRightPoint = nullptr;
	ATargetPoint* lowerLeftPoint = nullptr;
	ATargetPoint* lowerRightPoint = nullptr;

	/* TargetPoint 찾기 */
	for (TActorIterator<ATargetPoint> it(GetWorld()); it; ++it)
	{
		if (it->ActorHasTag(FName(TEXT("BombPatternCenter"))))
		{
			centerPoint = *it;
		}

		if (it->ActorHasTag(FName(TEXT("BombUpperLeft"))))
		{
			upperLeftPoint = *it;
		}

		if (it->ActorHasTag(FName(TEXT("BombUpperRight"))))
		{
			upperRightPoint = *it;
		}

		if (it->ActorHasTag(FName(TEXT("BombLowerLeft"))))
		{
			lowerLeftPoint = *it;
		}

		if (it->ActorHasTag(FName(TEXT("BombLowerRight"))))
		{
			lowerRightPoint = *it;
		}
	}

	if (!centerPoint || !upperLeftPoint || !upperRightPoint || !lowerLeftPoint || !lowerRightPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("1 of 5 not found TargetPoint"));

		return;
	}

	bombPatternCenter = centerPoint->GetActorLocation();
	bombUpeerLeft = upperLeftPoint->GetActorLocation();
	bombUpeerRight = upperRightPoint->GetActorLocation();
	bombLowerLeft = lowerLeftPoint->GetActorLocation();
	bombLowerRight = lowerRightPoint->GetActorLocation();

	bombPatternIndex = 0;

	bIsBombGameActive = true;

	progress = 0.f;

	targetProgress = static_cast<float>(stage->durationTime);

	BroadcastStage(CDTags::Msg_Stage_Progress);

	GetWorldTimerManager().SetTimer(bombSpawnTimer, this, &ThisClass::SpawnBomb, stage->bombSpawnInterval, true);

	GetWorldTimerManager().SetTimer(bombCompleteTimer, this, &ThisClass::CompleteBombGame, targetProgress, false);

	GetWorldTimerManager().SetTimer(bombProgressTimer, this, &ThisClass::UpdateBombProgress, 0.1f, true);
}

void ACDDungeonGameMode::SpawnBomb()
{
	if (!bIsBombGameActive)
	{
		return;
	}

	const FVector upperLeftDirection = (bombPatternCenter - bombUpeerLeft).GetSafeNormal();

	const FVector upperRightDirection = (bombPatternCenter - bombUpeerRight).GetSafeNormal();

	const FVector lowerLeftDirection = (bombPatternCenter - bombLowerLeft).GetSafeNormal();

	const FVector lowerRightDirection = (bombPatternCenter - bombLowerRight).GetSafeNormal();

	switch (bombPatternIndex)
	{
		case 0:
		{
			SpawnPatternBomb(bombPatternCenter, FVector::ForwardVector, false);
		
			break;
		}

		case 1:
		{
			SpawnPatternBomb(bombUpeerLeft,upperLeftDirection, true);
			SpawnPatternBomb(bombUpeerRight, upperRightDirection, true);
			SpawnPatternBomb(bombLowerLeft, lowerLeftDirection, true);
			SpawnPatternBomb(bombLowerRight, lowerRightDirection, true);

			break;
		}
		
		case 2:
		{
			SpawnPatternBomb(bombUpeerLeft, upperLeftDirection, true);
		
			break;
		}

		case 3:
		{
			SpawnPatternBomb(bombUpeerRight, upperRightDirection, true);
		 
			break;
		}

		case 4:
		{
			SpawnPatternBomb(bombLowerRight, lowerRightDirection, true);

			break;
		}
		
		case 5:
		{
			SpawnPatternBomb(bombLowerLeft, lowerLeftDirection, true);

			break;
		}
	}

	bombPatternIndex = (bombPatternIndex + 1) % 6;
}

void ACDDungeonGameMode::SpawnPatternBomb(const FVector& origin, const FVector& direction, bool bIsCone)
{
	const UCDStageData* stage = GetRun()->GetCurrentStage();

	const float radius = bIsCone ? coneBombRadius : centerBombRadius;

	FVector spawnLocation = origin;

	if (bIsCone)
	{
		const float landingDistance = FMath::Clamp(bombLandingDistance, 0.f, radius * 0.8f);

		spawnLocation += direction * landingDistance;
	}

	spawnLocation.Z += bombDropHeight;

	const FTransform spawnTransform(FRotator::ZeroRotator, spawnLocation);

	ACDMinigameBomb* bomb = GetWorld()->SpawnActorDeferred<ACDMinigameBomb>(
		stage->bombClass,
		spawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!IsValid(bomb))
	{
		return;
	}

	bomb->InitializePattern(origin, direction, radius, bIsCone);

	bomb->FinishSpawning(spawnTransform);

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
	GetWorldTimerManager().ClearTimer(bombProgressTimer);

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

void ACDDungeonGameMode::UpdateBombProgress()
{
	if (!bIsBombGameActive)
	{
		return;
	}

	const float elapsedTime = GetWorldTimerManager().GetTimerElapsed(bombCompleteTimer);

	progress = FMath::Clamp(elapsedTime, 0.0f, targetProgress);

	BroadcastStage(CDTags::Msg_Stage_Progress);
}
