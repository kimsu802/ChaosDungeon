// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CDReticleActor.h"
#include "Components/DecalComponent.h"

ACDReticleActor::ACDReticleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	RootComponent = root;

	skillReticle = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal Component"));
	skillReticle->SetupAttachment(RootComponent);
	skillReticle->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
}

// Called when the game starts or when spawned
void ACDReticleActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACDReticleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* pc = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (nullptr == pc)
	{
		return;
	}

	FHitResult result;
	if (pc->GetHitResultUnderCursor(ECC_Visibility, false, result))
	{
		SetActorLocation(result.ImpactPoint);
	}
}

void ACDReticleActor::SetReticleConfig(const FCDHitShape& inHitShape)
{
	if (inHitShape.type == ECDHitShapeType::Circle)
	{
		skillReticle->DecalSize.Y = inHitShape.radius + 50.0f;
		skillReticle->DecalSize.Z = inHitShape.radius + 50.0f;
	}
}