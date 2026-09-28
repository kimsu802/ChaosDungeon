#include "Core/CDMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"

UCDMessageSubsystem& UCDMessageSubsystem::Get(const UObject* worldContextObject)
{
	const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(worldContextObject);
	check(gameInstance);
	return *gameInstance->GetSubsystem<UCDMessageSubsystem>();
}

void UCDMessageSubsystem::BroadcastInternal(FGameplayTag channel, const UScriptStruct* type, const void* payload)
{
	const TArray<FListener>* channelListeners = listeners.Find(channel);
	if (!channelListeners)
	{
		return;
	}

	// 콜백 안에서 구독/해제가 일어나도 안전하도록 복사본으로 순회
	const TArray<FListener> snapshot = *channelListeners;
	for (const FListener& listener : snapshot)
	{
		if (ensureMsgf(listener.type == type, TEXT("채널 %s 의 메시지 타입이 다릅니다: %s"), *channel.ToString(), *GetNameSafe(type)))
		{
			listener.callback(payload);
		}
	}
}

FCDListenerHandle UCDMessageSubsystem::ListenInternal(FGameplayTag channel, const UScriptStruct* type, TFunction<void(const void*)>&& callback)
{
	FListener& listener = listeners.FindOrAdd(channel).AddDefaulted_GetRef();
	listener.id = ++lastListenerId;
	listener.type = type;
	listener.callback = MoveTemp(callback);

	FCDListenerHandle handle;
	handle.channel = channel;
	handle.id = listener.id;
	return handle;
}

void UCDMessageSubsystem::Unlisten(FCDListenerHandle& handle)
{
	if (TArray<FListener>* channelListeners = listeners.Find(handle.channel))
	{
		const int32 targetId = handle.id;
		auto IsTarget = [targetId](const FListener& listener)
		{
			return listener.id == targetId;
		};
		channelListeners->RemoveAll(IsTarget);
	}
	handle = FCDListenerHandle();
}
