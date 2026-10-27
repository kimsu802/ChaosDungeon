#pragma once

#include "UI/CDHealthBarWidget.h"
#include "Core/CDTypes.h"
#include "CDMonsterHealthBarWidget.generated.h"

class UAbilitySystemComponent;
class UImage;
class USizeBox;
class UWidgetAnimation;
struct FOnAttributeChangeData;

/**
 * 잡몹 · 정예 머리 위 HP 바 (WBP_MonsterHealthBar). ACDMonsterBase 의 healthBarComponent 가 띄운다.
 * - 몬스터 ASC 의 체력 델리게이트를 직접 구독 (몬스터 ↔ 바 1:1 이라 메시지 버스를 쓰지 않음)
 * - 풀 재사용 시 BindTo 를 다시 부르면 이전 구독을 풀고 지연 연출 없이 가득 찬 상태로 맞춘다
 * - 기본은 처음 맞았을 때부터 표시, 죽으면 hideAnim 후 숨김
 * - 지연 바 · 끝부분 글로우는 부모 UCDHealthBarWidget 그대로
 * - 구독은 AddUObject (약참조) 라 위젯이 먼저 사라져도 안전. 해제는 BindTo / Unbind 에서만
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDMonsterHealthBarWidget : public UCDHealthBarWidget
{
	GENERATED_BODY()

public:
	// 체력 구독 + 현재 값으로 즉시 맞춤 (활성화마다 호출)
	void BindTo(UAbilitySystemComponent* abilitySystem);

	// 구독 해제
	void Unbind();

	// 등급별 크기 · 테두리 (일반 / 정예)
	void SetGrade(ECDMonsterGrade grade);

	// 사망 시 숨김. bAnimated 면 hideAnim 재생 후 숨김
	void HideBar(bool bAnimated = true);

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UUserWidget::NativeTick()
	virtual void NativeTick(const FGeometry& myGeometry, float inDeltaTime) override;

private:
	// Health 속성 변경
	void HandleHealthChanged(const FOnAttributeChangeData& data);

	// MaxHealth 속성 변경
	void HandleMaxHealthChanged(const FOnAttributeChangeData& data);

	// 현재 체력 / 최대 체력
	float GetHealthPercent() const;

	// 바를 보이게 (이미 보이면 무시)
	void ShowBar();

	// 애니메이션이 바꿔 놓은 값(루트 Opacity, 번쩍임) 초기화
	void ResetAnimatedValues();

	// hideAnim 끝 → Collapsed
	UFUNCTION()
	void HandleHideFinished();

protected:
	// 바 전체 크기 (등급별 Width / Height Override)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> barSizeBox;

	// 정예 전용 금테 (일반은 숨김)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> eliteFrame;

	// 정예 전용 왼쪽 엠블럼 (일반은 숨김)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> eliteEmblem;

	// 피격 순간 번쩍임 (hitAnim 이 Render Opacity 를 움직임)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> hitFlash;

	// 피격 시 재생 (번쩍임)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> hitAnim;

	// 사망 시 재생 (잠깐 빈 바를 보여 준 뒤 사라짐)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> hideAnim;

	// 피격 전에는 숨기고, 처음 맞았을 때부터 표시
	UPROPERTY(EditAnywhere, Category = "HP|Monster")
	bool bShowOnlyAfterDamage = true;

	// 이 시간(초) 동안 안 맞으면 다시 숨김. 0 = 계속 표시
	UPROPERTY(EditAnywhere, Category = "HP|Monster", meta = (ClampMin = 0))
	float idleHideTime = 0.f;

	// 일반 몬스터 바 크기
	UPROPERTY(EditAnywhere, Category = "HP|Monster")
	FVector2D normalSize = FVector2D(96.f, 12.f);

	// 정예 몬스터 바 크기
	UPROPERTY(EditAnywhere, Category = "HP|Monster")
	FVector2D eliteSize = FVector2D(136.f, 16.f);

private:
	// 구독 중인 ASC
	TWeakObjectPtr<UAbilitySystemComponent> boundAbilitySystem;

	// Health 구독 핸들
	FDelegateHandle healthHandle;

	// MaxHealth 구독 핸들
	FDelegateHandle maxHealthHandle;

	// 현재 체력
	float currentHealth = 0.f;

	// 최대 체력
	float maxHealth = 0.f;

	// 마지막으로 맞은 시각 (월드 시간, idleHideTime 용)
	float lastHitTime = 0.f;

	// 바가 보이는 중 (hideAnim 재생 중은 false)
	bool bBarShown = false;
};
