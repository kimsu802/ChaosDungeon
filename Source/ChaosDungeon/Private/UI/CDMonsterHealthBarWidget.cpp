#include "UI/CDMonsterHealthBarWidget.h"
#include "AbilitySystem/CDAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Engine/World.h"

void UCDMonsterHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Screen Space 위젯은 컴포넌트를 숨겼다 켜면 다시 Construct 될 수 있으므로 중복 바인딩 방지
	if (hideAnim != nullptr)
	{
		UnbindAllFromAnimationFinished(hideAnim);
		FWidgetAnimationDynamicEvent finished;
		finished.BindDynamic(this, &ThisClass::HandleHideFinished);
		BindToAnimationFinished(hideAnim, finished);
	}

	if (hitFlash != nullptr)
	{
		hitFlash->SetRenderOpacity(0.f);
	}

	// 바인딩 전 (풀에 막 생성된 상태) 에는 숨김.
	// 이미 BindTo 가 끝난 뒤 다시 Construct 된 경우에는 지금 상태를 유지
	if (boundAbilitySystem.IsValid() == false)
	{
		bBarShown = (bShowOnlyAfterDamage == false);
	}
	SetVisibility(bBarShown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UCDMonsterHealthBarWidget::NativeTick(const FGeometry& myGeometry, float inDeltaTime)
{
	Super::NativeTick(myGeometry, inDeltaTime);

	// 한동안 안 맞으면 다시 숨김 (idleHideTime = 0 이면 계속 표시)
	if (bBarShown && (idleHideTime > 0.f) && bShowOnlyAfterDamage)
	{
		if ((GetWorld()->GetTimeSeconds() - lastHitTime) >= idleHideTime)
		{
			HideBar(true);
		}
	}
}

void UCDMonsterHealthBarWidget::BindTo(UAbilitySystemComponent* abilitySystem)
{
	Unbind();
	if (abilitySystem == nullptr)
	{
		return;
	}

	boundAbilitySystem = abilitySystem;
	healthHandle = abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	maxHealthHandle = abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);

	maxHealth = abilitySystem->GetNumericAttribute(UCDAttributeSet::GetMaxHealthAttribute());
	currentHealth = abilitySystem->GetNumericAttribute(UCDAttributeSet::GetHealthAttribute());

	// 풀에서 막 나온 상태: 지연 연출 없이 가득 채움 + 이전 활성화의 애니메이션 정리
	StopAllAnimations();
	ResetAnimatedValues();
	SnapToPercent(GetHealthPercent());

	bBarShown = (bShowOnlyAfterDamage == false);
	SetVisibility(bBarShown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UCDMonsterHealthBarWidget::Unbind()
{
	if (UAbilitySystemComponent* abilitySystem = boundAbilitySystem.Get())
	{
		abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetHealthAttribute()).Remove(healthHandle);
		abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetMaxHealthAttribute()).Remove(maxHealthHandle);
	}
	boundAbilitySystem = nullptr;
	healthHandle.Reset();
	maxHealthHandle.Reset();
}

void UCDMonsterHealthBarWidget::SetGrade(ECDMonsterGrade grade)
{
	const bool bElite = (grade == ECDMonsterGrade::Elite);
	const FVector2D size = bElite ? eliteSize : normalSize;

	if (barSizeBox != nullptr)
	{
		barSizeBox->SetWidthOverride(size.X);
		barSizeBox->SetHeightOverride(size.Y);
	}

	const ESlateVisibility eliteVisibility = bElite ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	if (eliteFrame != nullptr)
	{
		eliteFrame->SetVisibility(eliteVisibility);
	}

	if (eliteEmblem != nullptr)
	{
		eliteEmblem->SetVisibility(eliteVisibility);
	}
}

void UCDMonsterHealthBarWidget::HideBar(bool bAnimated)
{
	if (bBarShown == false)
	{
		return;
	}

	bBarShown = false;

	if ((bAnimated == false) || (hideAnim == nullptr))
	{
		StopAllAnimations();
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	PlayAnimation(hideAnim);
}

void UCDMonsterHealthBarWidget::HandleHideFinished()
{
	// hideAnim 도중 다시 맞아서 ShowBar 가 불렸으면 숨기지 않음
	if (bBarShown)
	{
		return;
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UCDMonsterHealthBarWidget::ShowBar()
{
	if (bBarShown)
	{
		return;
	}

	bBarShown = true;
	if ((hideAnim != nullptr) && IsAnimationPlaying(hideAnim))
	{
		StopAnimation(hideAnim);
	}
	ResetAnimatedValues();
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCDMonsterHealthBarWidget::HandleHealthChanged(const FOnAttributeChangeData& data)
{
	currentHealth = data.NewValue;

	// 피해: 표시 + 번쩍임 (0 이 된 순간은 몬스터의 HandleDeath 가 HideBar 를 부름)
	if ((data.NewValue < data.OldValue) && (data.NewValue > 0.f))
	{
		lastHitTime = GetWorld()->GetTimeSeconds();
		ShowBar();

		if (hitAnim != nullptr)
		{
			PlayAnimation(hitAnim);
		}
	}

	SetTargetPercent(GetHealthPercent());
}

void UCDMonsterHealthBarWidget::HandleMaxHealthChanged(const FOnAttributeChangeData& data)
{
	maxHealth = data.NewValue;
	SetTargetPercent(GetHealthPercent());
}

void UCDMonsterHealthBarWidget::ResetAnimatedValues()
{
	// 멈춘 애니메이션은 값이 그대로 남으므로 직접 되돌림 (hideAnim = 루트 Opacity, hitAnim = hitFlash Opacity)
	if (UWidget* root = GetRootWidget())
	{
		root->SetRenderOpacity(1.f);
	}

	if (hitFlash != nullptr)
	{
		hitFlash->SetRenderOpacity(0.f);
	}
}

float UCDMonsterHealthBarWidget::GetHealthPercent() const
{
	return (maxHealth > 0.f) ? (currentHealth / maxHealth) : 0.f;
}
