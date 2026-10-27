// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CDReticleActor.h"
#include "Components/DecalComponent.h"
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

}

void ACDReticleActor::SetReticleConfig(const UCDSkillData& skillData)
{
	//if (nullptr == skillData)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("SkillData for Reticle is nullptr"));
	//}

	auto material = shapeMaterials.Find(skillData.hitShape.type);
	if (material == nullptr || nullptr == *material)
	{
		UE_LOG(LogTemp, Error, TEXT("shapeMaterials is null"));
		return;
	}


	reticleMID = UMaterialInstanceDynamic::Create(*material, this);

	reticleMID->SetVectorParameterValue(FName("BaseColor"), skillData.reticleConfig.reticleColor);

	skillReticle->SetDecalMaterial(reticleMID);


	if (skillData.hitShape.type == ECDHitShapeType::Circle)
	{
		SetReticleSize(skillData.hitShape.radius, skillData.hitShape.radius);
	}
	else if (skillData.hitShape.type == ECDHitShapeType::Cone)
	{
		SetConeReticleSize(skillData.hitShape.radius, skillData.hitShape.angle);
	}
	else
	{
		SetReticleSize(skillData.hitShape.width * 0.5f, skillData.hitShape.length * 0.5f);
	}
}

void ACDReticleActor::SetReticleSize(const float newHorizontalSize, const float newVerticalSize)
{
	if (newHorizontalSize >= 0)
	{
		skillReticle->DecalSize.Y = newHorizontalSize;
	}
	if (newVerticalSize >= 0)
	{
		skillReticle->DecalSize.Z = newVerticalSize;
	}
}

void ACDReticleActor::SetConeReticleSize(const float radius, const float degree)
{
	reticleMID->SetScalarParameterValue(FName("ConeAngle"), degree);
	skillReticle->DecalSize.Y = radius;
	skillReticle->DecalSize.Z = radius;
}

void ACDReticleActor::SetValidColor(bool bIsValid)
{
	reticleMID->SetScalarParameterValue(FName("IsValid"), bIsValid ? 1.0f : 0.0f);
}
