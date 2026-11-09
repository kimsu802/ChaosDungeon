

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/CDTypes.h"
#include "CDMonsterDataInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCDMonsterDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CHAOSDUNGEON_API ICDMonsterDataInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual ECDMonsterGrade GetMonsterGrade() const = 0;
	
};
