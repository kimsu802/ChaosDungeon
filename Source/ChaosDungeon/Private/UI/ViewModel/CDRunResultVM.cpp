#include "UI/ViewModel/CDRunResultVM.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"

#define LOCTEXT_NAMESPACE "CDRunResultVM"

void UCDRunResultVM::StartListening(UCDMessageSubsystem& messages)
{
	finishedHandle = messages.Listen(CDTags::Msg_Run_Finished, this, &ThisClass::HandleRunFinished);
}

void UCDRunResultVM::StopListening(UCDMessageSubsystem& messages)
{
	messages.Unlisten(finishedHandle);
}

void UCDRunResultVM::HandleRunFinished(const FCDRunMessage& message)
{
	// 구조체는 == 비교가 없으므로 직접 대입 후 알림
	record = message.record;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(record);
	UE_MVVM_SET_PROPERTY_VALUE(bIsNewRecord, message.bNewRecord);

	// 표시용 텍스트는 record 에서 계산되므로 함께 알림
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetElapsedTimeText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetKillsText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDeathsText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDamageDealtText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDamageTakenText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetClearedStagesText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDifficultyText);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDifficultyColor);
}

FText UCDRunResultVM::GetElapsedTimeText() const
{
	return FormatRunTime(record.elapsedTime);
}

FText UCDRunResultVM::GetKillsText() const
{
	return FText::AsNumber(record.kills);
}

FText UCDRunResultVM::GetDeathsText() const
{
	return FText::AsNumber(record.deaths);
}

FText UCDRunResultVM::GetDamageDealtText() const
{
	return FText::AsNumber(FMath::RoundToInt(record.damageDealt));
}

FText UCDRunResultVM::GetDamageTakenText() const
{
	return FText::AsNumber(FMath::RoundToInt(record.damageTaken));
}

FText UCDRunResultVM::GetClearedStagesText() const
{
	return FText::AsNumber(record.clearedStages);
}

FText UCDRunResultVM::GetDifficultyText() const
{
	return GetDifficultyDisplayName(record.difficulty);
}

FLinearColor UCDRunResultVM::GetDifficultyColor() const
{
	return GetDifficultyAccentColor(record.difficulty);
}

FText UCDRunResultVM::FormatRunTime(float seconds)
{
	const int32 totalSeconds = FMath::Max(0, FMath::FloorToInt(seconds));
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), totalSeconds / 60, totalSeconds % 60));
}

FText UCDRunResultVM::GetDifficultyDisplayName(ECDDifficulty difficulty)
{
	switch (difficulty)
	{
	case ECDDifficulty::Easy:
	{
		return LOCTEXT("DifficultyEasy", "쉬움");
	}
	case ECDDifficulty::Hard:
	{
		return LOCTEXT("DifficultyHard", "어려움");
	}
	default:
	{
		return LOCTEXT("DifficultyNormal", "보통");
	}
	}
}

FLinearColor UCDRunResultVM::GetDifficultyAccentColor(ECDDifficulty difficulty)
{
	switch (difficulty)
	{
	case ECDDifficulty::Easy:
	{
		return FLinearColor::FromSRGBColor(FColor(0x7E, 0xD3, 0x8A));
	}
	case ECDDifficulty::Hard:
	{
		return FLinearColor::FromSRGBColor(FColor(0xE6, 0x4A, 0x3C));
	}
	default:
	{
		return FLinearColor::FromSRGBColor(FColor(0xF2, 0xB9, 0x5C));
	}
	}
}

#undef LOCTEXT_NAMESPACE
