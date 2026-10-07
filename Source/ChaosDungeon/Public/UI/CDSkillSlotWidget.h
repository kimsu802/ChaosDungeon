#pragma once

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "CDSkillSlotWidget.generated.h"

class UCDSkillSlotVM;
class UImage;
class UTexture2D;
class UWidget;

/**
 * 스킬/회피 슬롯 View. 표시는 MVVM 바인딩(BP), C++ 은 입력(클릭/드래그 앤 드롭)과 아이콘 머티리얼 갱신을 담당.
 * - 위젯 BP 의 Viewmodel 창에서 UCDSkillSlotVM 을 Creation Type = Manual 로 추가
 * - inputTag 만 인스턴스마다 지정하면 NativeConstruct 에서 해당 VM 을 넣어준다
 * - 클릭(드래그 없이 떼기) = 스킬 발동, 드래그 후 다른 슬롯에 드롭 = 스킬 교체 (드래그 중 아이콘이 커서를 따라감)
 * - 빈 슬롯(Icon = null)은 아이콘을 투명하게 숨기고 드래그를 시작하지 않는다
 * - MVVM 바인딩: VM.Icon → SetIconTexture, VM.CooldownPercent → SetCooldownPercent, VM.CooldownText → 텍스트
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDSkillSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** MVVM 바인딩 대상: 아이콘 머티리얼의 텍스처 지정 (null 이면 머티리얼 기본 텍스처 유지) */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	void SetIconTexture(UTexture2D* texture);

	/** MVVM 바인딩 대상: 아이콘 머티리얼의 쿨다운 비율 지정 (1 = 막 사용, 0 = 사용 가능) */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	void SetCooldownPercent(float percent);

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UUserWidget::NativeOnMouseButtonDown()
	virtual FReply NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent) override;

	// UUserWidget::NativeOnMouseButtonUp()
	virtual FReply NativeOnMouseButtonUp(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent) override;

	// UUserWidget::NativeOnDragDetected()
	virtual void NativeOnDragDetected(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent, UDragDropOperation*& outOperation) override;

	// UUserWidget::NativeOnDrop()
	virtual bool NativeOnDrop(const FGeometry& inGeometry, const FDragDropEvent& inDragDropEvent, UDragDropOperation* inOperation) override;

private:
	/** 드래그 중 커서를 따라다닐 아이콘 이미지 생성 */
	UWidget* CreateDragVisual() const;

protected:
	/** 이 슬롯의 입력 태그 (Input.Skill.1 ~ 4, Input.Dodge) */
	UPROPERTY(EditAnywhere, Category = "Slot", meta = (Categories = "Input"))
	FGameplayTag inputTag;

	/** 스킬 아이콘 (브러시에 M_SkillSlotIcon 계열 머티리얼 지정) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> iconImage;

	/** 아이콘 머티리얼의 텍스처 파라미터 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Slot")
	FName iconParameterName = TEXT("Icon");

	/** 아이콘 머티리얼의 쿨다운 파라미터 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Slot")
	FName cooldownParameterName = TEXT("CooldownPercent");

	/** 드래그 중 커서를 따라다니는 아이콘의 투명도 */
	UPROPERTY(EditDefaultsOnly, Category = "Slot", meta = (ClampMin = 0, ClampMax = 1))
	float dragVisualOpacity = 0.8f;

private:
	/** 이 슬롯의 ViewModel */
	UPROPERTY(Transient)
	TObjectPtr<UCDSkillSlotVM> slotViewModel;

	/** 이번 누름에서 드래그가 시작됐는가 (드래그였으면 떼어도 발동하지 않음) */
	bool bDragStarted = false;
};
