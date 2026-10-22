#include "UI/CDDifficultySelectScreen.h"
#include "CommonButtonBase.h"
#include "Groups/CommonButtonGroupBase.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Core/CDRunSubsystem.h"
#include "Engine/GameInstance.h"
#include "UI/CDDifficultyCard.h"

void UCDDifficultySelectScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 카드 3장을 버튼 그룹으로 묶는다. 그룹이 "하나 선택 → 나머지 해제"를 처리
	// (토글이 아닌 CommonButton 은 SetIsSelected(false) 로 해제되지 않기 때문)
	cardGroup = NewObject<UCommonButtonGroupBase>(this);
	cardGroup->SetSelectionRequired(true);

	for (UCDDifficultyCard* card : { easyCard.Get(), normalCard.Get(), hardCard.Get() })
	{
		card->SetIsSelectable(true);
		card->SetIsToggleable(false);
		cardGroup->AddWidget(card);
		card->OnClicked().AddUObject(this, &ThisClass::HandleCardClicked, card->GetDifficulty());
	}

	startButton->OnClicked().AddUObject(this, &ThisClass::HandleStartClicked);

	if (cancelButton)
	{
		cancelButton->OnClicked().AddUObject(this, &ThisClass::HandleCancelClicked);
	}
}

void UCDDifficultySelectScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	bStarting = false;
	SelectDifficulty(defaultDifficulty);

	if (openAnim)
	{
		PlayAnimation(openAnim);
	}
}

UWidget* UCDDifficultySelectScreen::NativeGetDesiredFocusTarget() const
{
	// 패드/키보드 포커스는 선택된 카드에서 시작
	return FindCard(selectedDifficulty);
}

void UCDDifficultySelectScreen::SelectDifficulty(ECDDifficulty newDifficulty)
{
	selectedDifficulty = newDifficulty;

	// 선택은 그룹에 맡긴다 → 이전 카드는 그룹이 해제하고 NativeOnDeselected 가 하이라이트를 끔
	if (UCDDifficultyCard* card = FindCard(newDifficulty))
	{
		if (card->GetSelected() == false)
		{
			cardGroup->SelectButtonAtIndex(cardGroup->FindButtonIndex(card), false);
		}
	}

	if (descriptionText)
	{
		if (const UCDDifficultyCard* card = FindCard(newDifficulty))
		{
			descriptionText->SetText(card->GetDescription());
		}
	}
}

void UCDDifficultySelectScreen::HandleCardClicked(ECDDifficulty clickedDifficulty)
{
	SelectDifficulty(clickedDifficulty);
}

void UCDDifficultySelectScreen::HandleStartClicked()
{
	if (bStarting)
	{
		return;
	}
	bStarting = true;

	// 레벨 이동 · 페이드는 RunSubsystem → LevelTransitionSubsystem 이 처리
	if (UCDRunSubsystem* run = GetGameInstance()->GetSubsystem<UCDRunSubsystem>())
	{
		run->StartRun(selectedDifficulty);
	}
}

void UCDDifficultySelectScreen::HandleCancelClicked()
{
	DeactivateWidget();
}

UCDDifficultyCard* UCDDifficultySelectScreen::FindCard(ECDDifficulty targetDifficulty) const
{
	switch (targetDifficulty)
	{
	case ECDDifficulty::Easy:
	{
		return easyCard;
	}
	case ECDDifficulty::Hard:
	{
		return hardCard;
	}
	default:
	{
		return normalCard;
	}
	}
}
