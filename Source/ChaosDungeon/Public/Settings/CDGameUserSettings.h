// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "CDGameUserSettings.generated.h"


// Getter / Setter은 계속 선언하고 정의하기 귀찮으니까 매크로로 대체해주자..
#define DECLARE_GETTER_SETTER(Type,Name)							\
FORCEINLINE Type Get##Name() const { return Name; }					\
FORCEINLINE void Set##Name(Type InValue) { Name = InValue; }		\


/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UCDGameUserSettings();
	static UCDGameUserSettings* Get();

	DECLARE_GETTER_SETTER(float, MainVolume);
	DECLARE_GETTER_SETTER(float, SoundFXVolume);
	DECLARE_GETTER_SETTER(float, MusicVolume);
	DECLARE_GETTER_SETTER(FString, Difficulty);


// 매크로 매개변수를 줄이기 위해 파스칼 표기법 사용.
private:
	UPROPERTY(Config)
	float MainVolume;

	UPROPERTY(Config)
	float SoundFXVolume;

	UPROPERTY(Config)
	float MusicVolume;

	UPROPERTY(Config)
	FString Difficulty;
};
