#pragma once

#include "GameFramework/GameModeBase.h"
#include "CDHubGameMode.generated.h"

class UCDCharacterClassData;

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

	/**
	 * 직업 교체: 선택 저장 → 기존 폰 삭제 → 같은 자리에 새 폰 스폰 · 빙의
	 * 새 폰은 빙의될 때 UCDPlayerClassSubsystem 에서 직업을 읽는다. 보고 있던 카메라(타이틀 카메라)는 유지
	 */
	UFUNCTION(BlueprintCallable)
	void RespawnPlayerAsClass(APlayerController* playerController, UCDCharacterClassData* newClass);

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
