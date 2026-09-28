#pragma once

#include "CommonUserWidget.h"
#include "Core/CDTypes.h"
#include "CDFloatingTextWidget.generated.h"

/**
 * 풀링되는 떠오르는 텍스트 1개. 월드 위치 추적 + 수명 관리만 C++, 색/애니메이션은 BP(OnPlay)
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDFloatingTextWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 표시 시작 */
	void Play(const FVector& inWorldLocation, const FText& text, ECDFloatingTextStyle style);

	/** 표시 중인가 (풀링용) */
	FORCEINLINE bool IsInUse() const
	{
		return bInUse;
	}

protected:
	// UUserWidget::NativeTick()
	virtual void NativeTick(const FGeometry& myGeometry, float inDeltaTime) override;

	/** 색/애니메이션 연출 (BP 구현) */
	UFUNCTION(BlueprintImplementableEvent)
	void OnPlay(const FText& text, ECDFloatingTextStyle style);

private:
	/** 숨기고 풀로 반환 */
	void Finish();

protected:
	/** 표시 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "FloatingText")
	float lifetime = 0.8f;

	/** 초당 떠오르는 높이 */
	UPROPERTY(EditDefaultsOnly, Category = "FloatingText")
	float riseSpeed = 80.f;

private:
	/** 따라갈 월드 위치 */
	FVector worldLocation = FVector::ZeroVector;

	/** 경과 시간 */
	float elapsed = 0.f;

	/** 표시 중 */
	bool bInUse = false;
};
