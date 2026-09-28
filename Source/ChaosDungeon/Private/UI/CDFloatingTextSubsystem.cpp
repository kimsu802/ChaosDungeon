#include "UI/CDFloatingTextSubsystem.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Engine/GameInstance.h"
#include "UI/CDFloatingTextWidget.h"

void UCDFloatingTextSubsystem::OnWorldBeginPlay(UWorld& inWorld)
{
	Super::OnWorldBeginPlay(inWorld);

	UCDMessageSubsystem& messages = UCDMessageSubsystem::Get(&inWorld);
	handles.Add(messages.Listen(CDTags::Msg_Combat_Damage, this, &ThisClass::HandleDamage));
	handles.Add(messages.Listen(CDTags::Msg_UI_FloatingText, this, &ThisClass::HandleWorldText));
}

void UCDFloatingTextSubsystem::Deinitialize()
{
	const UGameInstance* gameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (gameInstance)
	{
		UCDMessageSubsystem* messages = gameInstance->GetSubsystem<UCDMessageSubsystem>();
		for (FCDListenerHandle& handle : handles)
		{
			messages->Unlisten(handle);
		}
	}
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
	UClass* loadedClass = widgetClass.LoadSynchronous();
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
