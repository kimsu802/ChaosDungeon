#include "World/CDPortal.h"
#include "Components/BoxComponent.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "GameFramework/Pawn.h"

ACDPortal::ACDPortal()
{
	trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	trigger->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = trigger;
}

void ACDPortal::BeginPlay()
{
	Super::BeginPlay();
	trigger->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnTriggerOverlap);
	clearedHandle = UCDMessageSubsystem::Get(this).Listen(CDTags::Msg_Stage_Cleared, this, &ThisClass::HandleStageCleared);
	SetOpen(false);
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
	SetActorHiddenInGame(!bOpen);
	SetActorEnableCollision(bOpen);
}

void ACDPortal::OnTriggerOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
	int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	const APawn* pawn = Cast<APawn>(otherActor);
	if (!bEntered && pawn && pawn->IsPlayerControlled())
	{
		bEntered = true;
		UCDMessageSubsystem::Get(this).Broadcast(CDTags::Msg_Stage_PortalEntered, FCDStageMessage());
	}
}
