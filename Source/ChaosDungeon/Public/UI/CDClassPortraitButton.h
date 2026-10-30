#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CDClassPortraitButton.generated.h"

class UImage;
class UTextBlock;
class UCDCharacterClassData;

/**
 * 직업 선택 화면의 초상화 버튼 한 개.
 * WBP_ClassPortraitButton 하나를 만들고, 화면에 배치한 인스턴스마다 classData 만 지정한다.
 * 클릭 → ACDHubGameMode::RespawnPlayerAsClass → 보이는 캐릭터가 그 직업으로 교체 (스탯 · 스킬 · 메시 · 애님 일괄)
 * 현재 직업 표시(selectedMark)는 UCDPlayerClassSubsystem 의 변경 알림으로 모든 버튼이 각자 갱신한다.
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDClassPortraitButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	/** 이 버튼의 직업 */
	UFUNCTION(BlueprintPure, Category = "Class")
	UCDCharacterClassData* GetClassData() const
	{
		return classData;
	}

protected:
	// UUserWidget::NativePreConstruct()
	virtual void NativePreConstruct() override;

	// UCommonButtonBase::NativeConstruct() ← UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UCommonButtonBase::NativeDestruct() ← UUserWidget::NativeDestruct()
	virtual void NativeDestruct() override;

	// UCommonButtonBase::NativeOnClicked()
	virtual void NativeOnClicked() override;

private:
	/** 초상화 · 이름 적용 (디자이너 미리보기에서도 호출) */
	void ApplyAppearance();

	/** 직업 변경 알림 */
	void HandleClassChanged(const UCDCharacterClassData* currentClass);

	/** 현재 직업이면 selectedMark 표시 */
	void RefreshSelected(const UCDCharacterClassData* currentClass);

protected:
	/** 이 버튼의 직업 데이터 (인스턴스마다 Details 에서 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	TObjectPtr<UCDCharacterClassData> classData;

	/** 초상화 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> portraitImage;

	/** 직업 이름 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> nameText;

	/** 현재 직업 표시 (테두리 · 체크 등) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> selectedMark;
};
