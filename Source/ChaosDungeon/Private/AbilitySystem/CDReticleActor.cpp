// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CDReticleActor.h"
#include "Components/DecalComponent.h"
#include "Data/CDSkillData.h"
#include "Materials/MaterialInstance.h"

ACDReticleActor::ACDReticleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	RootComponent = root;

	skillReticle = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal Component"));
	skillReticle->SetupAttachment(RootComponent);
	skillReticle->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
}

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

void ACDReticleActor::SetReticleConfig(const UCDSkillData* skillData)
{
	if (nullptr == skillData)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillData for Reticle is nullptr"));
	}

	reticleMID = UMaterialInstanceDynamic::Create(skillData->reticleConfig.reticleMaterial, this);

	reticleMID->SetVectorParameterValue(FName("BaseColor"), skillData->reticleConfig.reticleColor);

	skillReticle->SetDecalMaterial(reticleMID);


	if (skillData->hitShape.type == ECDHitShapeType::Circle)
	{
		skillReticle->DecalSize.Y = skillData->hitShape.radius + 50.0f;
		skillReticle->DecalSize.Z = skillData->hitShape.radius + 50.0f;
	}
}