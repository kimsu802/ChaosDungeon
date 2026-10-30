#include "Core/CDPlayerClassSubsystem.h"
#include "Core/CDSkillLoadoutSubsystem.h"
#include "Data/CDCharacterClassData.h"
#include "Engine/GameInstance.h"

void UCDPlayerClassSubsystem::SelectClass(UCDCharacterClassData* newClass)
{
	if ((newClass == nullptr) || (newClass == selectedClass))
	{
		return;
	}

	// 이전 직업의 스킬 배치는 새 직업에 맞지 않는다 → 다음 빙의 때 새 직업의 기본 배치 사용
	if (selectedClass != nullptr)
	{
		if (UCDSkillLoadoutSubsystem* loadout = GetGameInstance()->GetSubsystem<UCDSkillLoadoutSubsystem>())
		{
			loadout->ClearLoadout();
		}
	}

	selectedClass = newClass;
	classChanged.Broadcast(selectedClass.Get());
}

void UCDPlayerClassSubsystem::RegisterDefaultClass(UCDCharacterClassData* defaultClass)
{
	if ((selectedClass != nullptr) || (defaultClass == nullptr))
	{
		return;
	}

	selectedClass = defaultClass;
	classChanged.Broadcast(selectedClass.Get());
}
