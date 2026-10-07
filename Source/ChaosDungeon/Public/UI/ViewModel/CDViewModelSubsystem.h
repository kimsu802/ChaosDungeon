#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "Core/CDMessageSubsystem.h"
#include "CDViewModelSubsystem.generated.h"

class APawn;
class APlayerController;
class UCDHealthVM;
class UCDStageVM;
class UCDSkillSlotVM;
class UCDRunResultVM;
struct FCDActorMessage;

/**
 * ViewModel 생성 · 등록 · 연결 담당. (VM 은 로컬 플레이어 수명 → 레벨이 바뀌어도 유지)
 * - 모든 VM 을 MVVM Global Viewmodel Collection 에 이름으로 등록한다.
 *   View(위젯 BP)는 Creation Type = Global Viewmodel Collection, 이름만 지정하면 된다.
 *     PlayerHealth, BossHealth, Stage, RunResult
 * - 스킬 슬롯 VM 은 슬롯마다 다르므로 GetSkillSlot(inputTag) 로 조회 (UCDSkillSlotWidget)
 * - 폰이 바뀌면 ASC 에 다시 연결
 */
UCLASS()
class CHAOSDUNGEON_API UCDViewModelSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem::Initialize()
	virtual void Initialize(FSubsystemCollectionBase& collection) override;

	// USubsystem::Deinitialize()
	virtual void Deinitialize() override;

	/** UIManager::InitializeForPlayer 에서 호출: 폰 변경 구독 + 현재 폰 연결 */
	void BindPlayer(APlayerController* playerController);

	/** 입력 태그에 해당하는 슬롯 VM */
	FORCEINLINE UCDSkillSlotVM* GetSkillSlot(FGameplayTag inputTag) const
	{
		return skillSlots.FindRef(inputTag);
	}

private:
	/** 폰 변경 → HP/슬롯 VM 을 새 ASC 에 연결 */
	UFUNCTION()
	void HandlePawnChanged(APawn* oldPawn, APawn* newPawn);

	/** 슬롯 VM 들을 키 매핑 설정에 연결 (슬롯별 키 표시용) */
	void BindSlotKeys(const APlayerController* playerController);

	/** Combat.BossAppeared → 보스 HP VM 연결 */
	void HandleBossAppeared(const FCDActorMessage& message);

	/** VM 생성 + Global Viewmodel Collection 등록 */
	template <typename TViewModel>
	TViewModel* CreateGlobalViewModel(FName name);

	/** 메시지 서브시스템 */
	UCDMessageSubsystem& GetMessages() const;

private:
	/** 플레이어 HP */
	UPROPERTY(Transient)
	TObjectPtr<UCDHealthVM> playerHealth;

	/** 보스 HP */
	UPROPERTY(Transient)
	TObjectPtr<UCDHealthVM> bossHealth;

	/** 스테이지 정보 */
	UPROPERTY(Transient)
	TObjectPtr<UCDStageVM> stage;

	/** 결과 화면 */
	UPROPERTY(Transient)
	TObjectPtr<UCDRunResultVM> runResult;

	/** 입력 태그별 슬롯 VM */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCDSkillSlotVM>> skillSlots;

	/** Combat.BossAppeared 구독 핸들 */
	FCDListenerHandle bossHandle;
};
