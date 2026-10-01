#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "Core/CDTypes.h"
#include "CDAsyncAction_ShowConfirm.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCDConfirmResultDelegate, ECDConfirmResult, result);

/**
 * BP 용 "Show Confirm" 비동기 노드. 확인창을 띄우고, 닫히면 onResult 핀으로 결과를 내보낸다.
 * C++ 에서는 이 노드 대신 UCDUIManagerSubsystem::ShowConfirm 을 직접 호출한다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDAsyncAction_ShowConfirm : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** 확인창을 띄우고 결과를 기다린다 */
	UFUNCTION(BlueprintCallable, Category = "UI", meta = (WorldContext = "worldContextObject", HidePin = "worldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Show Confirm"))
	static UCDAsyncAction_ShowConfirm* ShowConfirm(const UObject* worldContextObject, ECDConfirmType type, FText title, FText message);

	// UBlueprintAsyncActionBase::Activate()
	virtual void Activate() override;

private:
	/** 결과를 내보내고 노드 정리 */
	void Finish(ECDConfirmResult result);

public:
	/** 확인창이 닫혔을 때 (결과 포함) */
	UPROPERTY(BlueprintAssignable)
	FCDConfirmResultDelegate onResult;

private:
	/** 노드를 호출한 월드 */
	TWeakObjectPtr<UWorld> cachedWorld;

	/** 버튼 구성 */
	ECDConfirmType cachedType = ECDConfirmType::Ok;

	/** 제목 */
	FText cachedTitle;

	/** 본문 */
	FText cachedMessage;
};
