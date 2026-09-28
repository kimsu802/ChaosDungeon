#pragma once

#include "GameplayEffect.h"
#include "CDGameplayEffects.generated.h"

/**
 * 코드에서만 쓰는 공용 GameplayEffect.
 * 스킬마다 GE 에셋을 만들지 않도록, 수치는 SetByCaller 로 넣는다.
 * (회복/버프 구슬처럼 기획이 만지는 GE 는 에디터에서 에셋으로 만든다)
 */

/** 즉시 데미지: UCDDamageExecution 실행 */
UCLASS()
class CHAOSDUNGEON_API UCDDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	/** Instant + DamageExecution 설정 */
	UCDDamageEffect();
};

/** 쿨다운: 지속시간 = SetByCaller.Cooldown, 쿨다운 태그는 어빌리티가 동적으로 부여 */
UCLASS()
class CHAOSDUNGEON_API UCDCooldownEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	/** HasDuration + SetByCaller 지속시간 설정 */
	UCDCooldownEffect();
};
