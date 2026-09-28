#include "UI/CDSkillSlotWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Core/CDGameplayTags.h"
#include "Engine/LocalPlayer.h"
#include "MVVMSubsystem.h"
#include "View/MVVMView.h"
#include "UI/ViewModel/CDSkillSlotVM.h"
#include "UI/ViewModel/CDViewModelSubsystem.h"

void UCDSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const ULocalPlayer* localPlayer = GetOwningLocalPlayer();
	const UCDViewModelSubsystem* viewModels = localPlayer ? localPlayer->GetSubsystem<UCDViewModelSubsystem>() : nullptr;
	slotViewModel = viewModels ? viewModels->GetSkillSlot(inputTag) : nullptr;

	if (UMVVMView* view = UMVVMSubsystem::GetViewFromUserWidget(this))
	{
		view->SetViewModelByClass(slotViewModel);
	}
}

FReply UCDSkillSlotWidget::NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent)
{
	if (inputTag.MatchesTag(CDTags::Input_Skill) && inMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(inMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnMouseButtonDown(inGeometry, inMouseEvent);
}

void UCDSkillSlotWidget::NativeOnDragDetected(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent, UDragDropOperation*& outOperation)
{
	outOperation = NewObject<UDragDropOperation>(this);
	outOperation->Payload = slotViewModel;
	// 드래그 비주얼이 필요하면 BP 에서 OnDragDetected 를 재정의해 DefaultDragVisual 지정
}

bool UCDSkillSlotWidget::NativeOnDrop(const FGeometry& inGeometry, const FDragDropEvent& inDragDropEvent, UDragDropOperation* inOperation)
{
	UCDSkillSlotVM* from = inOperation ? Cast<UCDSkillSlotVM>(inOperation->Payload) : nullptr;
	if (!from || !slotViewModel)
	{
		return false;
	}
	// 교체 가능 여부 판단은 VM 책임
	from->SwapWith(slotViewModel);
	return true;
}
