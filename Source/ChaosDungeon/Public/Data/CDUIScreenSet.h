#pragma once

#include "Engine/DataAsset.h"
#include "CDUIScreenSet.generated.h"

class UCommonActivatableWidget;
class UCDPrimaryLayout;

/**
 * 화면 클래스 목록. 코드가 BP 화면을 직접 참조하지 않도록 UIManager 는 이 에셋만 본다.
 * 화면 추가 = 여기 필드 추가 + BP 화면 제작
 */
UCLASS(BlueprintType)
class CHAOSDUNGEON_API UCDUIScreenSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 루트 레이아웃 */
	UPROPERTY(EditDefaultsOnly, Category = "Layout")
	TSoftClassPtr<UCDPrimaryLayout> layoutClass;

	/** 인게임 HUD */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<UCommonActivatableWidget> hudScreen;

	/** 일시정지 메뉴 */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<UCommonActivatableWidget> pauseScreen;

	/** 옵션 (일시정지 메뉴에서 열림. 비어 있으면 옵션 버튼 비활성) */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<UCommonActivatableWidget> optionsScreen;

	/** 조작법/목표 안내 */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<UCommonActivatableWidget> guideScreen;

	/** 결과 화면 */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<UCommonActivatableWidget> resultScreen;

	/** 타이틀 화면 */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<UCommonActivatableWidget> titleScreen;

	/** 부트 화면 (실행 후 첫 타이틀 전에 한 번). 비워 두면 생략 */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<UCommonActivatableWidget> bootScreen;

	/** 에디터(PIE)에서는 부트 화면 생략 (패키지 빌드는 항상 재생) */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	bool bSkipBootInEditor = true;

	/** 컨펌 화면 */
	UPROPERTY(EditDefaultsOnly, Category = "Screen")
	TSoftClassPtr<UCommonActivatableWidget> confirmScreen;
};
