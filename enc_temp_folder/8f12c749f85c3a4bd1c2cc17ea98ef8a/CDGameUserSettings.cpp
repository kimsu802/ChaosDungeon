// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/CDGameUserSettings.h"
#include "Settings/CDDeveloperSettings.h"
#include "Sound/SoundClass.h"

UCDGameUserSettings::UCDGameUserSettings()
    :MainVolume(1.f),SoundFXVolume(1.f),MusicVolume(1.f)
{
    
}

void UCDGameUserSettings::ApplyVolumes() const
{
    const UCDDeveloperSettings* developerSettings = GetDefault<UCDDeveloperSettings>();

    // 사운드 클래스 볼륨을 직접 바꾼다 (SFX / Music 이 Master 의 자식이면 Master 볼륨이 곱해짐)
    auto ApplyVolume = [](const TSoftObjectPtr<USoundClass>& soundClass, float volume)
    {
        if (USoundClass* loadedClass = soundClass.LoadSynchronous())
        {
            loadedClass->Properties.Volume = volume;
        }
    };

    ApplyVolume(developerSettings->masterSoundClass, MainVolume);
    ApplyVolume(developerSettings->sfxSoundClass, SoundFXVolume);
    ApplyVolume(developerSettings->musicSoundClass, MusicVolume);
}

UCDGameUserSettings* UCDGameUserSettings::Get()
{
    if (GEngine)
    {
        return CastChecked<UCDGameUserSettings>(GEngine->GetGameUserSettings());
    }

    return nullptr;
}
