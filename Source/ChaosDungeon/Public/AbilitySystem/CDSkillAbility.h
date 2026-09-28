#pragma once

#include "AbilitySystem/CDGameplayAbility.h"
#include "CDSkillAbility.generated.h"

class UCDSkillData;

/**
 * 데이터 기반 스킬 (플레이어/몬스터 공용). 스킬 1종마다 클래스를 만들지 않는다.
 *
 * 캔슬 규칙
 *  - 시전 중 State.Casting 부여 → 다른 스킬 발동 불가 (선딜 = 회피로만 캔슬)
 *  - bRecoveryCancelable 스킬은 Event.Montage.Recovery 시점에 State.Casting 해제 → 후딜을 다른 스킬로 캔슬
 *  - 회피는 Ability.Skill 을 항상 캔슬 (UCDDodgeAbility)
 */
UCLASS()
class CHAOSDUNGEON_API UCDSkillAbility : public UCDGameplayAbility
{
	GENERATED_BODY()

public:
	/** 스킬 공통 태그 설정 */
	UCDSkillAbility();

	// UGameplayAbility::OnGiveAbility()
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* actorInfo, const FGameplayAbilitySpec& spec) override;

	// UGameplayAbility::ActivateAbility()
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData) override;

	// UGameplayAbility::EndAbility()
	virtual void EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 이 어빌리티의 스킬 데이터 */
	FORCEINLINE const UCDSkillData* GetSkillData() const
	{
		return skillData;
	}

protected:
	// UCDGameplayAbility::GetCooldownDuration()
	virtual float GetCooldownDuration() const override;

	// UCDGameplayAbility::GetCooldownTag()
	virtual FGameplayTag GetCooldownTag() const override;

	/** 타격 시점(애님 노티파이)마다 호출. 데이터로 표현 못 하는 특수 스킬은 이 함수만 재정의 */
	virtual void ApplyHit();

	/** 대상에게 데미지 GE 적용 + 피격 이벤트 전송 */
	void ApplyDamageTo(AActor* target) const;

private:
	/** Event.Montage.Hit 수신 */
	UFUNCTION()
	void OnHitEvent(FGameplayEventData payload);

	/** Event.Montage.Recovery 수신: 후딜 캔슬 허용 */
	UFUNCTION()
	void OnRecoveryEvent(FGameplayEventData payload);

	/** 몽타주 정상 종료 */
	UFUNCTION()
	void OnMontageFinished();

	/** 몽타주 중단 */
	UFUNCTION()
	void OnMontageCancelled();

	/** State.Casting 부여/해제 */
	void SetCasting(bool bNewCasting);

private:
	/** 스킬 데이터 (Spec 의 SourceObject) */
	UPROPERTY(Transient)
	TObjectPtr<const UCDSkillData> skillData;

	/** 장판 스킬의 판정 위치 */
	FVector targetLocation = FVector::ZeroVector;

	/** State.Casting 을 들고 있는가 */
	bool bIsCasting = false;
};
