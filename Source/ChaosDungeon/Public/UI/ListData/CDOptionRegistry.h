#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CDOptionRegistry.generated.h"

class APlayerController;
class UCDOptionData;

/** 옵션 탭 1개 (탭 버튼 + 리스트에 들어갈 항목들) */
USTRUCT()
struct CHAOSDUNGEON_API FCDOptionTab
{
	GENERATED_BODY()

	/** 탭 ID (탭 리스트 등록용) */
	UPROPERTY()
	FName tabId;

	/** 탭 이름 */
	UPROPERTY()
	FText displayName;

	/** 탭 항목 (리스트 뷰 아이템) */
	UPROPERTY()
	TArray<TObjectPtr<UCDOptionData>> options;
};

/**
 * 옵션 화면에 들어갈 항목을 전부 만들고 실제 설정과 연결한다. 옵션 화면이 처음 열릴 때 1회 생성.
 * - 그래픽: UGameUserSettings (화면 모드 · 해상도 · 전체 품질 → 개별 품질 · 수직 동기화 · 최대 프레임)
 * - 사운드: UCDGameUserSettings (전체 · 효과음 · 배경음)
 * - 조작:   UCDKeyBindingLibrary (UCDInputConfig 의 abilityInputs 중 키 변경이 설정된 입력)
 * 항목을 추가하려면 Build*Tab 에 AddOption 한 덩어리만 추가하면 된다 (위젯 수정 없음).
 */
UCLASS()
class CHAOSDUNGEON_API UCDOptionRegistry : public UObject
{
	GENERATED_BODY()

public:
	/** 탭 · 항목 생성 */
	void Build(APlayerController* playerController);

	/** 전체 탭 */
	const TArray<FCDOptionTab>& GetTabs() const
	{
		return tabs;
	}

	/** ID 로 탭 찾기 (없으면 nullptr) */
	const FCDOptionTab* FindTab(FName tabId) const;

	/** 탭의 모든 항목을 기본값으로 */
	void ResetTab(FName tabId);

	/** 모든 항목 표시 갱신 (다시 열었을 때, 다른 곳에서 값이 바뀌었을 수 있으므로) */
	void RefreshAll();

private:
	/** 그래픽 탭 */
	void BuildGraphicsTab();

	/** 사운드 탭 */
	void BuildSoundTab();

	/** 조작(키 설정) 탭 */
	void BuildControlsTab(APlayerController* playerController);

	/** 항목 생성 + 이름 · 설명 + 탭에 추가 */
	template <typename TOption>
	TOption* AddOption(FCDOptionTab& tab, const FText& displayName, const FText& description);

private:
	/** 탭 목록 (항목의 수명도 여기서 유지) */
	UPROPERTY()
	TArray<FCDOptionTab> tabs;
};
