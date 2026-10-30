#include "UI/CDBootScreen.h"
#include "Animation/WidgetAnimation.h"

UCDBootScreen::UCDBootScreen(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	SetIsFocusable(true);
}

void UCDBootScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 풀에서 다시 꺼내 쓰는 경우에 대비해 상태 초기화
	bFinished = false;
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
