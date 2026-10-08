// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CDDeveloperSettings.generated.h"

class UCDFloatingTextWidget;
class UCDDungeonData;
class UCDUIScreenSet;
class UUserWidget;

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

	/** 레벨 전환 / 화면 페이드 시간 */
	UPROPERTY(Config, EditAnywhere, Category = "UI|Fade", meta = (ClampMin = 0))
	float fadeDuration = 0.5f;

	/** 페이드 오버레이 위젯. 비우면 검은 화면 (로고/팁을 넣으려면 지정, 루트 Visibility = Visible 권장) */
	UPROPERTY(Config, EditAnywhere, Category = "UI|Fade")
	TSoftClassPtr<UUserWidget> fadeWidgetClass;

	/** 페이드 오버레이 ZOrder. CommonUI 레이아웃(100)보다 커야 UI 까지 덮는다 */
	UPROPERTY(Config, EditAnywhere, Category = "UI|Fade")
	int32 fadeZOrder = 1000;
	
};
