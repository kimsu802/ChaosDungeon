#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "CDSkillLoadoutSubsystem.generated.h"

/**
 * 스킬 슬롯 배치 보관소 (입력 태그 → 스킬 태그)
 * - 슬롯 배치는 캐릭터의 ASC(Spec 의 입력 태그)에 있으므로 레벨 이동으로 캐릭터가 파괴되면 사라진다.
 *   GameInstance 수명인 여기에 복사해 두고, 새 캐릭터가 빙의될 때 다시 적용한다.
 * - Spec 핸들은 ASC 마다 달라지므로 바뀌지 않는 스킬 태그(Ability.Skill.*)로 저장한다.
 * - 비어 있으면 DA_Class 의 기본 배치를 쓴다. (나중에 USaveGame 으로 디스크 저장 시 이 맵을 그대로 사용)
 */
UCLASS()
class CHAOSDUNGEON_API UCDSkillLoadoutSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 현재 배치 저장 (스왑할 때마다 호출) */
	void SaveLoadout(const TMap<FGameplayTag, FGameplayTag>& inLoadout);

	/** 저장된 배치 삭제 → 다음 빙의부터 기본 배치 */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ClearLoadout();

	/** 저장된 배치 (입력 태그 → 스킬 태그) */
	FORCEINLINE const TMap<FGameplayTag, FGameplayTag>& GetLoadout() const
	{
		return loadout;
	}

	/** 저장된 배치가 있는가 */
	FORCEINLINE bool HasLoadout() const
	{
		return bHasLoadout;
	}

private:
	/** 입력 태그(Input.Skill.N) → 스킬 태그(Ability.Skill.*) */
	TMap<FGameplayTag, FGameplayTag> loadout;

	/** 한 번이라도 저장했는가 (모든 슬롯을 비운 배치도 유효하므로 맵 크기로 판단하지 않는다) */
	bool bHasLoadout = false;
};
