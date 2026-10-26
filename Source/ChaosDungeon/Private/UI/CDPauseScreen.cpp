#include "UI/CDPauseScreen.h"
#include "CommonButtonBase.h"
#include "Animation/WidgetAnimation.h"
#include "Core/CDDungeonGameMode.h"
#include "Core/CDRunSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/CDUIManagerSubsystem.h"

#define LOCTEXT_NAMESPACE "CDPauseScreen"

void UCDPauseScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	resumeButton->OnClicked().AddUObject(this, &ThisClass::HandleResumeClicked);
	mainMenuButton->OnClicked().AddUObject(this, &ThisClass::HandleMainMenuClicked);

	if (optionsButton)
	{
		optionsButton->OnClicked().AddUObject(this, &ThisClass::HandleOptionsClicked);
	}

	if (guideButton)
	{
		guideButton->OnClicked().AddUObject(this, &ThisClass::HandleGuideClicked);
	}

	if (abandonButton)
	{
		abandonButton->OnClicked().AddUObject(this, &ThisClass::HandleAbandonClicked);
	}

	if (quitButton)
	{
		quitButton->OnClicked().AddUObject(this, &ThisClass::HandleQuitClicked);
	}
}

void UCDPauseScreen::NativeOnActivated()
{
	// Super 에서 bPauseGame 이면 게임 일시정지
	Super::NativeOnActivated();

	// 던전 포기는 던전에서만
	if (abandonButton)
	{
		abandonButton->SetVisibility(IsInDungeon() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 옵션 화면이 아직 없으면 버튼만 비활성
	const UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(GetOwningPlayer());
	if (optionsButton)
	{
		optionsButton->SetIsEnabled(uiManager && uiManager->HasOptionsScreen());
	}

	// 게임 입력(Esc) 으로 열려 포커스가 게임 뷰포트에 남아 있으므로, 입력 모드를 바로 적용하고 계속하기에 포커스
	const TOptional<FUIInputConfig> desiredConfig = GetDesiredInputConfig();
	UCommonUIActionRouterBase* actionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetOwningLocalPlayer());
	if (actionRouter && desiredConfig.IsSet())
	{
		actionRouter->SetActiveUIInputConfig(desiredConfig.GetValue(), this);
	}
	resumeButton->SetFocus();

	if (openAnim)
	{
		PlayAnimation(openAnim);
	}
}

UWidget* UCDPauseScreen::NativeGetDesiredFocusTarget() const
{
	return resumeButton;
}

void UCDPauseScreen::HandleResumeClicked()
{
	// Deactivate → UCDActivatableScreen::NativeOnDeactivated 에서 일시정지 해제
	DeactivateWidget();
}

void UCDPauseScreen::HandleOptionsClicked()
{
	if (UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(GetOwningPlayer()))
	{
		uiManager->ShowOptions();
	}
}

void UCDPauseScreen::HandleGuideClicked()
{
	if (UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(GetOwningPlayer()))
	{
		uiManager->ToggleGuide();
	}
}

void UCDPauseScreen::HandleAbandonClicked()
{
	UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(GetOwningPlayer());
	if (uiManager == nullptr)
	{
		return;
	}

	TWeakObjectPtr<UCDPauseScreen> weakThis(this);
	uiManager->ShowConfirm(ECDConfirmType::YesNo,
		LOCTEXT("AbandonTitle", "던전 포기"),
		LOCTEXT("AbandonMessage", "던전을 포기하시겠습니까?"),
		LOCTEXT("AbandonSub", "이번 도전은 실패로 기록됩니다."),
		[weakThis](ECDConfirmResult result)
		{
			UCDPauseScreen* screen = weakThis.Get();
			if (screen == nullptr || result != ECDConfirmResult::Confirmed)
			{
				return;
			}

			// 메뉴를 먼저 닫아 일시정지를 풀고, FailRun → Run.Finished → FAILED 결과 화면
			ACDDungeonGameMode* dungeonMode = screen->GetWorld()->GetAuthGameMode<ACDDungeonGameMode>();
			screen->DeactivateWidget();
			if (dungeonMode)
			{
				dungeonMode->AbandonRun();
			}
		});
}

void UCDPauseScreen::HandleMainMenuClicked()
{
	UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(GetOwningPlayer());
	if (uiManager == nullptr)
	{
		return;
	}

	const FText subMessage = IsInDungeon()
		? LOCTEXT("MainMenuSubDungeon", "진행 중인 던전 기록은 저장되지 않습니다.")
		: FText::GetEmpty();

	TWeakObjectPtr<UCDPauseScreen> weakThis(this);
	uiManager->ShowConfirm(ECDConfirmType::YesNo,
		LOCTEXT("MainMenuTitle", "메인 메뉴"),
		LOCTEXT("MainMenuMessage", "메인 메뉴로 돌아가시겠습니까?"),
		subMessage,
		[weakThis](ECDConfirmResult result)
		{
			UCDPauseScreen* screen = weakThis.Get();
			if (screen == nullptr || result != ECDConfirmResult::Confirmed)
			{
				return;
			}

			// 레벨 이동 시 일시정지는 LevelTransitionSubsystem 이 풀어 준다
			if (UCDRunSubsystem* run = screen->GetGameInstance()->GetSubsystem<UCDRunSubsystem>())
			{
				run->ReturnToMainMenu();
			}
		});
}

void UCDPauseScreen::HandleQuitClicked()
{
	UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(GetOwningPlayer());
	if (uiManager == nullptr)
	{
		return;
	}

	TWeakObjectPtr<UCDPauseScreen> weakThis(this);
	uiManager->ShowConfirm(ECDConfirmType::YesNo,
		LOCTEXT("QuitTitle", "게임 종료"),
		LOCTEXT("QuitMessage", "게임을 종료하시겠습니까?"),
		FText::GetEmpty(),
		[weakThis](ECDConfirmResult result)
		{
			UCDPauseScreen* screen = weakThis.Get();
			if (screen == nullptr || result != ECDConfirmResult::Confirmed)
			{
				return;
			}

			UKismetSystemLibrary::QuitGame(screen, screen->GetOwningPlayer(), EQuitPreference::Quit, false);
		});
}

bool UCDPauseScreen::IsInDungeon() const
{
	const UWorld* world = GetWorld();
	return world && (world->GetAuthGameMode<ACDDungeonGameMode>() != nullptr);
}

#undef LOCTEXT_NAMESPACE
