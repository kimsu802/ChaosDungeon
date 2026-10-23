#include "UI/ViewModel/CDStageVM.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Character/CDMonsterBase.h"
#include "Data/CDBossData.h"

#define LOCTEXT_NAMESPACE "CDStageVM"

void UCDStageVM::StartListening(UCDMessageSubsystem& messages)
{
	handles.Add(messages.Listen(CDTags::Msg_Stage_Started, this, &ThisClass::HandleStageStarted));
	handles.Add(messages.Listen(CDTags::Msg_Stage_Progress, this, &ThisClass::HandleStageProgress));
	handles.Add(messages.Listen(CDTags::Msg_Run_DeathCount, this, &ThisClass::HandleDeathCount));
	handles.Add(messages.Listen(CDTags::Msg_Run_Finished, this, &ThisClass::HandleRunFinished));
	handles.Add(messages.Listen(CDTags::Msg_Combat_BossAppeared, this, &ThisClass::HandleBossAppeared));
	handles.Add(messages.Listen(CDTags::Msg_Combat_Death, this, &ThisClass::HandleDeath));
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
	currentBoss = nullptr;
	UE_MVVM_SET_PROPERTY_VALUE(stageNumber, message.stageNumber);

	// 데이터에 이름이 없으면 "스테이지 N" 으로 대신 표시
	SetStageName(message.stageName.IsEmpty()
		? FText::Format(LOCTEXT("StageFallbackName", "스테이지 {0}"), message.stageNumber)
		: message.stageName);

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

void UCDStageVM::HandleRunFinished(const FCDRunMessage& message)
{
	// 결과 화면 이후 허브로 돌아가도 true 로 남지 않도록 여기서 내린다 (다음 런의 Stage.Started 에서 다시 true)
	UE_MVVM_SET_PROPERTY_VALUE(bIsInStage, false);
	UE_MVVM_SET_PROPERTY_VALUE(bIsBossVisible, false);
	currentBoss = nullptr;
}

void UCDStageVM::HandleBossAppeared(const FCDActorMessage& message)
{
	currentBoss = message.actor.Get();

	// 보스 데이터에서 HP 바 표시 정보 읽기 (UCDBossData 가 아니면 기본값)
	const ACDMonsterBase* monster = Cast<ACDMonsterBase>(message.actor.Get());
	const UCDBossData* bossData = monster ? Cast<UCDBossData>(monster->GetMonsterData()) : nullptr;

	// 줄 수 → 이름 → 표시 순서: 바가 보이는 순간 이미 올바른 줄 수로 그려지도록
	UE_MVVM_SET_PROPERTY_VALUE(bossHpLineCount, bossData ? FMath::Max(1, bossData->hpLineCount) : 1);
	SetBossName(bossData ? bossData->bossName : FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(bIsBossVisible, true);
}

void UCDStageVM::HandleDeath(const FCDDeathMessage& message)
{
	if (!currentBoss.IsValid() || message.victim != currentBoss.Get())
	{
		return;
	}

	currentBoss = nullptr;
	UE_MVVM_SET_PROPERTY_VALUE(bIsBossVisible, false);
}

void UCDStageVM::SetBossName(const FText& value)
{
	if (bossName.IdenticalTo(value))
	{
		return;
	}

	bossName = value;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bossName);
}

void UCDStageVM::SetStageName(const FText& value)
{
	if (stageName.IdenticalTo(value))
	{
		return;
	}

	stageName = value;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(stageName);
}

#undef LOCTEXT_NAMESPACE
