#pragma once

#include "CoreMinimal.h"
#include "CDReticleData.generated.h"
/** Reticle 표시 방식 */
UENUM(BlueprintType)
enum class ECDReticleDisplayMode : uint8
{
	None,
	Static,		// 고정 형태
	Fill,		// 게이지 형태
};

/** 스킬의 Reticle 표시 설정 */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDReticleConfig
{
	GENERATED_BODY()

public:
	/** Reticle 표시 방식 */
	UPROPERTY(EditDefaultsOnly, Category = "Reticle")
	ECDReticleDisplayMode displayMode;

	/** Reticle 색상 */
	UPROPERTY(EditDefaultsOnly, Category = "Reticle", meta = (EditCondition = "displayMode != ECDReticleDisplayMode::None", EditConditionHides))
	FLinearColor reticleColor = FLinearColor::Red;
};
