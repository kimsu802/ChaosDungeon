#include "Async/CDAsyncAction_FadeScreen.h"
#include "Core/CDLevelTransitionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UCDAsyncAction_FadeScreen* UCDAsyncAction_FadeScreen::FadeScreen(const UObject* worldContextObject, float targetAlpha)
{
	UWorld* world = GEngine->GetWorldFromContextObject(worldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!world)
	{
		return nullptr;
	}

	UCDAsyncAction_FadeScreen* action = NewObject<UCDAsyncAction_FadeScreen>();
	action->cachedWorld = world;
	action->cachedTargetAlpha = targetAlpha;

	// 페이드가 끝날 때까지 GC 되지 않도록 게임 인스턴스에 등록
	action->RegisterWithGameInstance(world);
	return action;
}

void UCDAsyncAction_FadeScreen::Activate()
{
	const UWorld* world = cachedWorld.Get();
	const UGameInstance* gameInstance = world ? world->GetGameInstance() : nullptr;
	UCDLevelTransitionSubsystem* transition = gameInstance ? gameInstance->GetSubsystem<UCDLevelTransitionSubsystem>() : nullptr;

	const bool bStarted = transition && transition->FadeScreen(cachedTargetAlpha, [weakThis = TWeakObjectPtr<ThisClass>(this)]()
	{
		if (weakThis.IsValid())
		{
			weakThis->Finish();
		}
	});

	if (!bStarted)
	{
		Finish();
	}
}

void UCDAsyncAction_FadeScreen::Finish()
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;
	onFinished.Broadcast();
	SetReadyToDestroy();
}
