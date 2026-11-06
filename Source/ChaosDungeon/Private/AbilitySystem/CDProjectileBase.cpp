// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CDProjectileBase.h"
#include "AbilitySystem/CDReticleActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Core/CDGameplayTags.h"
#include <Components/SphereComponent.h>
#include <Components/StaticMeshComponent.h>
#include <Character/CDCharacterBase.h>
#include "Engine/World.h"
#include "Engine/HitResult.h"
#include <GameFramework/ProjectileMovementComponent.h>
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include <Kismet/GameplayStatics.h>


// Sets default values
ACDProjectileBase::ACDProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	sphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("sphereComp"));
	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("meshComp"));
	niagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("niagaraComp"));
	projectileMove = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("projectileMove"));

	SetRootComponent(sphereComp);
	meshComp->SetupAttachment(sphereComp);
	niagaraComp->SetupAttachment(sphereComp);
	projectileMove->SetUpdatedComponent(sphereComp);
	sphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	meshComp->SetSimulatePhysics(false);
	projectileMove->SetAutoActivate(false);
	niagaraComp->SetAutoActivate(false);
}

void ACDProjectileBase::SetLandingReticle(ACDReticleActor* reticle)
{
	landingReticle = reticle;
}

void ACDProjectileBase::PauseFlight()
{
	projectileMove->StopMovementImmediately();
	SetFlightHitEnabled(false);
}

void ACDProjectileBase::LaunchAtLocation(
	const FVector& location,
	float speed,
	const FGameplayTag& launchCueTag)
{
	projectileData.targetPoint = location;
	projectileData.speed = speed;
	projectileData.projectileType = ECDProjectileType::Straight;

	if (!ConfigureMovement())
	{
		Destroy();
		return;
	}

	// 현재 위치부터 명중 검사 시작
	SetFlightHitEnabled(true);
	projectileMove->Activate(true);

	UAbilitySystemComponent* sourceASC =
		projectileData.sourceASC.Get();

	if (launchCueTag.IsValid() && IsValid(sourceASC))
	{
		FGameplayCueParameters params;
		params.Location = GetActorLocation();
		params.Normal = projectileMove->Velocity.GetSafeNormal();
		params.Instigator = projectileData.sourceActor.Get();
		params.EffectCauser = this;

		sourceASC->ExecuteGameplayCue(launchCueTag, params);
	}
}

void ACDProjectileBase::InitializeProjectile(const FCDProjectileBaseData& inData)
{
	projectileData = inData;

	SetActorLocation(projectileData.launchLocation);

	sphereComp->SetSphereRadius(projectileData.hitRadius, true);

	SetLifeSpan(projectileData.lifeTime);

	if (!ConfigureMovement())
	{
		Destroy();
		return;
	}

	sphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	sphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	sphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	previousLocation = GetActorLocation();
	bTraceEnabled = projectileData.bHitActorsFlight;

	projectileMove->Activate();
	niagaraComp->Activate();
}

bool ACDProjectileBase::ConfigureMovement()
{
	const FVector startLocation = GetActorLocation();
	const FVector toTarget = projectileData.targetPoint - startLocation;
	const float speed = projectileData.speed;

	if (speed <= 0.f || toTarget.IsNearlyZero())
	{
		return false;
	}

	projectileMove->InitialSpeed = speed;
	projectileMove->bInitialVelocityInLocalSpace = false;
	projectileMove->bRotationFollowsVelocity = true;
	projectileMove->bShouldBounce = false;
	projectileMove->bIsHomingProjectile = false;

	FVector launchVelocity = FVector::ZeroVector;

	switch (projectileData.projectileType)
	{
	case ECDProjectileType::Straight:
	{
		// 중력 없이 목표 방향으로 직진
		projectileMove->ProjectileGravityScale = 0.f;
		projectileMove->MaxSpeed = speed;

		launchVelocity = toTarget.GetSafeNormal() * speed;
		break;
	}

	case ECDProjectileType::Parabolic:
	{
		projectileMove->ProjectileGravityScale =
			projectileData.ProjectileGravityScale;

		// 이동 컴포넌트가 실제 적용하는 중력과 동일하게 계산
		const float gravityZ = projectileMove->GetGravityZ();

		if (gravityZ >= -KINDA_SMALL_NUMBER)
		{
			return false;
		}

		UGameplayStatics::FSuggestProjectileVelocityParameters params(
			this,
			startLocation,
			projectileData.targetPoint,
			speed
		);

		params.bFavorHighArc = true;
		params.OverrideGravityZ = gravityZ;
		params.CollisionRadius = projectileData.hitRadius;
		params.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;

		if (!UGameplayStatics::SuggestProjectileVelocity(
			params, launchVelocity))
		{
			return false;
		}

		// 하강 중 중력에 의해 속도가 증가할 수 있도록 제한 해제
		projectileMove->MaxSpeed = 0.f;
		break;
	}

	default:
		return false;
	}

	projectileMove->Velocity = launchVelocity;
	SetActorRotation(launchVelocity.Rotation());

	return true;
}

void ACDProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// 이동 컴포넌트가 위치를 갱신한 다음 Actor의 Tick 실행
	AddTickPrerequisiteComponent(projectileMove);

	// 이동 중단 이벤트 
	projectileMove->OnProjectileStop.AddDynamic(this, &ACDProjectileBase::OnProjectileStop);
}

void ACDProjectileBase::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	if (IsValid(landingReticle.Get()))
	{
		landingReticle->Destroy();
		landingReticle = nullptr;
	}

	Super::EndPlay(endPlayReason);
}

void ACDProjectileBase::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	if (!bTraceEnabled)
	{
		return;
	}

	const FVector currentLocation = GetActorLocation();

	TraceHit(previousLocation, currentLocation);

	// 다음 프레임에서 사용할 위치 저장
	previousLocation = currentLocation;
}

void ACDProjectileBase::TraceHit(const FVector& start, const FVector& end)
{
	const ACDCharacterBase* sourceCharacter = Cast<ACDCharacterBase>(projectileData.sourceActor.Get());

	if (!IsValid(sourceCharacter))
	{
		return;
	}

	// 검사에서 투사체 자신과 발사자를 제외
	FCollisionQueryParams queryParams(
		SCENE_QUERY_STAT(ProjectileTrace),
		false,
		this
	);

	queryParams.AddIgnoredActor(sourceCharacter);

	TArray<FHitResult> hitResults;

	// 투사체의 실제 Sphere 크기로 Pawn을 검사
	GetWorld()->SweepMultiByObjectType(
		hitResults,
		start,
		end,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(
			sphereComp->GetScaledSphereRadius()),
		queryParams
	);

	for (const FHitResult& hit : hitResults)
	{
		AActor* target = hit.GetActor();

		if (!IsValid(target) || !sourceCharacter->IsHostileTo(target))
		{
			continue;
		}

		// 기본 동작은 한 발당 적 한 명만 명중
		bTraceEnabled = false;

		OnTargetHit(hit);
		return;
	}
}

void ACDProjectileBase::OnTargetHit(const FHitResult& hit)
{
	AActor* target = hit.GetActor();

	UAbilitySystemComponent* sourceASC = projectileData.sourceASC.Get();

	UAbilitySystemComponent* targetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(target);

	if (!IsValid(target) ||
		!IsValid(sourceASC) ||
		!IsValid(targetASC) ||
		!projectileData.damageSpec.IsValid())
	{
		Destroy();
		return;
	}

	// 데미지 적용
	sourceASC->ApplyGameplayEffectSpecToTarget(*projectileData.damageSpec.Data, targetASC);

	// 10.28 Jun6 - 각성기 게이지 관련 코드 추가
	sourceASC->ApplyGameplayEffectSpecToSelf(*projectileData.awakenSpec.Data);

	// 데미지 적용 후 피격 이벤트 전송
	AActor* sourceActor = projectileData.sourceActor.Get();

	if (projectileData.bCausesHitReact &&
		IsValid(sourceActor) &&
		IsValid(target))
	{
		FGameplayEventData payload;
		payload.Instigator = sourceActor;
		payload.Target = target;
		payload.EventMagnitude = projectileData.knockbackStrength;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(target, CDTags::Event_Hit, payload);
	}

	if (impactCueTag.IsValid())
	{
		FGameplayCueParameters params;
		params.Location = hit.ImpactPoint;
		params.Normal = hit.ImpactNormal;
		params.Instigator = projectileData.sourceActor.Get();
		params.EffectCauser = this;

		sourceASC->ExecuteGameplayCue(impactCueTag, params);
	}

	if (hitComp)
	{
		// 피격 나이아가라 생성 (자동 제거)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, hitComp, hit.ImpactPoint, FRotator::ZeroRotator, FVector(1, 1, 1), true);
	}
	Destroy();
}

void ACDProjectileBase::OnProjectileStop(const FHitResult& hit)
{
	if (bTraceEnabled)
	{
		TraceHit(previousLocation, GetActorLocation());

		// 앞에 있던 적을 맞혀 이미 제거 중이라면 종료
		if (IsActorBeingDestroyed())
		{
			return;
		}
	}

	bTraceEnabled = false;

	// GCN 태그 여부를 확인해서 사라지는 여부 확인
	// 태그가 없으면 그냥 Destroy()
	if (impactCueTag.IsValid())
	{
		UAbilitySystemComponent* sourceASC = projectileData.sourceASC.Get();

		if (IsValid(sourceASC))
		{
			FGameplayCueParameters params;
			params.Location = hit.ImpactPoint;
			params.Normal = hit.ImpactNormal;
			params.Instigator = projectileData.sourceActor.Get();
			params.EffectCauser = this;

			sourceASC->ExecuteGameplayCue(impactCueTag, params);
		}
	}

	Destroy();
}




