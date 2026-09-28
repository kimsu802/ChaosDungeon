#pragma once

#include "MVVMViewModelBase.h"
#include "Core/CDMessageSubsystem.h"
#include "Core/CDTypes.h"
#include "CDRunResultVM.generated.h"

struct FCDRunMessage;

/**
 * 결과 화면. Run.Finished 메시지 한 번으로 모든 값이 채워진다.
 * View 에서는 record.kills 처럼 구조체 멤버 경로로 바인딩
 */
UCLASS()
class CHAOSDUNGEON_API UCDRunResultVM : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** 메시지 구독 시작 */
	void StartListening(UCDMessageSubsystem& messages);

	/** 메시지 구독 해제 */
	void StopListening(UCDMessageSubsystem& messages);

	/** 런 기록 */
	FCDRunRecord GetRecord() const
	{
		return record;
	}

	/** 최고 기록 갱신 여부 */
	bool GetIsNewRecord() const
	{
		return bIsNewRecord;
	}

private:
	/** Run.Finished */
	void HandleRunFinished(const FCDRunMessage& message);

private:
	/** 런 기록 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetRecord, meta = (AllowPrivateAccess = true))
	FCDRunRecord record;

	/** 최고 기록 갱신 여부 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = GetIsNewRecord, meta = (AllowPrivateAccess = true))
	bool bIsNewRecord = false;

	/** Run.Finished 구독 핸들 */
	FCDListenerHandle finishedHandle;
};
