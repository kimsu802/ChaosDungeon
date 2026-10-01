#include "Async/CDAsyncAction_ShowConfirm.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UI/CDUIManagerSubsystem.h"

UCDAsyncAction_ShowConfirm* UCDAsyncAction_ShowConfirm::ShowConfirm(const UObject* worldContextObject, ECDConfirmType type, FText title, FText message)
{
	UWorld* world = GEngine->GetWorldFromContextObject(worldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!world)
	{
		return nullptr;
	}

	UCDAsyncAction_ShowConfirm* action = NewObject<UCDAsyncAction_ShowConfirm>();
	action->cachedWorld = world;
	action->cachedType = type;
	action->cachedTitle = title;
	action->cachedMessage = message;

	// 결과가 나올 때까지 GC 되지 않도록 게임 인스턴스에 등록
	action->RegisterWithGameInstance(world);
	return action;
}

void UCDAsyncAction_ShowConfirm::Activate()
{
	const UWorld* world = cachedWorld.Get();
	UCDUIManagerSubsystem* uiManager = world ? UCDUIManagerSubsystem::Get(world->GetFirstPlayerController()) : nullptr;
	if (!uiManager)
	{
		Finish(ECDConfirmResult::Closed);
		return;
	}

	uiManager->ShowConfirm(cachedType, cachedTitle, cachedMessage, [weakThis = TWeakObjectPtr<ThisClass>(this)](ECDConfirmResult result)
		{
			if (weakThis.IsValid())
			{
				weakThis->Finish(result);
			}
		});
}

void UCDAsyncAction_ShowConfirm::Finish(ECDConfirmResult result)
{
	onResult.Broadcast(result);
	SetReadyToDestroy();
}
