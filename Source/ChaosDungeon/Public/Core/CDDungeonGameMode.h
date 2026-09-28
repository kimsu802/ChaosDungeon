#pragma once

#include "GameFramework/GameModeBase.h"
#include "Core/CDMessageSubsystem.h"
#include "CDDungeonGameMode.generated.h"

class UCDSpawnManagerComponent;
class UCDRunSubsystem;
struct FCDDamageMessage;
struct FCDDeathMessage;
struct FCDStageMessage;

/**
 * 던전 스테이지의 규칙 담당: 시작 / 처치 게이지 / 클리어 / 실패 / 부활 / 기록 집계
 * - 스폰은 SpawnManager, 런 흐름·기록은 RunSubsystem 에 위임
 * - 캐릭터·포탈·UI 는 모른다. 메시지로만 주고받는다.
 *     구독: Combat.Damage, Combat.Death, Stage.PortalEntered
 *     발행: Stage.Started, Stage.Progress, Stage.Cleared
 */
UCLASS()
class CHAOSDUNGEON_API ACDDungeonGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 기본 클래스/컴포넌트 설정 */
	ACDDungeonGameMode();

	/** 일시정지 메뉴의 '중도 포기' */
	UFUNCTION(BlueprintCallable)
	void AbandonRun();

protected:
	// AActor::BeginPlay()
	virtual void BeginPlay() override;

	// AActor::EndPlay()
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

private:
	/** "STAGE N" 표시 + 스폰 시작 */
	void BeginStage();

	/** 처치 게이지 달성: 스폰 중지, 포탈 열기 또는 런 성공 */
	void ClearStage();

	/** 데스카운트 초과/포기: 런 실패 */
	void FailRun();

	/** 스테이지 경과 시간을 기록에 누적 */
	void CommitStageTime();

	/** 현재 스테이지 상태로 Stage.* 메시지 발행 */
	void BroadcastStage(FGameplayTag channel) const;

	/** Combat.Damage: 딜량/받은 피해 집계 */
	void HandleDamage(const FCDDamageMessage& message);

	/** Combat.Death: 몬스터면 게이지, 플레이어면 부활/실패 */
	void HandleDeath(const FCDDeathMessage& message);

	/** Stage.PortalEntered: 다음 스테이지로 */
	void HandlePortalEntered(const FCDStageMessage& message);

	/** 레벨에 배치된 스폰 포인트 위치 수집 */
	TArray<FVector> GatherSpawnPoints() const;

	/** 런 서브시스템 */
	UCDRunSubsystem* GetRun() const;

protected:
	/** 몬스터 스폰/풀링 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCDSpawnManagerComponent> spawnManager;

	/** 페이드 인 후 "STAGE N" 표시 + 스폰 시작까지의 대기 */
	UPROPERTY(EditDefaultsOnly, Category = "Rule")
	float stageIntroDelay = 1.5f;

	/** 사망 후 부활까지 대기 */
	UPROPERTY(EditDefaultsOnly, Category = "Rule")
	float reviveDelay = 3.f;

	/** 부활 직후 무적 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Rule")
	float reviveInvincibleTime = 2.f;

private:
	/** 메시지 구독 핸들 */
	TArray<FCDListenerHandle> listenerHandles;

	/** 누적 처치 기여도 */
	float progress = 0.f;

	/** 클리어에 필요한 기여도 */
	float targetProgress = 100.f;

	/** 스테이지 시작 시각 */
	float stageStartTime = 0.f;

	/** 클리어 중복 처리 방지 */
	bool bStageCleared = false;
};
