#include "UI/CDClassPortraitButton.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Core/CDHubGameMode.h"
#include "Core/CDPlayerClassSubsystem.h"
#include "Data/CDCharacterClassData.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

void UCDClassPortraitButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 디자이너에서 classData 를 바꾸면 바로 미리보기에 반영
	ApplyAppearance();
}

void UCDClassPortraitButton::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyAppearance();

	const UGameInstance* gameInstance = GetGameInstance();
	UCDPlayerClassSubsystem* classSubsystem = gameInstance ? gameInstance->GetSubsystem<UCDPlayerClassSubsystem>() : nullptr;
	if (classSubsystem == nullptr)
	{
		RefreshSelected(nullptr);
		return;
	}

	classSubsystem->OnClassChanged().RemoveAll(this);
	classSubsystem->OnClassChanged().AddUObject(this, &ThisClass::HandleClassChanged);
	RefreshSelected(classSubsystem->GetSelectedClass());
}

void UCDClassPortraitButton::NativeDestruct()
{
	const UGameInstance* gameInstance = GetGameInstance();
	if (UCDPlayerClassSubsystem* classSubsystem = gameInstance ? gameInstance->GetSubsystem<UCDPlayerClassSubsystem>() : nullptr)
	{
		classSubsystem->OnClassChanged().RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UCDClassPortraitButton::NativeOnClicked()
{
	Super::NativeOnClicked();

	UWorld* world = GetWorld();
	ACDHubGameMode* hubGameMode = world ? world->GetAuthGameMode<ACDHubGameMode>() : nullptr;
	if (hubGameMode == nullptr)
	{
		return;
	}

	// 이미 같은 직업이면 GameMode 쪽에서 무시
	hubGameMode->RespawnPlayerAsClass(GetOwningPlayer(), classData);
}

void UCDClassPortraitButton::ApplyAppearance()
{
	if (classData == nullptr)
	{
		return;
	}

	if (portraitImage && classData->portrait)
	{
		// 크기는 디자이너 값 유지
		portraitImage->SetBrushFromTexture(classData->portrait, false);
	}

	if (nameText)
	{
		nameText->SetText(classData->className);
	}
}

void UCDClassPortraitButton::HandleClassChanged(const UCDCharacterClassData* currentClass)
{
	RefreshSelected(currentClass);
}

void UCDClassPortraitButton::RefreshSelected(const UCDCharacterClassData* currentClass)
{
	if (selectedMark == nullptr)
	{
		return;
	}

	const bool isCurrent = (classData != nullptr) && (classData.Get() == currentClass);
	selectedMark->SetVisibility(isCurrent ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
