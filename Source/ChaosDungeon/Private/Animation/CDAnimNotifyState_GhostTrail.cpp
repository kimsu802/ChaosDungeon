#include "Animation/CDAnimNotifyState_GhostTrail.h"
#include "Animation/CDGhostTrailActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

UCDAnimNotifyState_GhostTrail::UCDAnimNotifyState_GhostTrail()
{
	ghostClass = ACDGhostTrailActor::StaticClass();

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(120, 200, 255);
#endif
}

void UCDAnimNotifyState_GhostTrail::NotifyBegin(USkeletalMeshComponent* meshComp, UAnimSequenceBase* animation, float totalDuration, const FAnimNotifyEventReference& eventReference)
{
	Super::NotifyBegin(meshComp, animation, totalDuration, eventReference);

	if (meshComp == nullptr)
	{
		return;
	}

	// 이전 구간이 NotifyEnd 없이 끊겼어도 여기서 새로 시작
	timeUntilNext.Add(meshComp, spawnInterval);

	if (bSpawnOnBegin)
	{
		SpawnGhost(meshComp);
	}
}

void UCDAnimNotifyState_GhostTrail::NotifyTick(USkeletalMeshComponent* meshComp, UAnimSequenceBase* animation, float frameDeltaTime, const FAnimNotifyEventReference& eventReference)
{
	Super::NotifyTick(meshComp, animation, frameDeltaTime, eventReference);

	float* remaining = timeUntilNext.Find(meshComp);
	if (remaining == nullptr)
	{
		return;
	}

	// frameDeltaTime 은 애니메이션 진행 시간 → 슬로우 · 일시정지 중에는 잔영도 그만큼 덜 생긴다
	*remaining -= frameDeltaTime;
	if (*remaining > 0.f)
	{
		return;
	}

	// 한 프레임이 간격보다 길어도 같은 포즈라 1개만 생성
	*remaining = spawnInterval;
	SpawnGhost(meshComp);
}

void UCDAnimNotifyState_GhostTrail::NotifyEnd(USkeletalMeshComponent* meshComp, UAnimSequenceBase* animation, const FAnimNotifyEventReference& eventReference)
{
	timeUntilNext.Remove(meshComp);

	Super::NotifyEnd(meshComp, animation, eventReference);
}

FString UCDAnimNotifyState_GhostTrail::GetNotifyName_Implementation() const
{
	return TEXT("Ghost Trail");
}

void UCDAnimNotifyState_GhostTrail::SpawnGhost(USkeletalMeshComponent* meshComp) const
{
	UWorld* world = meshComp->GetWorld();
	if ((world == nullptr) || (ghostClass == nullptr) || (ghostMaterial == nullptr) || (meshComp->GetSkinnedAsset() == nullptr))
	{
		return;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = meshComp->GetOwner();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 메시 컴포넌트 위치 그대로 (캡슐 오프셋 · 회전 포함). 페르소나 미리보기 월드에서도 생성돼 바로 튜닝 가능
	ACDGhostTrailActor* ghost = world->SpawnActor<ACDGhostTrailActor>(ghostClass, meshComp->GetComponentTransform(), spawnParams);
	if (ghost != nullptr)
	{
		ghost->Init(meshComp, ghostMaterial, ghostLifeTime);
	}
}
