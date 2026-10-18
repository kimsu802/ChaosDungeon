#pragma once

#include "CoreMinimal.h"
#include "CDReticleData.generated.h"
// 
UENUM(BlueprintType)
enum class ECDReticleDisplayMode : uint8
{
	None,
	Static,		// 고정 형태
	Fill,		// 게이지 형태
};

USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDReticleConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Reticle")
	ECDReticleDisplayMode displayMode;

	UPROPERTY(EditDefaultsOnly, Category = "Reticle")
	FLinearColor reticleColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly, Category = "Reticle")
	TObjectPtr<class UMaterialInterface> reticleMaterial;		// 장판 외형 머티리얼
};