#include "UI/ViewModel/CDRunResultVM.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"

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
}
