#include "UI/CDFloatingTextSubsystem.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Engine/GameInstance.h"
#include "UI/CDFloatingTextWidget.h"
#include "Settings/CDDeveloperSettings.h"

void UCDFloatingTextSubsystem::OnWorldBeginPlay(UWorld& inWorld)
{
	Super::OnWorldBeginPlay(inWorld);

	UCDMessageSubsystem& messages = UCDMessageSubsystem::Get(&inWorld);
	handles.Add(messages.Listen(CDTags::Msg_Combat_Damage, this, &ThisClass::HandleDamage));
	handles.Add(messages.Listen(CDTags::Msg_UI_FloatingText, this, &ThisClass::HandleWorldText));
}

void UCDFloatingTextSubsystem::Deinitialize()
{
	const UWorld* world = GetWorld();
	const UGameInstance* gameInstance = world ? world->GetGameInstance() : nullptr;

	// 레벨 이동: 메시지 서브시스템이 살아 있으므로 해제
	// 게임/PIE 종료: GameInstance 가 먼저 정리돼 nullptr → 리스너 목록도 이미 사라졌으므로 건너뜀
	if (UCDMessageSubsystem* messages = gameInstance ? gameInstance->GetSubsystem<UCDMessageSubsystem>() : nullptr)
	{
		for (FCDListenerHandle& handle : handles)
		{
			messages->Unlisten(handle);
		}
	}
	handles.Reset();
	Super::Deinitialize();
}

void UCDFloatingTextSubsystem::HandleDamage(const FCDDamageMessage& message)
{
	if (!bDamageNumbersEnabled || !message.target)
	{
		return;
	}

	ECDFloatingTextStyle style = ECDFloatingTextStyle::Damage;
	if (message.bTargetIsPlayer)
	{
		style = ECDFloatingTextStyle::DamageTaken;
	}
	else if (message.bCritical)
	{
		style = ECDFloatingTextStyle::Critical;
	}

	const FVector location = message.target->GetActorLocation() + FVector(0.f, 0.f, 100.f);
	Show(location, FText::AsNumber(FMath::RoundToInt(message.amount)), style);
}

void UCDFloatingTextSubsystem::HandleWorldText(const FCDWorldTextMessage& message)
{
	Show(message.location, message.text, message.style);
}

void UCDFloatingTextSubsystem::Show(const FVector& worldLocation, const FText& text, ECDFloatingTextStyle style)
{
	if (UCDFloatingTextWidget* widget = Acquire())
	{
		widget->Play(worldLocation, text, style);
	}
}

UCDFloatingTextWidget* UCDFloatingTextSubsystem::Acquire()
{
	for (UCDFloatingTextWidget* widget : pool)
	{
		if (!widget->IsInUse())
		{
			return widget;
		}
	}

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	UClass* loadedClass = GetDefault<UCDDeveloperSettings>()->floatingTextWidgetClass.LoadSynchronous();
	if (!playerController || !loadedClass)
	{
		return nullptr;
	}
	UCDFloatingTextWidget* widget = CreateWidget<UCDFloatingTextWidget>(playerController, loadedClass);
	// CommonUI 레이어 밖 (입력을 받지 않는 월드 텍스트)
	widget->AddToPlayerScreen(5);
	pool.Add(widget);
	return widget;
}
