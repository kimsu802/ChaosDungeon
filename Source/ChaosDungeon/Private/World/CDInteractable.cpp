#include "World/CDInteractable.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Core/CDMessageSubsystem.h"
#include "Core/CDTypes.h"
#include "UI/CDUIManagerSubsystem.h"

ACDInteractable::ACDInteractable()
{
}

void ACDInteractable::Interact(APlayerController* interactor)
{
    UCDUIManagerSubsystem* uiManager =
        UCDUIManagerSubsystem::Get(interactor);

    if (!uiManager)
    {
        return;
    }

    /* 일반 NPC와 입장 문은 지정된 화면을 표시 */
    if (!bUsePortalInteraction
        || portalType == ECDPortalType::DungeonEntrance)
    {
        if (!screenClass.IsNull())
        {
            uiManager->PushScreen(
                CDTags::UI_Layer_Modal,
                screenClass);
        }

        return;
    }

    
    if (bPortalRequested)
    {
        return;
    }

    bPortalRequested = true;

    auto HandleConfirmResult =
        [weakThis = TWeakObjectPtr<ACDInteractable>(this)]
        (ECDConfirmResult result)
        {
            ACDInteractable* door = weakThis.Get();

            if (!door || !door->HasActorBegunPlay())
            {
                return;
            }

            // 취소하거나 닫으면 다시 상호작용할 수 있다.
            if (result != ECDConfirmResult::Confirmed)
            {
                door->bPortalRequested = false;
                return;
            }

            // 기존 CDPortal이 사용하던 이동 요청을 그대로 보냄
            UCDMessageSubsystem::Get(door).Broadcast(
                CDTags::Msg_Stage_PortalEntered,
                FCDStageMessage());
        };

    uiManager->ShowConfirm(
        ECDConfirmType::OkCancel,
        FText::FromString(TEXT("다음 스테이지")),
        FText::FromString(TEXT("다음 스테이지로 이동하시겠습니까?")),
        FText::FromString(TEXT("클리어 전까지 저장/세이브가 되지 않습니다")),
        MoveTemp(HandleConfirmResult));
}
