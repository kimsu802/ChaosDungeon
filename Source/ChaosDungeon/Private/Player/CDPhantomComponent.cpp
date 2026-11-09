#include "Player/CDPhantomComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Animation/CDGhostTrailActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/CDGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

UCDPhantomComponent::UCDPhantomComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	phantomTag = CDTags::State_Buff_Phantom;
	ghostClass = ACDGhostTrailActor::StaticClass();
}

void UCDPhantomComponent::BeginPlay()
{
	Super::BeginPlay();

	abilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if ((abilitySystem == nullptr) || (phantomTag.IsValid() == false))
	{
		return;
	}

	tagChangedHandle = abilitySystem->RegisterGameplayTagEvent(phantomTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ThisClass::HandlePhantomTagChanged);

	// 이미 버프가 걸려 있으면 (리스폰 등) 바로 반영
	HandlePhantomTagChanged(phantomTag, abilitySystem->GetTagCount(phantomTag));
}

void UCDPhantomComponent::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(trailTimer);
	}

	if (abilitySystem != nullptr)
	{
		abilitySystem->RegisterGameplayTagEvent(phantomTag, EGameplayTagEventType::NewOrRemoved).Remove(tagChangedHandle);
	}

	Super::EndPlay(endPlayReason);
}

bool UCDPhantomComponent::QueueEcho(const UObject* context, TFunction<void()> echo)
{
	if ((bPhantomActive == false) || (context == nullptr))
	{
		return false;
	}

	// 때린 순간의 포즈로 잔영 → echoDelay 뒤 이 잔영이 때리는 것처럼 보인다
	SpawnGhost(echoGhostLifeTime);

	// 요청한 어빌리티가 그 사이 사라졌으면 실행하지 않음
	const TWeakObjectPtr<const UObject> weakContext(context);
	const FTimerDelegate echoDelegate = FTimerDelegate::CreateWeakLambda(this, [weakContext, echo]()
		{
			if (weakContext.IsValid())
			{
				echo();
			}
		});

	FTimerHandle echoTimer;
	GetWorld()->GetTimerManager().SetTimer(echoTimer, echoDelegate, echoDelay, false);
	return true;
}

void UCDPhantomComponent::HandlePhantomTagChanged(const FGameplayTag tag, int32 newCount)
{
	const bool bActive = (newCount > 0);
	if (bActive == bPhantomActive)
	{
		return;
	}
	bPhantomActive = bActive;

	FTimerManager& timers = GetWorld()->GetTimerManager();
	if (bPhantomActive)
	{
		SpawnTrailGhost();
		timers.SetTimer(trailTimer, this, &ThisClass::SpawnTrailGhost, trailInterval, true);
		return;
	}

	// 버프 종료: 새 잔영만 멈춘다 (이미 남은 잔영은 스스로 사라짐)
	timers.ClearTimer(trailTimer);
}

void UCDPhantomComponent::SpawnTrailGhost()
{
	SpawnGhost(trailLifeTime);
}

void UCDPhantomComponent::SpawnGhost(float lifeTime) const
{
	const ACharacter* character = Cast<ACharacter>(GetOwner());
	USkeletalMeshComponent* mesh = character ? character->GetMesh() : nullptr;
	UWorld* world = GetWorld();
	if ((mesh == nullptr) || (world == nullptr) || (ghostClass == nullptr) || (ghostMaterial == nullptr))
	{
		return;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = GetOwner();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACDGhostTrailActor* ghost = world->SpawnActor<ACDGhostTrailActor>(ghostClass, mesh->GetComponentTransform(), spawnParams);
	if (ghost != nullptr)
	{
		ghost->Init(mesh, ghostMaterial, lifeTime);
	}
}
