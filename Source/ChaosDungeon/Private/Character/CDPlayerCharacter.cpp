#include "Character/CDPlayerCharacter.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AbilitySystem/CDDodgeAbility.h"
#include "Camera/CameraComponent.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDTypes.h"
#include "Data/CDCharacterClassData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/CDOcclusionFadeComponent.h"

ACDPlayerCharacter::ACDPlayerCharacter()
{
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// 기획 확정값: Roll/Pitch/Yaw (0, 290, 208), ArmLength 2000, FOV 55~60, 회전 고정
	cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	cameraBoom->SetupAttachment(RootComponent);
	cameraBoom->SetUsingAbsoluteRotation(true);
	cameraBoom->SetRelativeRotation(FRotator(-70.f, 208.f, 0.f));
	cameraBoom->TargetArmLength = 2000.f;
	// 벽은 OcclusionFade(반투명)로 처리
	cameraBoom->bDoCollisionTest = false;

	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	camera->SetupAttachment(cameraBoom, USpringArmComponent::SocketName);
	camera->SetFieldOfView(90.f);
	camera->bUsePawnControlRotation = false;

	occlusionFade = CreateDefaultSubobject<UCDOcclusionFadeComponent>(TEXT("OcclusionFade"));

	// 벽 뒤 실루엣: 항상 CustomDepth 렌더 (포스트프로세스 머티리얼에서 처리)
	GetMesh()->SetRenderCustomDepth(false);
	GetMesh()->SetCustomDepthStencilValue(CDStencil::Player);
}

void ACDPlayerCharacter::PossessedBy(AController* newController)
{
	Super::PossessedBy(newController);

	check(classData);
	InitializeAbilitySystem(classData->stats);
	for (const FCDSkillSlot& slot : classData->defaultSkills)
	{
		abilitySystem->GrantSkill(slot.skill, slot.inputTag);
	}
	abilitySystem->GrantAbility(classData->dodgeAbility, CDTags::Input_Dodge);
}

FVector ACDPlayerCharacter::GetAimLocation() const
{
	FHitResult hit;
	const APlayerController* playerController = GetController<APlayerController>();
	if (playerController && playerController->GetHitResultUnderCursor(ECC_Visibility, false, hit))
	{
		return hit.Location;
	}
	return Super::GetAimLocation();
}
