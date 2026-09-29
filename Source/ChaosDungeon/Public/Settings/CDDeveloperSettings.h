// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CDDeveloperSettings.generated.h"

class UCDFloatingTextWidget;
class UCDDungeonData;
class UCDUIScreenSet;

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "ChaosDungeon General Settings"))
class CHAOSDUNGEON_API UCDDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 던전 구성 (스테이지 순서, 난이도) */
	UPROPERTY(Config, EditAnywhere, Category = "Data")
	TSoftObjectPtr<UCDDungeonData> dungeonData;

	/** UI 화면 목록 */
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	TSoftObjectPtr<UCDUIScreenSet> screenSet;

	/** 로딩 화면 위젯 */
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	TSoftClassPtr<UUserWidget> loadingScreenClass;

	/** 데미지 숫자 위젯 */
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	TSoftClassPtr<UCDFloatingTextWidget> floatingTextWidgetClass;

	/** 레벨 전환 페이드 시간 */
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	float fadeDuration = 0.5f;
	
};
