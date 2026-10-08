#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "Core/CDMessageSubsystem.h"
#include "Core/CDTypes.h"
#include "CDUIManagerSubsystem.generated.h"

class APlayerController;
class UCommonActivatableWidget;
class UCDPrimaryLayout;
class UCDUIScreenSet;
struct FCDRunMessage;

/**
 * 화면을 띄우고 닫는 유일한 창구.
 * - 게임 코드는 "어떤 레이어에 어떤 화면"만 요청하고, 위젯 생성/입력 모드는 여기와 CommonUI 가 처리
 * - 결과 화면은 Run.Finished 메시지를 구독해서 스스로 띄운다
 *
 * DefaultGame.ini
 *   [/Script/ChaosDungeon.CDUIManagerSubsystem]
 *   screenSetAsset=/Game/UI/DA_UIScreenSet.DA_UIScreenSet
 */
UCLASS(Config = Game,BlueprintType)
class CHAOSDUNGEON_API UCDUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** 플레이어 컨트롤러로 서브시스템 얻기 */
	UFUNCTION(BlueprintCallable)
	static UCDUIManagerSubsystem* Get(const APlayerController* playerController);

	// USubsystem::Initialize()
	virtual void Initialize(FSubsystemCollectionBase& collection) override;

	// USubsystem::Deinitialize()
	virtual void Deinitialize() override;

	/** PlayerController::BeginPlay 에서 호출. 레벨마다 레이아웃 + HUD 를 새로 만든다 */
	void InitializeForPlayer(APlayerController* playerController);

	/** 레이어에 화면 추가 */
	UFUNCTION(BlueprintCallable)
	UCommonActivatableWidget* PushScreen(FGameplayTag layerTag, const TSoftClassPtr<UCommonActivatableWidget>& screenClass);

	void PushScreenAsync(FGameplayTag layerTag, const TSoftClassPtr<UCommonActivatableWidget>& screenClass, TFunction<void(UCommonActivatableWidget&)> initFunc);

	UFUNCTION(BlueprintCallable)
	void ClearLayerByTag(FGameplayTag layerTag);

	/** 일시정지 메뉴 열기/닫기 */
	void TogglePauseMenu();

	/** 조작법 안내 열기/닫기 */
	void ToggleGuide();

	/** 타이틀 화면 열기 */
	void ShowTitle();

	/** 게임 화면 열기 **/
	void ShowGameHud();

	/* 컨펌 화면 열기*/
	void ShowConfirm(ECDConfirmType type, const FText& title, const FText& message, TFunction<void(ECDConfirmResult)> onResult);

private:
	/** 레이아웃이 없으면 생성 (GameMode/PlayerController 중 누가 먼저 요청해도 동작) */
	UCDPrimaryLayout* EnsureLayout();

	/** 최상단이 같은 화면이면 닫고, 아니면 연다 */
	void ToggleScreen(FGameplayTag layerTag, const TSoftClassPtr<UCommonActivatableWidget>& screenClass);

	/** Run.Finished → 결과 화면 */
	void HandleRunFinished(const FCDRunMessage& message);

	/** 월드 정리(레벨 이동/종료): 그 월드의 레이아웃을 버린다. 남겨 두면 다음 레벨 HUD 가 화면에 없는 옛 레이아웃에 들어간다 */
	void HandleWorldCleanup(UWorld* world, bool bSessionEnded, bool bCleanupResources);

	/** 메시지 서브시스템 */
	UCDMessageSubsystem* GetMessages() const;

private:
	/** 로드된 화면 목록 */
	UPROPERTY(Transient)
	TObjectPtr<UCDUIScreenSet> screenSet;

	/** 현재 레벨의 루트 레이아웃 */
	UPROPERTY(Transient)
	TObjectPtr<UCDPrimaryLayout> layout;

	/** Run.Finished 구독 핸들 */
	FCDListenerHandle runFinishedHandle;

	/** FWorldDelegates::OnWorldCleanup 핸들 */
	FDelegateHandle worldCleanupHandle;
};
