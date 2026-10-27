#include "UI/ListData/CDOptionRegistry.h"
#include "UI/ListData/CDOptionData.h"
#include "UI/ListData/CDOptionData_Discrete.h"
#include "UI/ListData/CDOptionData_KeyBinding.h"
#include "UI/ListData/CDOptionData_Scalar.h"
#include "Data/CDInputConfig.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "InputAction.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/CDPlayerController.h"
#include "PlayerMappableKeySettings.h"
#include "Settings/CDGameUserSettings.h"

#define LOCTEXT_NAMESPACE "CDOptionRegistry"

namespace CDOptionTabs
{
	static const FName Graphics = TEXT("Graphics");
	static const FName Sound = TEXT("Sound");
	static const FName Controls = TEXT("Controls");
}

template <typename TOption>
TOption* UCDOptionRegistry::AddOption(FCDOptionTab& tab, const FText& displayName, const FText& description)
{
	TOption* option = NewObject<TOption>(this);
	option->InitInfo(displayName, description);
	tab.options.Add(option);
	return option;
}

void UCDOptionRegistry::Build(APlayerController* playerController)
{
	tabs.Reset();
	BuildGraphicsTab();
	BuildSoundTab();
	BuildControlsTab(playerController);
}

const FCDOptionTab* UCDOptionRegistry::FindTab(FName tabId) const
{
	return tabs.FindByPredicate([tabId](const FCDOptionTab& tab)
		{
			return tab.tabId == tabId;
		});
}

void UCDOptionRegistry::ResetTab(FName tabId)
{
	const FCDOptionTab* tab = FindTab(tabId);
	if (tab == nullptr)
	{
		return;
	}

	// 키 맞바꿈은 순서에 따라 한 번에 안 맞을 수 있어 두 번 돈다 (이미 기본값인 항목은 그대로)
	for (int32 pass = 0; pass < 2; ++pass)
	{
		for (UCDOptionData* option : tab->options)
		{
			if (option)
			{
				option->ResetToDefault();
			}
		}
	}
}

void UCDOptionRegistry::RefreshAll()
{
	for (const FCDOptionTab& tab : tabs)
	{
		for (UCDOptionData* option : tab.options)
		{
			if (option)
			{
				option->NotifyValueChanged();
			}
		}
	}
}

void UCDOptionRegistry::BuildGraphicsTab()
{
	UGameUserSettings* settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (settings == nullptr)
	{
		return;
	}

	FCDOptionTab& tab = tabs.AddDefaulted_GetRef();
	tab.tabId = CDOptionTabs::Graphics;
	tab.displayName = LOCTEXT("TabGraphics", "그래픽");

	// 화면 모드 (EWindowMode 순서: Fullscreen, WindowedFullscreen, Windowed)
	UCDOptionData_Discrete* windowMode = AddOption<UCDOptionData_Discrete>(tab,
		LOCTEXT("WindowMode", "화면 모드"),
		LOCTEXT("WindowModeDesc", "전체 화면은 성능이 가장 좋고, 테두리 없는 창은 다른 프로그램으로 빠르게 전환할 수 있습니다."));
	windowMode->InitDiscrete(
		{ LOCTEXT("Fullscreen", "전체 화면"), LOCTEXT("Borderless", "테두리 없는 창"), LOCTEXT("Windowed", "창 모드") },
		[settings]()
		{
			return static_cast<int32>(settings->GetFullscreenMode());
		},
		[settings](int32 index)
		{
			settings->SetFullscreenMode(static_cast<EWindowMode::Type>(index));
			settings->ApplyResolutionSettings(false);
		},
		static_cast<int32>(EWindowMode::WindowedFullscreen));

	// 해상도 (모니터가 지원하는 목록, 큰 것부터)
	TArray<FIntPoint> resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(resolutions);
	if (resolutions.Num() == 0)
	{
		resolutions.Add(settings->GetScreenResolution());
	}
	resolutions.Sort([](const FIntPoint& a, const FIntPoint& b)
		{
			return (a.X != b.X) ? (a.X > b.X) : (a.Y > b.Y);
		});

	FNumberFormattingOptions noGrouping;
	noGrouping.UseGrouping = false;
	TArray<FText> resolutionNames;
	for (const FIntPoint& resolution : resolutions)
	{
		resolutionNames.Add(FText::Format(LOCTEXT("ResolutionFormat", "{0} × {1}"), FText::AsNumber(resolution.X, &noGrouping), FText::AsNumber(resolution.Y, &noGrouping)));
	}

	UCDOptionData_Discrete* resolution = AddOption<UCDOptionData_Discrete>(tab,
		LOCTEXT("Resolution", "해상도"),
		LOCTEXT("ResolutionDesc", "화면 해상도입니다. 테두리 없는 창에서는 바탕 화면 해상도를 따릅니다."));
	resolution->InitDiscrete(MoveTemp(resolutionNames),
		[settings, resolutions]()
		{
			return resolutions.IndexOfByKey(settings->GetScreenResolution());
		},
		[settings, resolutions](int32 index)
		{
			settings->SetScreenResolution(resolutions[index]);
			settings->ApplyResolutionSettings(false);
		},
		FMath::Max(0, resolutions.IndexOfByKey(settings->GetDesktopResolution())));

	// 전체 품질 → 개별 품질 (개별을 따로 바꾸면 전체는 "사용자 지정")
	const TArray<FText> qualityNames = { LOCTEXT("QualityLow", "낮음"), LOCTEXT("QualityMedium", "중간"), LOCTEXT("QualityHigh", "높음"), LOCTEXT("QualityEpic", "최상") };
	const int32 defaultQuality = 2;

	UCDOptionData_Discrete* overall = AddOption<UCDOptionData_Discrete>(tab,
		LOCTEXT("OverallQuality", "전체 품질"),
		LOCTEXT("OverallQualityDesc", "아래 품질 항목을 한 번에 바꿉니다. 항목을 따로 바꾸면 '사용자 지정'으로 표시됩니다."));
	TWeakObjectPtr<UCDOptionData_Discrete> weakOverall = overall;
	TArray<TWeakObjectPtr<UCDOptionData>> details;

	auto AddQuality = [&](const FText& displayName, const FText& description, TFunction<int32()>&& getter, TFunction<void(int32)>&& setter)
	{
		UCDOptionData_Discrete* quality = AddOption<UCDOptionData_Discrete>(tab, displayName, description);
		quality->InitDiscrete(TArray<FText>(qualityNames), MoveTemp(getter),
			[settings, weakOverall, setter = MoveTemp(setter)](int32 index)
			{
				setter(index);
				settings->ApplyNonResolutionSettings();

				// 전체 품질 표시 갱신 (같은 단계가 아니면 "사용자 지정")
				if (UCDOptionData_Discrete* overallOption = weakOverall.Get())
				{
					overallOption->NotifyValueChanged();
				}
			},
			defaultQuality);
		details.Add(quality);
	};

	AddQuality(LOCTEXT("TextureQuality", "텍스처"), LOCTEXT("TextureQualityDesc", "표면 디테일입니다. 그래픽 메모리를 가장 많이 사용합니다."),
		[settings]() { return settings->GetTextureQuality(); }, [settings](int32 index) { settings->SetTextureQuality(index); });
	AddQuality(LOCTEXT("ShadowQuality", "그림자"), LOCTEXT("ShadowQualityDesc", "그림자의 선명도와 거리입니다. 성능에 영향이 큽니다."),
		[settings]() { return settings->GetShadowQuality(); }, [settings](int32 index) { settings->SetShadowQuality(index); });
	AddQuality(LOCTEXT("EffectQuality", "이펙트"), LOCTEXT("EffectQualityDesc", "스킬 · 폭발 이펙트의 디테일입니다. 몬스터가 많을 때 프레임에 영향을 줍니다."),
		[settings]() { return settings->GetVisualEffectQuality(); }, [settings](int32 index) { settings->SetVisualEffectQuality(index); });
	AddQuality(LOCTEXT("PostProcessQuality", "후처리"), LOCTEXT("PostProcessQualityDesc", "블룸 · 피사계 심도 등 화면 전체 효과입니다."),
		[settings]() { return settings->GetPostProcessingQuality(); }, [settings](int32 index) { settings->SetPostProcessingQuality(index); });
	AddQuality(LOCTEXT("AntiAliasingQuality", "안티에일리어싱"), LOCTEXT("AntiAliasingQualityDesc", "가장자리 계단 현상을 줄입니다."),
		[settings]() { return settings->GetAntiAliasingQuality(); }, [settings](int32 index) { settings->SetAntiAliasingQuality(index); });

	// 개별 항목을 다 만든 뒤 전체 품질 연결 (바꾸면 개별 항목 표시도 갱신)
	overall->InitDiscrete(TArray<FText>(qualityNames),
		[settings]()
		{
			return settings->GetOverallScalabilityLevel();
		},
		[settings, details](int32 index)
		{
			settings->SetOverallScalabilityLevel(index);
			settings->ApplyNonResolutionSettings();
			for (const TWeakObjectPtr<UCDOptionData>& detail : details)
			{
				if (UCDOptionData* detailOption = detail.Get())
				{
					detailOption->NotifyValueChanged();
				}
			}
		},
		defaultQuality);

	// 수직 동기화
	UCDOptionData_Discrete* vsync = AddOption<UCDOptionData_Discrete>(tab,
		LOCTEXT("VSync", "수직 동기화"),
		LOCTEXT("VSyncDesc", "화면 찢어짐을 막습니다. 켜면 입력이 아주 조금 늦어질 수 있습니다."));
	vsync->InitDiscrete(
		{ LOCTEXT("Off", "끄기"), LOCTEXT("On", "켜기") },
		[settings]()
		{
			return settings->IsVSyncEnabled() ? 1 : 0;
		},
		[settings](int32 index)
		{
			settings->SetVSyncEnabled(index == 1);
			settings->ApplyNonResolutionSettings();
		},
		1);

	// 최대 프레임 (0 = 제한 없음)
	const TArray<float> frameLimits = { 30.f, 60.f, 120.f, 144.f, 0.f };
	UCDOptionData_Discrete* frameLimit = AddOption<UCDOptionData_Discrete>(tab,
		LOCTEXT("FrameLimit", "최대 프레임"),
		LOCTEXT("FrameLimitDesc", "초당 프레임 상한입니다. 노트북에서는 낮추면 발열과 배터리 소모가 줄어듭니다."));
	frameLimit->InitDiscrete(
		{ FText::AsNumber(30), FText::AsNumber(60), FText::AsNumber(120), FText::AsNumber(144), LOCTEXT("Unlimited", "제한 없음") },
		[settings, frameLimits]()
		{
			return frameLimits.IndexOfByPredicate([current = settings->GetFrameRateLimit()](float limit)
				{
					return FMath::IsNearlyEqual(limit, current);
				});
		},
		[settings, frameLimits](int32 index)
		{
			settings->SetFrameRateLimit(frameLimits[index]);
			settings->ApplyNonResolutionSettings();
		},
		frameLimits.Num() - 1);
}

void UCDOptionRegistry::BuildSoundTab()
{
	UCDGameUserSettings* settings = Cast<UCDGameUserSettings>(GEngine ? GEngine->GetGameUserSettings() : nullptr);
	if (settings == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Option] DefaultEngine.ini 의 GameUserSettingsClassName 이 CDGameUserSettings 가 아니어서 사운드 탭을 만들지 않습니다."));
		return;
	}

	FCDOptionTab& tab = tabs.AddDefaulted_GetRef();
	tab.tabId = CDOptionTabs::Sound;
	tab.displayName = LOCTEXT("TabSound", "사운드");

	// 볼륨 3종: 0 ~ 100%, 5% 단위
	auto AddVolume = [&](const FText& displayName, const FText& description, TFunction<float()>&& getter, TFunction<void(float)>&& setter)
	{
		UCDOptionData_Scalar* volume = AddOption<UCDOptionData_Scalar>(tab, displayName, description);
		volume->InitScalar(MoveTemp(getter),
			[settings, setter = MoveTemp(setter)](float value)
			{
				setter(value);
				settings->ApplyVolumes();
			},
			0.f, 1.f, 0.05f, 1.f);
	};

	AddVolume(LOCTEXT("MainVolume", "전체 볼륨"), LOCTEXT("MainVolumeDesc", "모든 소리의 크기입니다. 효과음과 배경음에 곱해집니다."),
		[settings]() { return settings->GetMainVolume(); }, [settings](float value) { settings->SetMainVolume(value); });
	AddVolume(LOCTEXT("SoundFXVolume", "효과음"), LOCTEXT("SoundFXVolumeDesc", "스킬 · 타격 · UI 소리의 크기입니다."),
		[settings]() { return settings->GetSoundFXVolume(); }, [settings](float value) { settings->SetSoundFXVolume(value); });
	AddVolume(LOCTEXT("MusicVolume", "배경음"), LOCTEXT("MusicVolumeDesc", "던전 · 보스전 음악의 크기입니다."),
		[settings]() { return settings->GetMusicVolume(); }, [settings](float value) { settings->SetMusicVolume(value); });
}

void UCDOptionRegistry::BuildControlsTab(APlayerController* playerController)
{
	const ACDPlayerController* cdController = Cast<ACDPlayerController>(playerController);
	const UCDInputConfig* inputConfig = cdController ? cdController->GetInputConfig() : nullptr;
	if (inputConfig == nullptr)
	{
		return;
	}

	FCDOptionTab& tab = tabs.AddDefaulted_GetRef();
	tab.tabId = CDOptionTabs::Controls;
	tab.displayName = LOCTEXT("TabControls", "조작");

	TArray<TWeakObjectPtr<UCDOptionData_KeyBinding>> keyOptions;
	for (const FCDAbilityInput& abilityInput : inputConfig->abilityInputs)
	{
		// 키 변경이 설정되지 않은 입력(Player Mappable Key Settings 없음)은 제외
		if ((abilityInput.action == nullptr) || inputConfig->FindMappingName(abilityInput.inputTag).IsNone())
		{
			continue;
		}

		// 이름은 입력 액션의 Player Mappable Key Settings > Display Name (비어 있으면 태그 이름)
		FText displayName = FText::FromName(abilityInput.inputTag.GetTagName());
		if (const UPlayerMappableKeySettings* keySettings = abilityInput.action->GetPlayerMappableKeySettings())
		{
			if (keySettings->DisplayName.IsEmpty() == false)
			{
				displayName = keySettings->DisplayName;
			}
		}

		UCDOptionData_KeyBinding* keyOption = AddOption<UCDOptionData_KeyBinding>(tab, displayName,
			LOCTEXT("KeyBindingDesc", "클릭한 뒤 바꿀 키를 누르세요. 다른 슬롯이 쓰는 키를 고르면 두 키가 서로 바뀝니다.\nEsc · 마우스 좌/우 · F1 은 쓸 수 없습니다."));
		keyOption->InitKeyBinding(playerController, abilityInput.inputTag);
		keyOptions.Add(keyOption);
	}

	// 맞바꿈이 일어나면 같은 탭의 키 항목 전부 갱신
	for (const TWeakObjectPtr<UCDOptionData_KeyBinding>& weakOption : keyOptions)
	{
		if (UCDOptionData_KeyBinding* keyOption = weakOption.Get())
		{
			keyOption->onRemapped = [keyOptions]()
			{
				for (const TWeakObjectPtr<UCDOptionData_KeyBinding>& other : keyOptions)
				{
					if (UCDOptionData_KeyBinding* otherOption = other.Get())
					{
						otherOption->NotifyValueChanged();
					}
				}
			};
		}
	}
}

#undef LOCTEXT_NAMESPACE
