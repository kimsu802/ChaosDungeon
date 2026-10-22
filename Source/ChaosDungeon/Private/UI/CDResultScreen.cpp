#include "UI/CDResultScreen.h"
#include "CommonButtonBase.h"
#include "Animation/WidgetAnimation.h"
#include "Core/CDRunSubsystem.h"
#include "Engine/GameInstance.h"

void UCDResultScreen::SetSuccess(bool bSuccess)
{
	if (clearTitle)
	{
		clearTitle->SetVisibility(bSuccess ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (failTitle)
	{
		failTitle->SetVisibility(bSuccess ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UCDResultScreen::SetNewRecord(bool bNewRecord)
{
	bShowNewRecord = bNewRecord;

	// 배지는 introAnim 이 끝난 뒤 newRecordAnim 으로 등장. 애니메이션이 없으면 바로 표시
	if (newRecordBadge)
	{
		const bool bShowNow = bNewRecord && (newRecordAnim == nullptr);
		newRecordBadge->SetVisibility(bShowNow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UCDResultScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	returnButton->OnClicked().AddUObject(this, &ThisClass::HandleReturnClicked);

	if (introAnim)
	{
		FWidgetAnimationDynamicEvent finishedEvent;
		finishedEvent.BindDynamic(this, &ThisClass::HandleIntroFinished);
		BindToAnimationFinished(introAnim, finishedEvent);
	}
}

void UCDResultScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	bReturning = false;

	if (introAnim == nullptr)
	{
		HandleIntroFinished();
		return;
	}

	// 연출 중에는 클릭을 무시한다 (보스 처치 직후 공격 키 연타로 바로 넘어가지 않도록)
	// SetIsInteractionEnabled(false) 를 쓰면 WBP_MenuButton 이 비활성 프레임으로 바뀌어 보이므로 플래그로 막음
	bInputLocked = true;
	PlayAnimation(introAnim);
}

UWidget* UCDResultScreen::NativeGetDesiredFocusTarget() const
{
	return returnButton;
}

void UCDResultScreen::HandleIntroFinished()
{
	bInputLocked = false;
	returnButton->SetFocus();

	if (bShowNewRecord && newRecordBadge && newRecordAnim)
	{
		newRecordBadge->SetVisibility(ESlateVisibility::HitTestInvisible);
		PlayAnimation(newRecordAnim);
	}
}

void UCDResultScreen::HandleReturnClicked()
{
	if (bInputLocked || bReturning)
	{
		return;
	}
	bReturning = true;

	// 런 종료 후 허브(입구) 레벨로. 페이드 · 로딩은 UCDLevelTransitionSubsystem 이 처리
	if (UCDRunSubsystem* run = GetGameInstance()->GetSubsystem<UCDRunSubsystem>())
	{
		run->ReturnToHub();
	}
}
