#pragma once

#include "GameFramework/GameModeBase.h"
#include "CDHubGameMode.generated.h"

/**
 * 타이틀 + 허브 (같은 레벨)
 * - 처음 진입: 캐릭터 전신 카메라 + 타이틀 화면 (UIManager 에 요청)
 * - '게임 시작': 쿼터뷰 카메라로 블렌드 → 허브 (NPC/기록 게시판은 ACDInteractable)
 * - 던전에서 돌아온 경우: 타이틀 생략
 */
UCLASS()
class CHAOSDUNGEON_API ACDHubGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 기본 클래스 설정 */
	ACDHubGameMode();

	/** 타이틀 → 허브 카메라 전환 */
	UFUNCTION(BlueprintCallable)
	void EnterHub(bool bSkipBlend = false);

protected:
	// AActor::BeginPlay()
	virtual void BeginPlay() override;

protected:
	/** 레벨에 배치한 타이틀용 CameraActor 의 Actor Tag */
	UPROPERTY(EditDefaultsOnly, Category = "Title")
	FName titleCameraTag = TEXT("TitleCamera");

	/** 타이틀 → 쿼터뷰 카메라 블렌드 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Title")
	float cameraBlendTime = 1.5f;
};
