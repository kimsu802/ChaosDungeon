#include "Animation/CDGhostTrailActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/App.h"

ACDGhostTrailActor::ACDGhostTrailActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetActorEnableCollision(false);

	poseMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseMesh"));
	RootComponent = poseMesh;

	// 보이기만 하는 메시: 충돌 · 오버랩 · 내비 · 그림자 · 데칼 모두 끔
	poseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	poseMesh->SetGenerateOverlapEvents(false);
	poseMesh->SetCanEverAffectNavigation(false);
	poseMesh->SetCastShadow(false);
	poseMesh->SetReceivesDecals(false);
}

void ACDGhostTrailActor::Init(USkeletalMeshComponent* source, UMaterialInterface* ghostMaterial, float inLifeTime)
{
	if ((source == nullptr) || (source->GetSkinnedAsset() == nullptr))
	{
		Destroy();
		return;
	}

	// 0 으로 나누기 방지
	lifeTime = FMath::Max(inLifeTime, 0.01f);
	elapsed = 0.f;

	// 같은 메시여야 본 이름이 맞아 포즈 복사가 된다
	poseMesh->SetSkinnedAssetAndUpdate(source->GetSkinnedAsset());
	poseMesh->SetWorldTransform(source->GetComponentTransform());
	poseMesh->CopyPoseFromSkeletalComponent(source);

	if (ghostMaterial == nullptr)
	{
		return;
	}

	ghostMaterialInstance = UMaterialInstanceDynamic::Create(ghostMaterial, this);
	ghostMaterialInstance->SetScalarParameterValue(opacityParamName, startOpacity);

	for (int32 i = 0; i < poseMesh->GetNumMaterials(); ++i)
	{
		poseMesh->SetMaterial(i, ghostMaterialInstance);
	}
}

void ACDGhostTrailActor::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	// deltaTime 은 시간 감속이 적용된 값. 실제 시간이 필요하면 감속 전 프레임 시간 사용
	elapsed += bUseRealTime ? static_cast<float>(FApp::GetDeltaTime()) : deltaTime;

	const float alpha = 1.f - FMath::Clamp(elapsed / lifeTime, 0.f, 1.f);
	if (ghostMaterialInstance != nullptr)
	{
		ghostMaterialInstance->SetScalarParameterValue(opacityParamName, startOpacity * alpha);
	}

	if (elapsed >= lifeTime)
	{
		Destroy();
	}
}
