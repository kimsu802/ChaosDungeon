#pragma once

#include "CoreMinimal.h"
#include "CDTypes.generated.h"

/** 난이도 */
UENUM(BlueprintType)
enum class ECDDifficulty : uint8
{
	Easy,
	Normal,
	Hard
};

/** 떠오르는 텍스트 종류 (색/연출은 위젯 BP 에서 결정) */
UENUM(BlueprintType)
enum class ECDFloatingTextStyle : uint8
{
	Damage,       // 내가 준 피해
	Critical,     // 내가 준 치명타
	DamageTaken,  // 내가 받은 피해
	Buff          // 버프 획득 텍스트
};

/** 몬스터 등급 (경직/넉백 면역 규칙의 기준) */
UENUM(BlueprintType)
enum class ECDMonsterGrade : uint8
{
	Normal,
	Elite,
	Boss
};

/** 커서 호버 외곽선용 스텐실 값 (포스트프로세스 머티리얼과 값을 맞출 것) */
namespace CDStencil
{
	constexpr int32 Player = 1;
	constexpr int32 Monster = 2;
	constexpr int32 Npc = 3;
}

/** 플레이어/몬스터 공용 기본 스탯 */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDBaseStats
{
	GENERATED_BODY()

	/** 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float maxHealth = 1000.f;

	/** 공격력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float attackPower = 100.f;

	/** 받는 피해 감소 비율 (0 ~ 0.9) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, ClampMax = 0.9))
	float defense = 0.f;

	/** 치명타 확률 (0 ~ 1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, ClampMax = 1))
	float critChance = 0.1f;

	/** 치명타 피해 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float critDamage = 1.5f;
};

/** 난이도별 배율. 몬스터 스폰 시 스탯에 곱해진다. */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDDifficultySettings
{
	GENERATED_BODY()

	/** 몬스터 체력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float healthMultiplier = 1.f;

	/** 몬스터 공격력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float attackMultiplier = 1.f;

	/** 한 번에 스폰되는 수 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float spawnCountMultiplier = 1.f;

	/** 허용 사망 횟수. -1 = 무제한 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 maxDeaths = -1;
};

/** 한 번의 던전 도전 기록 (결과 화면/기록 게시판) */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDRunRecord
{
	GENERATED_BODY()

	/** 도전한 난이도 */
	UPROPERTY(BlueprintReadOnly)
	ECDDifficulty difficulty = ECDDifficulty::Normal;

	/** 성공 여부 */
	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	/** 클리어한 스테이지 수 */
	UPROPERTY(BlueprintReadOnly)
	int32 clearedStages = 0;

	/** 걸린 시간(초) */
	UPROPERTY(BlueprintReadOnly)
	float elapsedTime = 0.f;

	/** 처치 수 */
	UPROPERTY(BlueprintReadOnly)
	int32 kills = 0;

	/** 사망 수 */
	UPROPERTY(BlueprintReadOnly)
	int32 deaths = 0;

	/** 누적 딜량 */
	UPROPERTY(BlueprintReadOnly)
	float damageDealt = 0.f;

	/** 받은 피해 */
	UPROPERTY(BlueprintReadOnly)
	float damageTaken = 0.f;
};
