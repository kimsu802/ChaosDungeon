#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Core/CDTypes.h"
#include "CDDifficultyCard.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

/**
 * 난이도 선택 카드 한 장 (쉬움 / 보통 / 어려움).
 * WBP_DifficultyCard 하나를 만들고, 화면에 3개 배치한 뒤 인스턴스마다 difficulty 만 바꾼다.
 * 이름 · 엠블럼 · 색은 difficulty 로 정해지고, 배율 · 부활 횟수 · 최고 기록은 DA_Dungeon / RunSubsystem 에서 읽는다.
 * 선택(Selected) 상태는 UCDDifficultySelectScreen 이 관리한다.
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDDifficultyCard : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	// 이 카드의 난이도
	FORCEINLINE ECDDifficulty GetDifficulty() const
	{
		return difficulty;
	}

	// 화면 하단 설명 문구
	FORCEINLINE const FText& GetDescription() const
	{
		return description;
	}

protected:
	// UUserWidget::NativePreConstruct()
	virtual void NativePreConstruct() override;

	// UCommonButtonBase::NativeConstruct() ← UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UCommonButtonBase::NativeOnHovered()
	virtual void NativeOnHovered() override;

	// UCommonButtonBase::NativeOnUnhovered()
	virtual void NativeOnUnhovered() override;

	// UCommonButtonBase::NativeOnSelected()
	virtual void NativeOnSelected(bool bBroadcast) override;

	// UCommonButtonBase::NativeOnDeselected()
	virtual void NativeOnDeselected(bool bBroadcast) override;

private:
	// 이름 · 엠블럼 · 색 적용 (디자이너 미리보기에서도 호출)
	void ApplyAppearance();

	// 배율 · 부활 횟수 · 최고 기록 텍스트 채우기
	void ApplyInfo();

	// 호버 · 선택 상태에 맞춰 테두리와 글로우 갱신
	void RefreshHighlight();

protected:
	// 이 카드의 난이도 (인스턴스마다 Details 에서 지정)
	UPROPERTY(EditAnywhere, Category = "Difficulty")
	ECDDifficulty difficulty = ECDDifficulty::Normal;

	// 엠블럼 텍스처 (T_DiffEmblem_Easy / Normal / Hard)
	UPROPERTY(EditAnywhere, Category = "Difficulty")
	TObjectPtr<UTexture2D> emblemTexture;

	// 영문 표기 (EASY / NORMAL / HARD)
	UPROPERTY(EditAnywhere, Category = "Difficulty")
	FText englishName;

	// 선택 시 화면 하단에 표시할 설명
	UPROPERTY(EditAnywhere, Category = "Difficulty", meta = (MultiLine = true))
	FText description;

	// 호버만 됐을 때 테두리 · 글로우 투명도
	UPROPERTY(EditAnywhere, Category = "Difficulty")
	float hoverHighlightOpacity = 0.45f;

	// 이름 "보통"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> nameText;

	// 엠블럼
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> emblemImage;

	// 영문 "NORMAL"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> englishText;

	// 몬스터 체력 배율 "×1.0"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> healthText;

	// 몬스터 공격력 배율 "×1.0"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> attackText;

	// 부활 가능 횟수 "3회" / "무제한"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> reviveText;

	// 최고 기록 "05:32" / "기록 없음"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> bestRecordText;

	// 카드 위쪽 색 번짐 (난이도 색으로 틴트)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> headerWash;

	// 선택 테두리 (난이도 색으로 틴트)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> selectBorder;

	// 바깥 글로우 (난이도 색으로 틴트)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> outerGlow;

private:
	// 현재 마우스가 올라가 있는가
	bool bHovered = false;
};
