#include "UI/CDSkillSlotWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
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

	// 시작 시 하이라이트는 숨김 (디자이너 설정과 무관하게 상태 기준으로 맞춘다)
	bIconHovered = false;
	RefreshHighlight();
}

void UCDSkillSlotWidget::NativeOnMouseEnter(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent)
{
	Super::NativeOnMouseEnter(inGeometry, inMouseEvent);

	// 슬롯에 들어왔어도 아이콘 영역 밖(키 라벨 탭 등)일 수 있으므로 위치로 판단
	UpdateIconHover(inMouseEvent);
}

FReply UCDSkillSlotWidget::NativeOnMouseMove(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent)
{
	UpdateIconHover(inMouseEvent);
	return Super::NativeOnMouseMove(inGeometry, inMouseEvent);
}

void UCDSkillSlotWidget::NativeOnMouseLeave(const FPointerEvent& inMouseEvent)
{
	Super::NativeOnMouseLeave(inMouseEvent);

	bIconHovered = false;
	RefreshHighlight();
}

FReply UCDSkillSlotWidget::NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent)
{
	if (inMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(inGeometry, inMouseEvent);
	}

	bDragStarted = false;

	// 스킬 슬롯: 드래그 감지 시작 (그대로 떼면 MouseButtonUp 에서 발동)
	if (inputTag.MatchesTag(CDTags::Input_Skill))
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(inMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	// 회피 등 드래그 불가 슬롯: 클릭만. 처리하지 않으면 뷰포트가 마우스를 가져가 MouseButtonUp 이 오지 않는다
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UCDSkillSlotWidget::NativeOnMouseButtonUp(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent)
{
	if (inMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(inGeometry, inMouseEvent);
	}

	// 슬롯 위에서 뗐고 드래그가 아니었으면 클릭 = 발동
	const bool bReleasedOnSlot = inGeometry.IsUnderLocation(inMouseEvent.GetScreenSpacePosition());
	if (!bDragStarted && bReleasedOnSlot && slotViewModel)
	{
		slotViewModel->Activate();
	}
	bDragStarted = false;

	// 마우스 캡처 중에는 Leave 가 오지 않을 수 있으므로 뗀 위치 기준으로 호버를 다시 판단
	UpdateIconHover(inMouseEvent);
	return FReply::Handled().ReleaseMouseCapture();
}

void UCDSkillSlotWidget::NativeOnDragDetected(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent, UDragDropOperation*& outOperation)
{
	bDragStarted = true;

	// 드래그 중에는 마우스 이동/이탈 이벤트 대신 드래그 이벤트가 오므로, 원래 슬롯의 하이라이트를 미리 끈다
	bIconHovered = false;
	RefreshHighlight();

	// 빈 슬롯은 드래그하지 않는다
	if (!slotViewModel || !slotViewModel->GetIcon())
	{
		return;
	}

	outOperation = NewObject<UDragDropOperation>(this);
	outOperation->Payload = slotViewModel;
	outOperation->DefaultDragVisual = CreateDragVisual();
	// 아이콘 중앙이 커서에 오도록
	outOperation->Pivot = EDragPivot::CenterCenter;
}

UWidget* UCDSkillSlotWidget::CreateDragVisual() const
{
	if (!iconImage)
	{
		return nullptr;
	}

	// 슬롯 아이콘과 같은 브러시(머티리얼 포함)를 같은 크기로 복사한 이미지가 커서를 따라다닌다
	UImage* dragImage = NewObject<UImage>(const_cast<UCDSkillSlotWidget*>(this));
	FSlateBrush brush = iconImage->GetBrush();
	const FVector2D iconSize = iconImage->GetCachedGeometry().GetLocalSize();
	if (!iconSize.IsNearlyZero())
	{
		brush.ImageSize = iconSize;
	}
	dragImage->SetBrush(brush);
	dragImage->SetRenderOpacity(dragVisualOpacity);
	// 드래그 비주얼이 드롭 대상 히트 테스트를 가리지 않도록
	dragImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	return dragImage;
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

void UCDSkillSlotWidget::SetIconTexture(UTexture2D* texture)
{
	bHasIcon = (texture != nullptr);

	// 아이콘이 생기거나 사라지면 하이라이트 표시 조건도 바뀐다 (스킬 교체 직후 등)
	RefreshHighlight();

	if (!iconImage)
	{
		return;
	}

	// 빈 슬롯: 아이콘을 숨긴다. (Visibility 대신 투명도를 써야 빈 슬롯도 드롭 히트 테스트를 받는다)
	iconImage->SetRenderOpacity(texture ? 1.f : 0.f);
	if (!texture)
	{
		return;
	}

	iconImage->SetBrushFromTexture(texture);
}

void UCDSkillSlotWidget::SetCooldownPercent(float percent)
{
	if (UMaterialInstanceDynamic* material = coolDownImage ? coolDownImage->GetDynamicMaterial() : nullptr)
	{
		material->SetScalarParameterValue(cooldownParameterName, percent);
	}
}

void UCDSkillSlotWidget::UpdateIconHover(const FPointerEvent& inMouseEvent)
{
	// 아이콘 위젯의 실제 화면 영역 안에 커서가 있는지로 판단 (슬롯 프레임/키 라벨 위는 제외)
	const bool bNowHovered = iconImage && iconImage->GetCachedGeometry().IsUnderLocation(inMouseEvent.GetScreenSpacePosition());
	if (bNowHovered == bIconHovered)
	{
		return;
	}

	bIconHovered = bNowHovered;
	RefreshHighlight();
}

void UCDSkillSlotWidget::RefreshHighlight()
{
	if (!highlightImage)
	{
		return;
	}

	// 빈 슬롯은 누를 게 없으므로 하이라이트하지 않는다
	const bool bShow = bIconHovered && bHasIcon;

	// 하이라이트가 아이콘 위에 겹쳐도 클릭/드래그/드롭을 가로채지 않도록 HitTestInvisible
	highlightImage->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
