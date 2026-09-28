#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "CDLevelTransitionSubsystem.generated.h"

class UUserWidget;

/**
 * 레벨 전환 연출 전담: 페이드 아웃 → 로딩 화면 → 레벨 로드 → 페이드 인
 * 허브→던전, 스테이지→스테이지, 결과→허브 모두 이 경로를 쓴다.
 *
 * DefaultGame.ini
 *   [/Script/ChaosDungeon.CDLevelTransitionSubsystem]
 *   loadingScreenClass=/Game/UI/WBP_Loading.WBP_Loading_C
 */
UCLASS(Config = Game)
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

private:
	/** 페이드 아웃 후 실제 레벨 열기 */
	void OpenPendingLevel();

	/** 맵 로드 직전: 로딩 화면 표시 */
	void HandlePreLoadMap(const FString& mapName);

	/** 맵 로드 직후: 페이드 인 */
	void HandlePostLoadMap(UWorld* loadedWorld);

	/** 카메라 페이드 */
	void StartFade(float from, float to) const;

private:
	/** 로딩 화면 위젯 클래스 (ini) */
	UPROPERTY(Config)
	TSoftClassPtr<UUserWidget> loadingScreenClass;

	/** 페이드 시간 (ini) */
	UPROPERTY(Config)
	float fadeDuration = 0.5f;

	/** 로딩 중 표시되는 위젯 (GC 방지) */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> loadingScreen;

	/** 이동할 레벨 */
	TSoftObjectPtr<UWorld> pendingLevel;

	/** 이동 중 중복 요청 방지 */
	bool bTraveling = false;
};
