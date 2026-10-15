// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CDOptionData.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDOptionData : public UObject
{
	GENERATED_BODY()

public:
	/** 표시 이름 */
	FORCEINLINE const FText& GetDisplayName() const
	{
		return displayName;
	}

public:
	/** 값 변경 알림 (엔트리 갱신, "전체 품질 → 개별 항목" 연동용) */
	FSimpleMulticastDelegate onValueChanged;

protected:
	/** 표시 이름 */
	FText displayName;
};
