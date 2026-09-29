// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/CDGameUserSettings.h"

UCDGameUserSettings::UCDGameUserSettings()
    :MainVolume(1.f),SoundFXVolume(1.f),MusicVolume(1.f)
{
    
}

UCDGameUserSettings* UCDGameUserSettings::Get()
{
    if (GEngine)
    {
        return CastChecked<UCDGameUserSettings>(GEngine->GetGameUserSettings());
    }

    return nullptr;
}
