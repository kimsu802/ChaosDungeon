#include "World/CDPickupOrb.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Core/CDMessageSubsystem.h"
#include "GameFramework/Pawn.h"

ACDPickupOrb::ACDPickupOrb()
{
	PrimaryActorTick.bCanEverTick = true;
	// 플레이어가 가까이 오면 켠다
	PrimaryActorTick.bStartWithTickEnabled = false;

	magnetSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MagnetSphere"));
	magnetSphere->SetSphereRadius(300.f);
	magnetSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	magnetSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnMagnetOverlap);
	RootComponent = magnetSphere;
}

void ACDPickupOrb::OnMagnetOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
	int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	APawn* pawn = Cast<APawn>(otherActor);
	if (pawn && pawn->IsPlayerControlled() && !target.IsValid())
	{
		target = pawn;
		SetActorTickEnabled(true);
	}
}

void ACDPickupOrb::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	const APawn* pawn = target.Get();
	if (!pawn)
	{
		Destroy();
		return;
	}
	const FVector newLocation = FMath::VInterpConstantTo(GetActorLocation(), pawn->GetActorLocation(), deltaSeconds, flySpeed);
	SetActorLocation(newLocation);

	if (FVector::Dist(newLocation, pawn->GetActorLocation()) <= absorbDistance)
	{
		Absorb();
	}
}

void ACDPickupOrb::Absorb()
{
	APawn* pawn = target.Get();
	UAbilitySystemComponent* abilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(pawn);
	if (abilitySystem && effect)
	{
		abilitySystem->ApplyGameplayEffectToSelf(effect.GetDefaultObject(), 1.f, abilitySystem->MakeEffectContext());
	}
	if (!pickupText.IsEmpty())
	{
		FCDWorldTextMessage message;
		message.location = pawn->GetActorLocation();
		message.text = pickupText;
		message.style = ECDFloatingTextStyle::Buff;
		UCDMessageSubsystem::Get(this).Broadcast(CDTags::Msg_UI_FloatingText, message);
	}
	Destroy();
}
