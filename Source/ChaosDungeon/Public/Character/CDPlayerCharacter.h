#pragma once

#include "Character/CDCharacterBase.h"
#include "CDPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UCDCharacterClassData;
class UCDOcclusionFadeComponent;

/**
 * 플레이어 캐릭터: 쿼터뷰 카메라 + 직업 데이터로 스킬 구성
 * 직업이 늘어나면 classData 만 다른 BP 를 만든다. (클래스 상속 X)
 */
UCLASS()
class CHAOSDUNGEON_API ACDPlayerCharacter : public ACDCharacterBase
{
	GENERATED_BODY()

public:
	/** 카메라/벽 반투명 컴포넌트 생성 */
	ACDPlayerCharacter();

	// ACDCharacterBase::GetAimLocation()
	virtual FVector GetAimLocation() const override;

protected:
	// APawn::PossessedBy()
	virtual void PossessedBy(AController* newController) override;

protected:
	/** 카메라 암 (회전 고정) */
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> cameraBoom;

	/** 쿼터뷰 카메라 */
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> camera;

	/** 카메라를 가리는 벽 반투명 처리 */
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCDOcclusionFadeComponent> occlusionFade;

	/** 직업 데이터 (스탯, 기본 스킬, 회피) */
	UPROPERTY(EditDefaultsOnly, Category = "Class")
	TObjectPtr<UCDCharacterClassData> classData;
};
