#include "World/CDPortal.h"
#include "Components/BoxComponent.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "GameFramework/Pawn.h"
#include "Core/CDRunSubsystem.h"

ACDPortal::ACDPortal()
{
	trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	trigger->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = trigger;
}

void ACDPortal::BeginPlay()
{
	Super::BeginPlay();

	// 진입 감지는 클리어 전까지 비활성화한다.
	SetOpen(false);

	trigger->SetGenerateOverlapEvents(true);
	trigger->OnComponentBeginOverlap.AddDynamic(
		this,
		&ThisClass::OnTriggerOverlap);

	// 입장 레벨 문은 바로 사용 가능
	if (portalType == ECDPortalType::DungeonEntrance)
	{
		SetOpen(true);
	}

	else
	{
	clearedHandle = UCDMessageSubsystem::Get(this).Listen(
		CDTags::Msg_Stage_Cleared,
		this,
		&ThisClass::HandleStageCleared);
	}
}

void ACDPortal::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	UCDMessageSubsystem::Get(this).Unlisten(clearedHandle);
	Super::EndPlay(endPlayReason);
}

void ACDPortal::HandleStageCleared(const FCDStageMessage& message)
{
	SetOpen(true);
	OnOpened();
}

void ACDPortal::SetOpen(bool bOpen)
{
	bCanEnter = bOpen;

	trigger->SetCollisionEnabled(bOpen ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ACDPortal::OnTriggerOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
	int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	const APawn* pawn = Cast<APawn>(otherActor);

	if (bCanEnter && !bEntered && pawn && pawn->IsPlayerControlled())
	{
		// 이동 요청을 한 번만 보낸다.
		bEntered = true;
		SetOpen(false);

		if (portalType == ECDPortalType::DungeonEntrance)
		{
			// 새 던전 진행을 시작한다.
			UCDRunSubsystem* run =
				GetGameInstance()->GetSubsystem<UCDRunSubsystem>();

			run->StartRun(ECDDifficulty::Normal);
		}
		else
		{
			// 현재 던전의 다음 단계 이동을 요청한다.
			UCDMessageSubsystem::Get(this).Broadcast(
				CDTags::Msg_Stage_PortalEntered,
				FCDStageMessage());
		}
	}
}
