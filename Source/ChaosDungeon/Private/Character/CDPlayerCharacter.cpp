#include "Character/CDPlayerCharacter.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AbilitySystem/CDDodgeAbility.h"
#include "AbilitySystem/CDAttackAbility.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDSkillLoadoutSubsystem.h"
#include "Core/CDPlayerClassSubsystem.h"
#include "Core/CDTypes.h"
#include "Data/CDCharacterClassData.h"
#include "Player/CDPlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Player/CDOcclusionFadeComponent.h"
#include "Player/CDAwakenDirectorComponent.h"
#include "Player/CDSpeechBubbleComponent.h"
#include "Player/CDPhantomComponent.h"

ACDPlayerCharacter::ACDPlayerCharacter()
{
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// 09.30 Jun6 - Anim 설정을 위해 Nav Movement의 Use Accelration for Paths 변경
	GetMovementComponent()->GetNavMovementProperties()->bUseAccelerationForPaths = true;

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
	awakenDirector = CreateDefaultSubobject<UCDAwakenDirectorComponent>(TEXT("AwakenDirector"));

	// 머리 위 대사창: 메시의 head 소켓 (소켓 이름이 다르면 BP 에서 Parent Socket 변경)
	speechBubble = CreateDefaultSubobject<UCDSpeechBubbleComponent>(TEXT("SpeechBubble"));
	speechBubble->SetupAttachment(GetMesh(), TEXT("head"));

	// 11.03 환영 버프: State.Buff.Phantom 동안 노란 잔영 + 스킬 · 평타 추가 타격
	phantom = CreateDefaultSubobject<UCDPhantomComponent>(TEXT("Phantom"));

	// 벽 뒤 실루엣: 항상 CustomDepth 렌더 (포스트프로세스 머티리얼에서 처리)
	GetMesh()->SetRenderCustomDepth(false);
	GetMesh()->SetCustomDepthStencilValue(CDStencil::Player);

	// 09.30 Jun6 - Animation 추가
	static ConstructorHelpers::FClassFinder<UAnimInstance> anim(TEXT("/Game/Character/Player/ABP_CDPlayer.ABP_CDPlayer_C"));
	if (anim.Succeeded())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(anim.Class);
	}
}

void ACDPlayerCharacter::PossessedBy(AController* newController)
{
	Super::PossessedBy(newController);

	// 10.30 - 직업 선택: 어빌리티 · 스탯을 만들기 전에 직업부터 확정
	ApplySelectedClass();

	check(classData);
	InitializeAbilitySystem(classData->stats);
}

void ACDPlayerCharacter::GrantOwnedAbilities()
{
	if (!bAbilitySystemInitialized)
	{
		bAbilitySystemInitialized = true;

		GrantHitReactAbility();

		// 10.02 - Jun6 캐릭터 기본 데이터 구조체 구조 변경에 따른 타입 변경
		//for (const FCDSkillSlot& slot : classData->defaultSkills)
		for (auto& skill : classData->defaultSkills)
		{
			//abilitySystem->GrantSkill(slot.skill, slot.inputTag);
			abilitySystem->GrantSkill(skill.Value, skill.Key);
		}

		abilitySystem->GrantAbility(dodgeAbility, CDTags::Input_Dodge);
		abilitySystem->GrantAbility(classData->attackAbility, CDTags::Input_Attack);

		// 10.02 - Jun6 스킬 Tag로 등록된 GA에 Input Tag 추가
		// 10.08 - 레벨 이동 전 슬롯 배치 유지 (저장된 배치 우선)
		ApplySkillSlots();

		// 재빙의 시 중복 등록 방지
		abilitySystem->onSkillLoadoutChanged.RemoveAll(this);
		abilitySystem->onSkillLoadoutChanged.AddUObject(this, &ThisClass::HandleSkillLoadoutChanged);
	}
}

void ACDPlayerCharacter::ApplySkillSlots()
{
	const UGameInstance* gameInstance = GetGameInstance();
	const UCDSkillLoadoutSubsystem* loadout = gameInstance ? gameInstance->GetSubsystem<UCDSkillLoadoutSubsystem>() : nullptr;
	if (loadout && loadout->HasLoadout())
	{
		abilitySystem->ApplySkillLoadout(loadout->GetLoadout());
		return;
	}

	for (const FCDSkillSlot& slot : classData->defaultSkillSlots)
	{
		abilitySystem->AssignInputTag(slot.skillTag, slot.inputTag);
	}
}

void ACDPlayerCharacter::HandleSkillLoadoutChanged()
{
	const UGameInstance* gameInstance = GetGameInstance();
	if (UCDSkillLoadoutSubsystem* loadout = gameInstance ? gameInstance->GetSubsystem<UCDSkillLoadoutSubsystem>() : nullptr)
	{
		loadout->SaveLoadout(abilitySystem->GetSkillLoadout());
	}
}

void ACDPlayerCharacter::ApplySelectedClass()
{
	const UGameInstance* gameInstance = GetGameInstance();
	UCDPlayerClassSubsystem* classSubsystem = gameInstance ? gameInstance->GetSubsystem<UCDPlayerClassSubsystem>() : nullptr;
	if (classSubsystem != nullptr)
	{
		// 처음이면 BP 기본 직업을 등록, 이미 고른 직업이 있으면 그 직업으로 교체
		classSubsystem->RegisterDefaultClass(classData);
		if (classSubsystem->GetSelectedClass() != nullptr)
		{
			classData = classSubsystem->GetSelectedClass();
		}
	}

	if (classData == nullptr)
	{
		return;
	}

	// 메시를 먼저 바꾼 뒤 AnimClass 를 지정 (스켈레톤이 맞아야 애님 인스턴스가 정상 생성됨)
	USkeletalMeshComponent* meshComponent = GetMesh();
	if (classData->skeletalMesh != nullptr)
	{
		meshComponent->SetSkeletalMeshAsset(classData->skeletalMesh);
	}

	if (classData->animClass != nullptr)
	{
		meshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		meshComponent->SetAnimInstanceClass(classData->animClass);
	}
}

FVector ACDPlayerCharacter::GetAimLocation() const
{
	//FHitResult hit;
	//const APlayerController* playerController = GetController<APlayerController>();
	//if (playerController && playerController->GetHitResultUnderCursor(ECC_Visibility, false, hit))

	ACDPlayerController* playerController = GetController<ACDPlayerController>();
	if (playerController)
	{
		FVector aimLocation = FVector::ZeroVector;
		playerController->GetNearestNavLocation(aimLocation);

		return aimLocation;
	}
	return Super::GetAimLocation();
}

bool ACDPlayerCharacter::CanBeTargeted() const
{
	return !IsDead() && !abilitySystem->HasMatchingGameplayTag(CDTags::State_Untargetable);
}
