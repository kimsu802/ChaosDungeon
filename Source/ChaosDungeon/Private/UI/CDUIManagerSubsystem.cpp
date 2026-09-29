#include "UI/CDUIManagerSubsystem.h"
#include "CommonActivatableWidget.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Data/CDUIScreenSet.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "UI/CDPrimaryLayout.h"
#include "UI/ViewModel/CDViewModelSubsystem.h"
#include "Settings/CDDeveloperSettings.h"

UCDUIManagerSubsystem* UCDUIManagerSubsystem::Get(const APlayerController* playerController)
{
	const ULocalPlayer* localPlayer = playerController ? playerController->GetLocalPlayer() : nullptr;
	if (!localPlayer)
	{
		return nullptr;
	}
	return localPlayer->GetSubsystem<UCDUIManagerSubsystem>();
}

void UCDUIManagerSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);
	collection.InitializeDependency<UCDViewModelSubsystem>();

	const UCDDeveloperSettings* DeveloperSettings = GetDefault<UCDDeveloperSettings>();
	screenSet = DeveloperSettings->screenSet.LoadSynchronous();

	//ensureMsgf(screenSet, TEXT("DefaultGame.ini 에 screenSetAsset 을 지정하세요."));

	runFinishedHandle = GetMessages()->Listen(CDTags::Msg_Run_Finished, this, &ThisClass::HandleRunFinished);
}

void UCDUIManagerSubsystem::Deinitialize()
{
	GetMessages()->Unlisten(runFinishedHandle);
	Super::Deinitialize();
}

void UCDUIManagerSubsystem::InitializeForPlayer(APlayerController* playerController)
{
	if (layout && layout->GetOwningPlayer() != playerController)
	{
		layout->RemoveFromParent();
		layout = nullptr;
	}

	// HUD 가 바인딩할 ViewModel 을 먼저 이 플레이어의 폰에 연결
	GetLocalPlayer()->GetSubsystem<UCDViewModelSubsystem>()->BindPlayer(playerController);

	EnsureLayout();
	PushScreen(CDTags::UI_Layer_Game, screenSet->hudScreen);
}

UCommonActivatableWidget* UCDUIManagerSubsystem::PushScreen(FGameplayTag layerTag, const TSoftClassPtr<UCommonActivatableWidget>& screenClass)
{
	UCDPrimaryLayout* rootLayout = EnsureLayout();
	if (!rootLayout)
	{
		return nullptr;
	}
	return rootLayout->PushToLayer(layerTag, screenClass.LoadSynchronous());
}

void UCDUIManagerSubsystem::ClearLayerByTag(FGameplayTag layerTag)
{
	if (layout)
	{
		layout->ClearStack(layerTag);
	}
}

void UCDUIManagerSubsystem::TogglePauseMenu()
{
	ToggleScreen(CDTags::UI_Layer_GameMenu, screenSet->pauseScreen);
}

void UCDUIManagerSubsystem::ToggleGuide()
{
	ToggleScreen(CDTags::UI_Layer_GameMenu, screenSet->guideScreen);
}

void UCDUIManagerSubsystem::ShowTitle()
{
	PushScreen(CDTags::UI_Layer_Modal, screenSet->titleScreen);
}

UCDPrimaryLayout* UCDUIManagerSubsystem::EnsureLayout()
{
	APlayerController* playerController = GetLocalPlayer()->GetPlayerController(nullptr);
	if (!layout && playerController && screenSet)
	{
		layout = CreateWidget<UCDPrimaryLayout>(playerController, screenSet->layoutClass.LoadSynchronous());
		layout->AddToPlayerScreen(100);
	}
	return layout;
}

void UCDUIManagerSubsystem::ToggleScreen(FGameplayTag layerTag, const TSoftClassPtr<UCommonActivatableWidget>& screenClass)
{
	UCDPrimaryLayout* rootLayout = EnsureLayout();
	if (rootLayout && !rootLayout->DeactivateIfActive(layerTag, screenClass.LoadSynchronous()))
	{
		PushScreen(layerTag, screenClass);
	}
}

void UCDUIManagerSubsystem::HandleRunFinished(const FCDRunMessage& message)
{
	// 표시할 값은 UCDRunResultVM 이 같은 메시지로 채운다
	PushScreen(CDTags::UI_Layer_Modal, screenSet->resultScreen);
}

UCDMessageSubsystem* UCDUIManagerSubsystem::GetMessages() const
{
	return GetLocalPlayer()->GetGameInstance()->GetSubsystem<UCDMessageSubsystem>();
}
