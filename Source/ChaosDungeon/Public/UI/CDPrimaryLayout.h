#pragma once

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "CDPrimaryLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

/**
 * 화면 레이어 루트. BP 에서 세 스택을 겹쳐 배치한다. (아래 → 위)
 *   UI.Layer.Game     : HUD
 *   UI.Layer.GameMenu : 일시정지, 조작법
 *   UI.Layer.Modal    : 결과, NPC 메뉴, 타이틀
 * BP 위젯 이름은 gameLayer / gameMenuLayer / modalLayer 로 맞춘다. (BindWidget)
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDPrimaryLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 레이어에 화면 추가 */
	UCommonActivatableWidget* PushToLayer(FGameplayTag layerTag, TSubclassOf<UCommonActivatableWidget> screenClass, TFunction<void(UCommonActivatableWidget&)> initFunc = nullptr);

	/** 해당 레이어 최상단이 screenClass 면 닫고 true */
	bool DeactivateIfActive(FGameplayTag layerTag, TSubclassOf<UCommonActivatableWidget> screenClass);

	/* 특정 레이어의 모든 위젯들을 비운다.*/
	UFUNCTION(BlueprintCallable)
	void ClearStack(FGameplayTag layerTag);

protected:
	// UUserWidget::NativeOnInitialized()
	virtual void NativeOnInitialized() override;

protected:
	/** HUD 레이어 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> gameLayer;

	/** 일시정지/조작법 레이어 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> gameMenuLayer;

	/** 결과/NPC/타이틀 레이어 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> modalLayer;

private:
	/** 태그 → 레이어 */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>> layers;
};
