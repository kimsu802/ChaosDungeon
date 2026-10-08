#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "CDAsyncAction_FadeScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCDFadeFinishedDelegate);

/**
 * BP 용 "Fade Screen" 비동기 노드. 화면 전체(UI 포함)를 페이드하고, 끝나면 onFinished 핀 실행.
 * - Target Alpha 1 = 가림, 0 = 걷힘
 * - 레벨 이동 중이라 실행할 수 없으면 바로 onFinished
 * C++ 에서는 UCDLevelTransitionSubsystem::FadeScreen 을 직접 호출한다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDAsyncAction_FadeScreen : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** 화면 페이드 */
	UFUNCTION(BlueprintCallable, Category = "UI", meta = (WorldContext = "worldContextObject", HidePin = "worldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Fade Screen"))
	static UCDAsyncAction_FadeScreen* FadeScreen(const UObject* worldContextObject, float targetAlpha = 1.f);

	// UBlueprintAsyncActionBase::Activate()
	virtual void Activate() override;

private:
	/** 완료 알림 + 노드 정리 */
	void Finish();

public:
	/** 페이드가 끝났을 때 */
	UPROPERTY(BlueprintAssignable)
	FCDFadeFinishedDelegate onFinished;

private:
	/** 노드를 호출한 월드 */
	TWeakObjectPtr<UWorld> cachedWorld;

	/** 목표 알파 */
	float cachedTargetAlpha = 1.f;

	/** 중복 완료 방지 */
	bool bFinished = false;
};
