#include "UI/CDDifficultyCard.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Core/CDRunSubsystem.h"
#include "Data/CDDungeonData.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Settings/CDDeveloperSettings.h"
#include "UI/ViewModel/CDRunResultVM.h"

#define LOCTEXT_NAMESPACE "CDDifficultyCard"

namespace
{
	// 배율 표기 "×1.5" (소수 한 자리까지, 1.0 은 "×1")
	FText FormatMultiplier(float value)
	{
		FNumberFormattingOptions options;
		options.MinimumFractionalDigits = 0;
		options.MaximumFractionalDigits = 1;
		return FText::Format(LOCTEXT("Multiplier", "×{0}"), FText::AsNumber(value, &options));
	}
}

void UCDDifficultyCard::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 디자이너에서 difficulty 를 바꾸면 바로 미리보기에 반영
	ApplyAppearance();
	RefreshHighlight();
}

void UCDDifficultyCard::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyAppearance();
	ApplyInfo();
	RefreshHighlight();
}

void UCDDifficultyCard::NativeOnHovered()
{
	Super::NativeOnHovered();

	bHovered = true;
	RefreshHighlight();
}

void UCDDifficultyCard::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();

	bHovered = false;
	RefreshHighlight();
}

void UCDDifficultyCard::NativeOnSelected(bool bBroadcast)
{
	Super::NativeOnSelected(bBroadcast);

	RefreshHighlight();
}

void UCDDifficultyCard::NativeOnDeselected(bool bBroadcast)
{
	Super::NativeOnDeselected(bBroadcast);

	RefreshHighlight();
}

void UCDDifficultyCard::ApplyAppearance()
{
	const FLinearColor accent = UCDRunResultVM::GetDifficultyAccentColor(difficulty);

	if (nameText)
	{
		nameText->SetText(UCDRunResultVM::GetDifficultyDisplayName(difficulty));
	}

	if (englishText)
	{
		englishText->SetText(englishName);
	}

	if (emblemImage && emblemTexture)
	{
		// 크기는 디자이너 값 유지
		emblemImage->SetBrushFromTexture(emblemTexture, false);
	}

	// 색(RGB)만 난이도 색으로 바꾸고, 알파는 디자이너에서 정한 값을 유지
	for (UImage* tinted : { headerWash.Get(), selectBorder.Get(), outerGlow.Get() })
	{
		if (tinted)
		{
			FLinearColor color = accent;
			color.A = tinted->GetColorAndOpacity().A;
			tinted->SetColorAndOpacity(color);
		}
	}
}

void UCDDifficultyCard::ApplyInfo()
{
	const UCDDungeonData* dungeonData = GetDefault<UCDDeveloperSettings>()->dungeonData.LoadSynchronous();
	if (dungeonData)
	{
		const FCDDifficultySettings& settings = dungeonData->GetDifficultySettings(difficulty);

		if (healthText)
		{
			healthText->SetText(FormatMultiplier(settings.healthMultiplier));
		}

		if (attackText)
		{
			attackText->SetText(FormatMultiplier(settings.attackMultiplier));
		}

		if (reviveText)
		{
			reviveText->SetText(settings.maxDeaths < 0
				? LOCTEXT("ReviveUnlimited", "무제한")
				: FText::Format(LOCTEXT("ReviveCount", "{0}회"), settings.maxDeaths));
		}
	}

	if (bestRecordText == nullptr)
	{
		return;
	}

	const UCDRunSubsystem* run = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCDRunSubsystem>() : nullptr;
	const FCDRunRecord* best = run ? run->FindBestRecord(difficulty) : nullptr;

	if (best == nullptr)
	{
		bestRecordText->SetText(LOCTEXT("NoRecord", "기록 없음"));
	}
	else if (best->bSuccess)
	{
		bestRecordText->SetText(UCDRunResultVM::FormatRunTime(best->elapsedTime));
	}
	else
	{
		// 클리어 전 기록은 도달한 스테이지로 표시
		bestRecordText->SetText(FText::Format(LOCTEXT("BestStage", "{0}스테이지"), best->clearedStages));
	}
}

void UCDDifficultyCard::RefreshHighlight()
{
	// 선택 = 테두리 · 글로우 100%, 호버만 = 일부, 그 외 = 숨김
	const bool isSelected = GetSelected();
	const float opacity = isSelected ? 1.f : (bHovered ? hoverHighlightOpacity : 0.f);

	if (selectBorder)
	{
		selectBorder->SetRenderOpacity(opacity);
	}

	if (outerGlow)
	{
		outerGlow->SetRenderOpacity(isSelected ? 1.f : 0.f);
	}

	if (headerWash)
	{
		headerWash->SetRenderOpacity(isSelected ? 1.f : 0.55f);
	}
}

#undef LOCTEXT_NAMESPACE
