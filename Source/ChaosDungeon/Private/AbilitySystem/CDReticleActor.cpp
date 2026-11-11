// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CDReticleActor.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

ACDReticleActor::ACDReticleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
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

	/* 장판 채우기 효과를 위한 추가 - yoonsang */
	if (!IsValid(reticleMID))
	{
		SetActorTickEnabled(false);
		return;
	}

	fillElapsedTime += DeltaTime;

	const float fillAmount = FMath::Clamp(fillElapsedTime / fillDuration, 0.f, 1.f);

	reticleMID->SetScalarParameterValue(TEXT("FillAmount"), fillAmount);

	if (fillAmount >= 1.f)
	{
		SetActorTickEnabled(false);
	}

}

void ACDReticleActor::SetReticleConfig(const UCDSkillData& skillData, bool bReuseMaterial)
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


	// Scatter 풀의 표시는 같은 MID를 재사용한다. 일반 표시는 기존 생성 경로 유지.
	if (!bReuseMaterial || !IsValid(reticleMID) || reticleMID->Parent != *material)
	{
		reticleMID = UMaterialInstanceDynamic::Create(*material, this);
	}

	SetActorTickEnabled(false);
	fillElapsedTime = 0.f;

	reticleMID->SetScalarParameterValue(TEXT("FillAmount"), 1.f);

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

	/* 몬스터 전용 Circle을 위한 처리 항상 원을 표시하기 위해 고정 값 사용 - yoonsang */
	else if (skillData.hitShape.type == ECDHitShapeType::MonsterCircle)
	{
		SetConeReticleSize(skillData.hitShape.radius, 360.f);
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

void ACDReticleActor::StartFill(float duration)
{
	fillElapsedTime = 0.f;

	fillDuration = duration;

	reticleMID->SetScalarParameterValue(TEXT("FillAmount"), 0.f);

	SetActorTickEnabled(true);
}
