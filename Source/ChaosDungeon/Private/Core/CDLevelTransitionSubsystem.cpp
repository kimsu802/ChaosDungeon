#include "Core/CDLevelTransitionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "MoviePlayer.h"
#include "Settings/CDDeveloperSettings.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

namespace
{
	// 로딩(블로킹) 직후 첫 틱은 델타가 매우 커서 페이드가 한 프레임에 끝나버리므로 상한을 둔다
	constexpr float maxFadeDeltaTime = 1.f / 30.f;
}

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
	if (fadeTickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(fadeTickHandle);
		fadeTickHandle.Reset();
	}
	fadeFinished = nullptr;
	HideOverlay();
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

	// 이미 FadeScreen 으로 어두워진 상태면 그 알파에서 이어서 진행
	ShowOverlay(currentAlpha);
	StartFade(1.f, [this]()
	{
		OpenPendingLevel();
	});
}

bool UCDLevelTransitionSubsystem::FadeScreen(float targetAlpha, TFunction<void()> onFinished)
{
	if (bTraveling)
	{
		return false;
	}

	// 다른 경로의 레벨 로드로 오버레이가 떨어졌을 수 있으므로 항상 다시 붙인다 (중복 추가는 ShowOverlay 가 방지)
	ShowOverlay(currentAlpha);
	StartFade(FMath::Clamp(targetAlpha, 0.f, 1.f), MoveTemp(onFinished));
	return true;
}

void UCDLevelTransitionSubsystem::OpenPendingLevel()
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetGameInstance(), pendingLevel);
}

void UCDLevelTransitionSubsystem::HandlePreLoadMap(const FString& mapName)
{
	const TSoftClassPtr<UUserWidget>& loadingScreenClass = GetDefault<UCDDeveloperSettings>()->loadingScreenClass;
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
	if (!bTraveling)
	{
		return;
	}
	bTraveling = false;

	// 레벨 로드 중 뷰포트 위젯이 모두 제거되므로, 가린 상태로 다시 붙이고 페이드 인
	ShowOverlay(1.f);
	StartFade(0.f, nullptr);
}

void UCDLevelTransitionSubsystem::StartFade(float targetAlpha, TFunction<void()> onFinished)
{
	// 진행 중이던 페이드가 끊기면 그 완료 콜백을 먼저 호출 (BP 노드가 영원히 대기하지 않도록)
	TFunction<void()> interrupted = MoveTemp(fadeFinished);

	fadeStartAlpha = currentAlpha;
	fadeTargetAlpha = targetAlpha;
	fadeElapsed = 0.f;
	fadeFinished = MoveTemp(onFinished);

	if (!fadeTickHandle.IsValid())
	{
		fadeTickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::TickFade));
	}

	if (interrupted)
	{
		interrupted();
	}
}

bool UCDLevelTransitionSubsystem::TickFade(float deltaTime)
{
	const float duration = GetDefault<UCDDeveloperSettings>()->fadeDuration;
	fadeElapsed += FMath::Min(deltaTime, maxFadeDeltaTime);
	const float ratio = duration > 0.f ? FMath::Clamp(fadeElapsed / duration, 0.f, 1.f) : 1.f;
	SetOverlayAlpha(FMath::Lerp(fadeStartAlpha, fadeTargetAlpha, ratio));
	if (ratio < 1.f)
	{
		return true;
	}

	// 완료: 티커 해제 (false 반환), 완전히 걷혔으면 오버레이 제거
	fadeTickHandle.Reset();
	if (fadeTargetAlpha <= 0.f)
	{
		HideOverlay();
	}

	// 콜백 안에서 다시 페이드를 시작해도 안전하도록 먼저 꺼내고 비운다
	TFunction<void()> finished = MoveTemp(fadeFinished);
	fadeFinished = nullptr;
	if (finished)
	{
		finished();
	}
	return false;
}

TSharedPtr<SWidget> UCDLevelTransitionSubsystem::GetOrCreateOverlay()
{
	if (overlay.IsValid())
	{
		return overlay;
	}

	const TSoftClassPtr<UUserWidget>& fadeWidgetClass = GetDefault<UCDDeveloperSettings>()->fadeWidgetClass;
	if (UClass* widgetClass = fadeWidgetClass.LoadSynchronous())
	{
		// 게임 인스턴스 소유 → 레벨이 바뀌어도 위젯 객체는 유지된다
		fadeWidget = CreateWidget<UUserWidget>(GetGameInstance(), widgetClass);
		if (fadeWidget)
		{
			overlay = fadeWidget->TakeWidget();
		}
	}

	if (!overlay.IsValid())
	{
		// Visible: 페이드 중에는 아래 UI 클릭을 막는다
		blackOverlay = SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
			.Visibility(EVisibility::Visible);
		overlay = blackOverlay;
	}
	return overlay;
}

void UCDLevelTransitionSubsystem::ShowOverlay(float alpha)
{
	const UGameInstance* gameInstance = GetGameInstance();
	UGameViewportClient* viewportClient = gameInstance ? gameInstance->GetGameViewportClient() : nullptr;
	ULocalPlayer* localPlayer = gameInstance ? gameInstance->GetFirstGamePlayer() : nullptr;
	const TSharedPtr<SWidget> overlayWidget = GetOrCreateOverlay();
	if (!viewportClient || !localPlayer || !overlayWidget.IsValid())
	{
		return;
	}

	// 중복 추가 방지 후 플레이어 화면 최상단에 붙임 (CommonUI 레이아웃보다 높은 ZOrder)
	const int32 zOrder = GetDefault<UCDDeveloperSettings>()->fadeZOrder;
	viewportClient->RemoveViewportWidgetForPlayer(localPlayer, overlayWidget.ToSharedRef());
	viewportClient->AddViewportWidgetForPlayer(localPlayer, overlayWidget.ToSharedRef(), zOrder);
	SetOverlayAlpha(alpha);
}

void UCDLevelTransitionSubsystem::HideOverlay()
{
	const UGameInstance* gameInstance = GetGameInstance();
	UGameViewportClient* viewportClient = gameInstance ? gameInstance->GetGameViewportClient() : nullptr;
	ULocalPlayer* localPlayer = gameInstance ? gameInstance->GetFirstGamePlayer() : nullptr;
	if (overlay.IsValid() && viewportClient && localPlayer)
	{
		viewportClient->RemoveViewportWidgetForPlayer(localPlayer, overlay.ToSharedRef());
	}
	currentAlpha = 0.f;
}

void UCDLevelTransitionSubsystem::SetOverlayAlpha(float alpha)
{
	currentAlpha = alpha;
	if (fadeWidget)
	{
		fadeWidget->SetRenderOpacity(alpha);
	}
	else if (blackOverlay.IsValid())
	{
		blackOverlay->SetBorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, alpha));
	}
}
