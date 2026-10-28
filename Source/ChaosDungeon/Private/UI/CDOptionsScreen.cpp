#include "UI/CDOptionsScreen.h"
#include "Animation/WidgetAnimation.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameUserSettings.h"
#include "Input/CommonUIActionRouterBase.h"
#include "UI/CDUIManagerSubsystem.h"
#include "UI/Component/CDCommonTabListWidgetBase.h"
#include "UI/Component/CDOptionListView.h"
#include "UI/ListData/CDOptionData.h"
#include "UI/ListData/CDOptionRegistry.h"

#define LOCTEXT_NAMESPACE "CDOptionsScreen"

void UCDOptionsScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	tabList->OnTabSelected.AddDynamic(this, &ThisClass::HandleTabSelected);
	optionList->OnItemIsHoveredChanged().AddUObject(this, &ThisClass::HandleItemHovered);

	if (resetButton)
	{
		resetButton->OnClicked().AddUObject(this, &ThisClass::HandleResetClicked);
	}

	if (closeButton)
	{
		closeButton->OnClicked().AddUObject(this, &ThisClass::HandleCloseClicked);
	}
}

void UCDOptionsScreen::NativeOnActivated()
{
	// Super 에서 bPauseGame 이면 게임 일시정지 (일시정지 메뉴가 비활성되며 풀린 것을 다시 건다)
	Super::NativeOnActivated();

	// 처음 열릴 때만 탭 · 항목 생성. 다시 열면 다른 곳에서 바뀐 값이 있을 수 있으므로 표시만 갱신
	if (registry == nullptr)
	{
		registry = NewObject<UCDOptionRegistry>(this);
		registry->Build(GetOwningPlayer());
	}
	else
	{
		registry->RefreshAll();
	}

	for (const FCDOptionTab& tab : registry->GetTabs())
	{
		tabList->RequestRegisterTab(tab.tabId, tab.displayName);
	}

	// 마지막으로 보던 탭 (처음이면 첫 탭)
	FName tabToShow = currentTabId;
	if (tabToShow.IsNone() && (registry->GetTabs().Num() > 0))
	{
		tabToShow = registry->GetTabs()[0].tabId;
	}

	if (tabToShow.IsNone() == false)
	{
		if (tabList->GetSelectedTabId() == tabToShow)
		{
			HandleTabSelected(tabToShow);
		}
		else
		{
			tabList->SelectTabByID(tabToShow);
		}
	}

	// 일시정지 메뉴와 같은 방식: 입력 모드를 바로 적용
	const TOptional<FUIInputConfig> desiredConfig = GetDesiredInputConfig();
	UCommonUIActionRouterBase* actionRouter = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetOwningLocalPlayer());
	if (actionRouter && desiredConfig.IsSet())
	{
		actionRouter->SetActiveUIInputConfig(desiredConfig.GetValue(), this);
	}

	if (openAnim)
	{
		PlayAnimation(openAnim);
	}
}

void UCDOptionsScreen::NativeOnDeactivated()
{
	// 값은 이미 적용돼 있으므로 디스크 저장만
	if (UGameUserSettings* settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		settings->SaveSettings();
	}

	Super::NativeOnDeactivated();
}

UWidget* UCDOptionsScreen::NativeGetDesiredFocusTarget() const
{
	return optionList;
}

void UCDOptionsScreen::HandleTabSelected(FName tabId)
{
	const FCDOptionTab* tab = registry ? registry->FindTab(tabId) : nullptr;
	if (tab == nullptr)
	{
		return;
	}

	currentTabId = tabId;

	TArray<UObject*> items;
	for (UCDOptionData* option : tab->options)
	{
		items.Add(option);
	}
	optionList->SetListItems(items);
	optionList->ScrollToTop();

	ShowDescription(nullptr);
}

void UCDOptionsScreen::HandleItemHovered(UObject* item, bool bIsHovered)
{
	if (bIsHovered)
	{
		ShowDescription(Cast<UCDOptionData>(item));
	}
}

void UCDOptionsScreen::HandleResetClicked()
{
	UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(GetOwningPlayer());
	const FCDOptionTab* tab = registry ? registry->FindTab(currentTabId) : nullptr;
	if ((uiManager == nullptr) || (tab == nullptr))
	{
		return;
	}

	TWeakObjectPtr<UCDOptionsScreen> weakThis(this);
	const FName tabId = currentTabId;
	uiManager->ShowConfirm(ECDConfirmType::YesNo,
		LOCTEXT("ResetTitle", "기본값"),
		FText::Format(LOCTEXT("ResetMessage", "'{0}' 설정을 기본값으로 되돌리시겠습니까?"), tab->displayName),
		FText::GetEmpty(),
		[weakThis, tabId](ECDConfirmResult result)
		{
			UCDOptionsScreen* screen = weakThis.Get();
			if ((screen == nullptr) || (result != ECDConfirmResult::Confirmed) || (screen->registry == nullptr))
			{
				return;
			}

			screen->registry->ResetTab(tabId);
		});
}

void UCDOptionsScreen::HandleCloseClicked()
{
	DeactivateWidget();
}

void UCDOptionsScreen::ShowDescription(const UCDOptionData* option)
{
	if (descTitleText)
	{
		descTitleText->SetText(option ? option->GetDisplayName() : FText::GetEmpty());
	}

	if (descBodyText)
	{
		descBodyText->SetText(option ? option->GetDescription() : LOCTEXT("DescHint", "항목에 마우스를 올리면 설명이 표시됩니다."));
	}
}

#undef LOCTEXT_NAMESPACE
