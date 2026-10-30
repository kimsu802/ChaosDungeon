#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "CDPlayerClassSubsystem.generated.h"

class UCDCharacterClassData;

DECLARE_MULTICAST_DELEGATE_OneParam(FCDPlayerClassChangedSignature, const UCDCharacterClassData*);

/**
 * 선택한 직업 보관소 (GameInstance 수명 → 레벨 이동에도 유지)
 * - 플레이어 캐릭터는 빙의될 때 여기서 직업을 읽는다. (허브 리스폰 · 던전 이동 모두 같은 경로)
 * - 아직 고른 직업이 없으면 캐릭터 BP 의 기본 classData 가 등록된다.
 * - 직업이 바뀌면 이전 직업의 스킬 배치(UCDSkillLoadoutSubsystem)를 지운다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDPlayerClassSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 직업 선택. 이전 직업과 다르면 스킬 배치 초기화 + 알림 */
	void SelectClass(UCDCharacterClassData* newClass);

	/** 아직 고른 직업이 없을 때만 등록 (캐릭터 BP 기본값) */
	void RegisterDefaultClass(UCDCharacterClassData* defaultClass);

	/** 현재 직업 (아직 캐릭터가 빙의된 적 없으면 null) */
	UFUNCTION(BlueprintPure, Category = "Class")
	UCDCharacterClassData* GetSelectedClass() const
	{
		return selectedClass;
	}

	/** 직업이 바뀌었을 때 (초상화 버튼 선택 표시 갱신용) */
	FCDPlayerClassChangedSignature& OnClassChanged()
	{
		return classChanged;
	}

private:
	/** 현재 직업 */
	UPROPERTY(Transient)
	TObjectPtr<UCDCharacterClassData> selectedClass;

	/** 직업 변경 알림 */
	FCDPlayerClassChangedSignature classChanged;
};
