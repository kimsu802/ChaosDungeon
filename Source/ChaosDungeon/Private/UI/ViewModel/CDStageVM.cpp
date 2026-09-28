#include "UI/ViewModel/CDStageVM.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"

void UCDStageVM::StartListening(UCDMessageSubsystem& messages)
{
	handles.Add(messages.Listen(CDTags::Msg_Stage_Started, this, &ThisClass::HandleStageStarted));
	handles.Add(messages.Listen(CDTags::Msg_Stage_Progress, this, &ThisClass::HandleStageProgress));
	handles.Add(messages.Listen(CDTags::Msg_Run_DeathCount, this, &ThisClass::HandleDeathCount));
	handles.Add(messages.Listen(CDTags::Msg_Combat_BossAppeared, this, &ThisClass::HandleBossAppeared));
}

void UCDStageVM::StopListening(UCDMessageSubsystem& messages)
{
	for (FCDListenerHandle& handle : handles)
	{
		messages.Unlisten(handle);
	}
	handles.Reset();
}

void UCDStageVM::HandleStageStarted(const FCDStageMessage& message)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsInStage, true);
	UE_MVVM_SET_PROPERTY_VALUE(bIsBossVisible, false);
	UE_MVVM_SET_PROPERTY_VALUE(stageNumber, message.stageNumber);
	UE_MVVM_SET_PROPERTY_VALUE(progress, message.progress);
	UE_MVVM_SET_PROPERTY_VALUE(remainingDeaths, message.remainingDeaths);
}

void UCDStageVM::HandleStageProgress(const FCDStageMessage& message)
{
	UE_MVVM_SET_PROPERTY_VALUE(progress, message.progress);
}

void UCDStageVM::HandleDeathCount(const FCDRunMessage& message)
{
	UE_MVVM_SET_PROPERTY_VALUE(remainingDeaths, message.remainingDeaths);
}

void UCDStageVM::HandleBossAppeared(const FCDActorMessage& message)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsBossVisible, true);
}
