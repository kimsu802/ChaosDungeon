#pragma once

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "CDSkillSlotWidget.generated.h"

class UCDSkillSlotVM;

/**
 * 스킬/회피 슬롯 View. 표시는 전부 MVVM 바인딩(BP), C++ 은 드래그 앤 드롭만 담당.
 * - 위젯 BP 의 Viewmodel 창에서 UCDSkillSlotVM 을 Creation Type = Manual 로 추가
 * - inputTag 만 인스턴스마다 지정하면 NativeConstruct 에서 해당 VM 을 넣어준다
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDSkillSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UUserWidget::NativeOnMouseButtonDown()
	virtual FReply NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent) override;

	// UUserWidget::NativeOnDragDetected()
	virtual void NativeOnDragDetected(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent, UDragDropOperation*& outOperation) override;

	// UUserWidget::NativeOnDrop()
	virtual bool NativeOnDrop(const FGeometry& inGeometry, const FDragDropEvent& inDragDropEvent, UDragDropOperation* inOperation) override;

protected:
	/** 이 슬롯의 입력 태그 (Input.Skill.1 ~ 4, Input.Dodge) */
	UPROPERTY(EditAnywhere, Category = "Slot", meta = (Categories = "Input"))
	FGameplayTag inputTag;

private:
	/** 이 슬롯의 ViewModel */
	UPROPERTY(Transient)
	TObjectPtr<UCDSkillSlotVM> slotViewModel;
};
