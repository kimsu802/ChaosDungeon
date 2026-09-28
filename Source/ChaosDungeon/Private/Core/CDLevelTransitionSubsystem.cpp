#include "Core/CDLevelTransitionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "MoviePlayer.h"
#include "TimerManager.h"

void UCDLevelTransitionSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ThisClass::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

void UCDLevelTransitionSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	Super::Deinitialize();
}

void UCDLevelTransitionSubsystem::TravelTo(const TSoftObjectPtr<UWorld>& level)
{
	if (bTraveling || level.IsNull())
	{
		return;
	}
	bTraveling = true;
	pendingLevel = level;

	// 결과 화면 등에서 일시정지 상태일 수 있음
	UGameplayStatics::SetGamePaused(GetGameInstance(), false);
	StartFade(0.f, 1.f);

	FTimerHandle timerHandle;
	GetGameInstance()->GetTimerManager().SetTimer(timerHandle, this, &ThisClass::OpenPendingLevel, fadeDuration, false);
}

void UCDLevelTransitionSubsystem::OpenPendingLevel()
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetGameInstance(), pendingLevel);
}

void UCDLevelTransitionSubsystem::HandlePreLoadMap(const FString& mapName)
{
	if (IsRunningDedicatedServer() || loadingScreenClass.IsNull())
	{
		return;
	}
	loadingScreen = CreateWidget<UUserWidget>(GetGameInstance(), loadingScreenClass.LoadSynchronous());

	FLoadingScreenAttributes attributes;
	attributes.bAutoCompleteWhenLoadingCompletes = true;
	attributes.MinimumLoadingScreenDisplayTime = 0.5f;
	attributes.WidgetLoadingScreen = loadingScreen->TakeWidget();
	GetMoviePlayer()->SetupLoadingScreen(attributes);
}

void UCDLevelTransitionSubsystem::HandlePostLoadMap(UWorld* loadedWorld)
{
	loadingScreen = nullptr;
	if (bTraveling)
	{
		bTraveling = false;
		StartFade(1.f, 0.f);
	}
}

void UCDLevelTransitionSubsystem::StartFade(float from, float to) const
{
	if (APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController())
	{
		playerController->PlayerCameraManager->StartCameraFade(from, to, fadeDuration, FLinearColor::Black, false, true);
	}
}
