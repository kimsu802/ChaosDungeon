#include "Core/CDSkillLoadoutSubsystem.h"

void UCDSkillLoadoutSubsystem::SaveLoadout(const TMap<FGameplayTag, FGameplayTag>& inLoadout)
{
	loadout = inLoadout;
	bHasLoadout = true;
}

void UCDSkillLoadoutSubsystem::ClearLoadout()
{
	loadout.Reset();
	bHasLoadout = false;
}
