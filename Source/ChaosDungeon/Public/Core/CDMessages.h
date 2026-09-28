#pragma once

#include "CoreMinimal.h"
#include "Core/CDTypes.h"
#include "CDMessages.generated.h"

/**
 * UCDMessageSubsystem 으로 주고받는 메시지 페이로드.
 * 채널 태그는 CDGameplayTags.h 의 Msg_* 참고.
 * 발행 측과 구독 측은 이 헤더만 공유하고 서로의 클래스는 모른다.
 */

/** Combat.Damage */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDDamageMessage
{
	GENERATED_BODY()

	/** 피해를 준 액터 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> instigator;

	/** 피해를 받은 액터 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> target;

	/** 피해량 */
	UPROPERTY(BlueprintReadOnly)
	float amount = 0.f;

	/** 치명타 여부 */
	UPROPERTY(BlueprintReadOnly)
	bool bCritical = false;

	/** 받은 쪽이 플레이어인가 */
	UPROPERTY(BlueprintReadOnly)
	bool bTargetIsPlayer = false;
};

/** Combat.Death */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDDeathMessage
{
	GENERATED_BODY()

	/** 죽은 액터 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> victim;

	/** 죽은 쪽이 플레이어인가 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayer = false;

	/** 처치 게이지 기여도 (플레이어는 0) */
	UPROPERTY(BlueprintReadOnly)
	float contribution = 0.f;
};

/** Combat.BossAppeared */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDActorMessage
{
	GENERATED_BODY()

	/** 대상 액터 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> actor;
};

/** Stage.Started / Stage.Progress / Stage.Cleared / Stage.PortalEntered */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDStageMessage
{
	GENERATED_BODY()

	/** 스테이지 번호 (1부터) */
	UPROPERTY(BlueprintReadOnly)
	int32 stageNumber = 0;

	/** 처치 게이지 (0 ~ 1) */
	UPROPERTY(BlueprintReadOnly)
	float progress = 0.f;

	/** 남은 데스카운트. -1 = 무제한 */
	UPROPERTY(BlueprintReadOnly)
	int32 remainingDeaths = -1;
};

/** Run.DeathCount / Run.Finished */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDRunMessage
{
	GENERATED_BODY()

	/** 현재까지의 런 기록 */
	UPROPERTY(BlueprintReadOnly)
	FCDRunRecord record;

	/** 남은 데스카운트. -1 = 무제한 */
	UPROPERTY(BlueprintReadOnly)
	int32 remainingDeaths = -1;

	/** 최고 기록 갱신 여부 */
	UPROPERTY(BlueprintReadOnly)
	bool bNewRecord = false;
};

/** UI.FloatingText */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDWorldTextMessage
{
	GENERATED_BODY()

	/** 표시할 월드 위치 */
	UPROPERTY(BlueprintReadOnly)
	FVector location = FVector::ZeroVector;

	/** 표시할 텍스트 */
	UPROPERTY(BlueprintReadOnly)
	FText text;

	/** 텍스트 종류 */
	UPROPERTY(BlueprintReadOnly)
	ECDFloatingTextStyle style = ECDFloatingTextStyle::Buff;
};
