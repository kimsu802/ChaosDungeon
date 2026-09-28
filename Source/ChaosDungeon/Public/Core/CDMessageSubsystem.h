#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "CDMessageSubsystem.generated.h"

/** Listen 이 돌려주는 핸들. 구독 해제에 사용 */
struct CHAOSDUNGEON_API FCDListenerHandle
{
	/** 유효한 핸들인가 */
	FORCEINLINE bool IsValid() const
	{
		return id != 0;
	}

	/** 구독한 채널 */
	FGameplayTag channel;

	/** 리스너 고유 번호 (0 = 무효) */
	int32 id = 0;
};

/**
 * 태그 채널 메시지 버스. 로직 없음.
 * - 발행: messages.Broadcast(CDTags::Msg_Stage_Cleared, stageMessage);
 * - 구독: handle = messages.Listen(CDTags::Msg_Stage_Cleared, this, &ThisClass::HandleCleared);
 * - 해제: EndPlay / Deinitialize 에서 messages.Unlisten(handle);
 * 새 이벤트가 필요하면 태그(CDGameplayTags) + 구조체(CDMessages.h)만 추가한다.
 * GameInstance 수명이라 레벨 이동 중에도 유지된다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 월드 컨텍스트로 서브시스템 얻기 */
	static UCDMessageSubsystem& Get(const UObject* worldContextObject);

	/** 채널에 메시지 발행 */
	template <typename TMessage>
	void Broadcast(FGameplayTag channel, const TMessage& message)
	{
		BroadcastInternal(channel, TMessage::StaticStruct(), &message);
	}

	/** 채널 구독. 구독자가 파괴되면 콜백은 자동으로 무시된다 (그래도 핸들 해제 권장) */
	template <typename TMessage, typename TOwner>
	FCDListenerHandle Listen(FGameplayTag channel, TOwner* owner, void (TOwner::*func)(const TMessage&))
	{
		TWeakObjectPtr<TOwner> weakOwner(owner);
		auto Callback = [weakOwner, func](const void* payload)
		{
			if (TOwner* strongOwner = weakOwner.Get())
			{
				(strongOwner->*func)(*static_cast<const TMessage*>(payload));
			}
		};
		return ListenInternal(channel, TMessage::StaticStruct(), MoveTemp(Callback));
	}

	/** 구독 해제 (핸들은 무효화된다) */
	void Unlisten(FCDListenerHandle& handle);

private:
	/** 타입 확인 후 채널의 모든 리스너 호출 */
	void BroadcastInternal(FGameplayTag channel, const UScriptStruct* type, const void* payload);

	/** 리스너 등록 */
	FCDListenerHandle ListenInternal(FGameplayTag channel, const UScriptStruct* type, TFunction<void(const void*)>&& callback);

private:
	/** 등록된 리스너 1개 */
	struct FListener
	{
		/** 리스너 고유 번호 */
		int32 id = 0;

		/** 메시지 구조체 타입 (잘못된 타입 발행 검출용) */
		TObjectPtr<const UScriptStruct> type;

		/** 호출할 콜백 */
		TFunction<void(const void*)> callback;
	};

	/** 채널별 리스너 목록 */
	TMap<FGameplayTag, TArray<FListener>> listeners;

	/** 마지막으로 발급한 리스너 번호 */
	int32 lastListenerId = 0;
};
