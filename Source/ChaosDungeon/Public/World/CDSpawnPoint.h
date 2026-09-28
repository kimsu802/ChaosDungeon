#pragma once

#include "Engine/TargetPoint.h"
#include "CDSpawnPoint.generated.h"

/**
 * 레벨에 배치하는 스폰 위치 표식. 로직 없음.
 * 수작업 아레나(B플랜)든 청크 조합(A플랜)이든 이 액터만 배치하면 된다.
 */
UCLASS()
class CHAOSDUNGEON_API ACDSpawnPoint : public ATargetPoint
{
	GENERATED_BODY()
};
