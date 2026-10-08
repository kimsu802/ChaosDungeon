#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "CDLevelTransitionSubsystem.generated.h"

class SWidget;
class SBorder;
class UUserWidget;

/**
 * 화면 페이드 + 레벨 전환 연출 전담: 페이드 아웃 → 로딩 화면 → 레벨 로드 → 페이드 인
 * - 페이드는 UI 까지 덮도록 플레이어 화면 최상단(ZOrder)에 오버레이를 붙여서 처리한다.
 *   (카메라 페이드/포스트프로세스는 월드만 어둡게 하고 UMG 는 덮지 못함)
 * - CommonUI 레이어를 쓰지 않으므로 입력 모드·포커스·화면 스택에 영향이 없고, 레벨이 바뀌어도 끊기지 않는다.
 * - 오버레이 모양: Project Settings > ChaosDungeon General Settings > fadeWidgetClass
 *   비어 있으면 검은 화면, 지정하면 그 위젯(로고/팁 등)을 RenderOpacity 로 페이드한다.
 * - 레벨 이동 외 연출(사망, 컷 전환)은 FadeScreen 또는 BP "Fade Screen" 비동기 노드 사용
 */
UCLASS()
class CHAOSDUNGEON_API UCDLevelTransitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem::Initialize()
	virtual void Initialize(FSubsystemCollectionBase& collection) override;

	// USubsystem::Deinitialize()
	virtual void Deinitialize() override;

	/** 연출과 함께 레벨 이동 */
	void TravelTo(const TSoftObjectPtr<UWorld>& level);

	/**
	 * 화면 전체(UI 포함) 페이드. targetAlpha 1 = 가림, 0 = 걷힘
	 * 레벨 이동 중이면 무시하고 false. 끝나면 onFinished 호출 (다른 페이드에 끊겨도 호출된다)
	 */
	bool FadeScreen(float targetAlpha, TFunction<void()> onFinished = nullptr);

	/** 페이드/레벨 이동 중인가 */
	FORCEINLINE bool IsFading() const
	{
		return fadeTickHandle.IsValid();
	}

	/** 현재 오버레이 알파 (0 = 안 보임) */
	FORCEINLINE float GetFadeAlpha() const
	{
		return currentAlpha;
	}

private:
	/** 페이드 아웃 후 실제 레벨 열기 */
	void OpenPendingLevel();

	/** 맵 로드 직전: 로딩 화면 표시 */
	void HandlePreLoadMap(const FString& mapName);

	/** 맵 로드 직후: 페이드 인 */
	void HandlePostLoadMap(UWorld* loadedWorld);

	/** 현재 알파에서 targetAlpha 까지 fadeDuration 동안 페이드. 끝나면 onFinished 호출 */
	void StartFade(float targetAlpha, TFunction<void()> onFinished);

	/** 페이드 진행 (코어 티커, 일시정지 중에도 동작). false 를 반환하면 티커 해제 */
	bool TickFade(float deltaTime);

	/** 오버레이 생성 (fadeWidgetClass 가 있으면 그 위젯, 없으면 검은 SBorder) */
	TSharedPtr<SWidget> GetOrCreateOverlay();

	/** 오버레이를 플레이어 화면 최상단에 붙이고 알파 지정 (레벨 로드 시 뷰포트가 비워지므로 매번 다시 붙인다) */
	void ShowOverlay(float alpha);

	/** 오버레이를 화면에서 뗀다 */
	void HideOverlay();

	/** 오버레이 알파 적용 */
	void SetOverlayAlpha(float alpha);

private:
	/** 로딩 중 표시되는 위젯 (GC 방지) */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> loadingScreen;

	/** 페이드 오버레이 위젯 (fadeWidgetClass 지정 시, GC 방지) */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> fadeWidget;

	/** 이동할 레벨 */
	TSoftObjectPtr<UWorld> pendingLevel;

	/** 이동 중 중복 요청 방지 */
	bool bTraveling = false;

	/** 기본 오버레이 (검은 화면) */
	TSharedPtr<SBorder> blackOverlay;

	/** 실제로 뷰포트에 붙는 오버레이 (fadeWidget 의 Slate 또는 blackOverlay) */
	TSharedPtr<SWidget> overlay;

	/** 현재 오버레이 알파 */
	float currentAlpha = 0.f;

	/** 이번 페이드의 시작 알파 */
	float fadeStartAlpha = 0.f;

	/** 이번 페이드의 목표 알파 */
	float fadeTargetAlpha = 0.f;

	/** 이번 페이드 경과 시간 */
	float fadeElapsed = 0.f;

	/** 페이드 완료 콜백 */
	TFunction<void()> fadeFinished;

	/** 페이드 티커 핸들 */
	FTSTicker::FDelegateHandle fadeTickHandle;
};
