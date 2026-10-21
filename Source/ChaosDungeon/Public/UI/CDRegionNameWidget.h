// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CDRegionNameWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

/**
 * 우측 상단 지역명. 스테이지가 바뀌면 사라졌다가 새 이름으로 다시 나타난다.
 * 위젯 BP 의 Viewmodels 에 Stage (UCDStageVM, Global Viewmodel Collection) 를 추가하고 아래처럼 바인딩한다.
 *   Stage.StageName     → SetRegionName
 *   Stage.IsInStage     → SetInStage      (허브에서는 숨김)
 *   Stage.IsBossVisible → SetBossVisible  (보스전 중에는 숨김)
 * 주의: regionText.Text 에 직접 바인딩하지 말 것 (사라지기 전에 글자가 먼저 바뀜)
 */
UCLASS()
class CHAOSDUNGEON_API UCDRegionNameWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// MVVM 바인딩 대상: 지역명
	UFUNCTION(BlueprintCallable, Category = "Region")
	void SetRegionName(FText newName);

	// MVVM 바인딩 대상: 스테이지 진행 중 여부
	UFUNCTION(BlueprintCallable, Category = "Region")
	void SetInStage(bool bNewInStage);

	// MVVM 바인딩 대상: 보스 HP 바 표시 여부
	UFUNCTION(BlueprintCallable, Category = "Region")
	void SetBossVisible(bool bNewBossVisible);

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

private:
	// 현재 상태로 보여야 하는지
	FORCEINLINE bool ShouldShow() const { return bInStage && !bBossVisible && !pendingName.IsEmpty(); }

	// 표시 상태 갱신 (보여야 하는데 숨겨져 있으면 등장, 반대면 퇴장)
	void RefreshShown();

	// 숨김 → 텍스트 교체 → 등장 순서로 이름 바꾸기
	void SwapNameWithFade();

	// 애니메이션이 끝났을 때 (퇴장이 끝났으면 텍스트를 바꾸고 다시 등장)
	UFUNCTION()
	void HandleShowAnimFinished();

	// 즉시 표시 / 숨김 (애니메이션이 없을 때)
	void SetShownImmediate(bool bShow);

protected:
	// 지역명 텍스트
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> regionText;

	// 등장 애니메이션 (역재생 = 퇴장). Opacity 0→1, Translation X +30→0, 0.3s
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> showAnim;

	// 퇴장 재생 속도 (showAnim 역재생 배속. 1.5 → 약 0.2s)
	UPROPERTY(EditAnywhere, Category = "Region")
	float hideSpeed = 1.5f;

private:
	// 표시해야 할 이름 (최신 값)
	FText pendingName;

	// 스테이지 진행 중
	bool bInStage = false;

	// 보스전 중
	bool bBossVisible = false;

	// 현재 화면에 보이는 중 (등장 중 포함)
	bool bShown = false;

	// 퇴장이 끝나면 이름을 바꾸고 다시 등장해야 함
	bool bSwapPending = false;
};
