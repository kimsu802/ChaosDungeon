#pragma once

#include "Components/ActorComponent.h"
#include "CDOcclusionFadeComponent.generated.h"

/**
 * 카메라와 플레이어 사이를 가리는 메시를 반투명 처리 (플레이어 캐릭터에 부착)
 * 머티리얼 교체 대신 Custom Primitive Data 값만 바꾼다.
 * → 벽 머티리얼에 fadeParameterName 파라미터(디더 마스크)를 추가해 두어야 한다.
 */
UCLASS(ClassGroup = (CD), meta = (BlueprintSpawnableComponent))
class CHAOSDUNGEON_API UCDOcclusionFadeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 틱 설정 */
	UCDOcclusionFadeComponent();

	// UActorComponent::TickComponent()
	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

private:
	/** 반투명 켜기/끄기 */
	void SetFaded(UPrimitiveComponent* primitive, bool bFaded) const;

protected:
	/** 가림 판정 스윕 반지름 */
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	float probeRadius = 40.f;

	/** 벽 머티리얼의 Custom Primitive Data 파라미터 이름 */
	UPROPERTY(EditAnywhere, Category = "Occlusion")
	FName fadeParameterName = TEXT("OcclusionFade");

private:
	/** 현재 반투명 처리된 메시들 */
	TArray<TWeakObjectPtr<UPrimitiveComponent>> fadedPrimitives;
};
