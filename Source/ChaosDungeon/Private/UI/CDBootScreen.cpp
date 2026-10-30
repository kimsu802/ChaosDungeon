#include "UI/CDBootScreen.h"
#include "Animation/WidgetAnimation.h"

UCDBootScreen::UCDBootScreen(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	SetIsFocusable(true);
}

void UCDBootScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (sceneNotice != nullptr)
	{
		noticeVisibility = sceneNotice->GetVisibility();
	}
}

void UCDBootScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 풀에서 다시 꺼내 쓰는 경우에 대비해 상태 초기화
	bFinished = false;
	SetNoticeSceneVisible(true);
	PlayAnimation(sequenceAnim);
}

UWidget* UCDBootScreen::NativeGetDesiredFocusTarget() const
{
	// 자기 자신이 포커스를 받아 키 입력을 처리
	return const_cast<UCDBootScreen*>(this);
}

FReply UCDBootScreen::NativeOnKeyDown(const FGeometry& inGeometry, const FKeyEvent& inKeyEvent)
{
	HandleSkip();
	return FReply::Handled();
}

FReply UCDBootScreen::NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent)
{
	HandleSkip();
	return FReply::Handled();
}

void UCDBootScreen::OnAnimationFinished_Implementation(const UWidgetAnimation* animation)
{
	Super::OnAnimationFinished_Implementation(animation);

	if (animation == sequenceAnim)
	{
		Finish();
	}
}

void UCDBootScreen::HandleSkip()
{
	if ((bAllowSkip == false) || bFinished)
	{
		return;
	}

	const float current = GetAnimationCurrentTime(sequenceAnim);
	if (current < minSkipTime)
	{
		return;
	}

	// ① 주의 안내 중 → ② 시작 지점으로
	if (current < secondSceneTime)
	{
		// 점프는 사이 구간을 평가하지 않는다 → ① 의 페이드 아웃 키를 건너뛰어 불투명한 채로 남을 수 있음
		// Visibility 는 애니메이션이 건드리지 않으므로 여기서 숨기면 끝까지 유지된다
		SetNoticeSceneVisible(false);
		SetAnimationCurrentTime(sequenceAnim, secondSceneTime);
		return;
	}

	// ② 중 → 종료
	Finish();
}

void UCDBootScreen::Finish()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;
	bootFinished.Broadcast();
	DeactivateWidget();
}

void UCDBootScreen::SetNoticeSceneVisible(bool bVisible)
{
	if (sceneNotice == nullptr)
	{
		return;
	}

	sceneNotice->SetVisibility(bVisible ? noticeVisibility : ESlateVisibility::Collapsed);
}
