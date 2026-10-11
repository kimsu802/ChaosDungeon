// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include <Abilities/GameplayAbility.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <GameplayAbilitySpec.h>
#include "CDBossAIController.generated.h"


class UCDAbilitySystemComponent;
class UBehaviorTree;
class UCDBossSkillData;
/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API ACDBossAIController : public AAIController
{
	GENERATED_BODY()

public:
	/* 보스 AI 판단 시작 */
	void StartBossLogic();
	
	/* 보스 AI 판단 해제 */
	void StopBossLogic();

	/* 보스의 페이즈 열거형 구분 */
	bool HasReadyPatternInRange() const;

	/* 사용할 공격 실행 여부 */
	UFUNCTION(BlueprintCallable, Category = BossAI)
	bool StartPattern();

	/* 사용 공격 취소 */
	UFUNCTION(BlueprintCallable, Category = BossAI)
	void CancelPattern();

	/* 패턴 진행 여부 */
	UFUNCTION(BlueprintCallable, Category = BossAI)
	bool IsPatternRunnin() const { return bIsPatternRunning; }

	/* 패턴 성공 여부 */
	UFUNCTION(BlueprintCallable, Category = BossAI)
	bool PatternSucceed() const { return bPatternSucceed; }

	/* 조준 대상 갱신 */
	UFUNCTION(BlueprintCallable, Category = BossAI)
	void RefreshTarget();

	/* Behavior Tree 갱신 */
	UFUNCTION(BlueprintCallable, Category = BossAI)
	void RefreshBlackBoard();

protected:
	/* 보스 제어 연결 */
	virtual void OnPossess(APawn* inPawn) override;

	/* 보스 제어 해제 */
	virtual void OnUnPossess() override;

	/* behavior tree */
	UPROPERTY(EditDefaultsOnly, Category = BossAI)
	TObjectPtr<UBehaviorTree> bossBehaviorTree;


private:
	const UCDBossSkillData* ReadyPattern() const;

	/* GAS의 종료 알림 */
	void OnPatternFinished(const FAbilityEndedData& endedData);

	/* 공격 상태 정리 */
	void FinishPattern(bool bSucceeded);

private:
	/* 추적할 플레이어 */
	TWeakObjectPtr<AActor> targetActor;

	/* 공격 시작 종료 처리에 갱신 값 */
	bool bIsPatternRunning = false;

	/* 실행 중인 패턴 식별 */
	FGameplayAbilitySpecHandle patternHandle;

	/* 사용하는 abilityStstem */
	TWeakObjectPtr<UCDAbilitySystemComponent> activeAbilitySystem;

	/* 종료 알림 연결 해제 */
	FDelegateHandle patternEnd;

	bool bPatternSucceed = false;

	bool bBossLogicRunning = false;
};
