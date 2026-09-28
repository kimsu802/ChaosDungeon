#pragma once

#include "CommonActivatableWidget.h"
#include "CDActivatableScreen.generated.h"

/** 화면이 활성화됐을 때의 입력 모드 */
UENUM(BlueprintType)
enum class ECDScreenInputMode : uint8
{
	Game,  // HUD: 게임 입력 그대로, 커서 보임
	Menu,  // 메뉴: UI 입력만
	All    // 둘 다 (NPC 대화 중 이동 허용 등)
};

/**
 * 모든 화면의 공통 베이스. 화면별 차이는 값(입력 모드, 일시정지)만 다르다.
 * BP 자식: HUD, Pause, Guide, Result, NpcMenu, Title
 * 닫기(Esc/Back)는 CommonUI 의 bIsBackHandler 로 처리
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDActivatableScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// UCommonActivatableWidget::GetDesiredInputConfig()
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	// UCommonActivatableWidget::NativeOnActivated()
	virtual void NativeOnActivated() override;

	// UCommonActivatableWidget::NativeOnDeactivated()
	virtual void NativeOnDeactivated() override;

protected:
	/** 활성화 시 입력 모드 */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	ECDScreenInputMode inputMode = ECDScreenInputMode::Menu;

	/** 활성화 동안 게임 일시정지 */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	bool bPauseGame = false;
};
