#pragma once

#include "Engine/DataAsset.h"
#include "Core/CDTypes.h"
#include "CDDungeonData.generated.h"

class UCDStageData;

/** 던전 전체 구성: 스테이지 순서 + 난이도 배율 */
UCLASS(BlueprintType)
class CHAOSDUNGEON_API UCDDungeonData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 난이도 설정 (없으면 기본값) */
	const FCDDifficultySettings& GetDifficultySettings(ECDDifficulty difficulty) const
	{
		static const FCDDifficultySettings defaultSettings;
		const FCDDifficultySettings* found = difficulties.Find(difficulty);
		if (!found)
		{
			return defaultSettings;
		}
		return *found;
	}

public:
	/** 스테이지 순서 */
	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UCDStageData>> stages;

	// 모든 일반 스테이지를 방문한 뒤 진행할 보스 스테이지.
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCDStageData> bossStage;

	/** 난이도별 배율 */
	UPROPERTY(EditDefaultsOnly)
	TMap<ECDDifficulty, FCDDifficultySettings> difficulties;

	/** 허브 레벨 */
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> hubLevel;
};
