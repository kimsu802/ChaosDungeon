#include "Player/CDPlayerController.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"
#include "Data/CDInputConfig.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Player/CDCursorHighlightComponent.h"
#include "UI/CDUIManagerSubsystem.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "World/CDInteractable.h"

ACDPlayerController::ACDPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	cursorHighlight = CreateDefaultSubobject<UCDCursorHighlightComponent>(TEXT("CursorHighlight"));
}

void ACDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* inputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		inputSubsystem->AddMappingContext(inputConfig->mappingContext, 0);
		// 키 매핑 (Project Settings > Enhanced Input > Enable User Settings 필요)
		if (UEnhancedInputUserSettings* userSettings = inputSubsystem->GetUserSettings())
		{
			userSettings->RegisterInputMappingContext(inputConfig->mappingContext);
		}
	}

	// 레이아웃 + HUD 화면 생성 (입력 모드는 활성 화면의 GetDesiredInputConfig 가 결정)
	if (UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(this))
	{
		uiManager->InitializeForPlayer(this);
	}
}

void ACDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* input = CastChecked<UEnhancedInputComponent>(InputComponent);
	input->BindAction(inputConfig->moveAction, ETriggerEvent::Started, this, &ThisClass::OnMoveStarted);
	input->BindAction(inputConfig->moveAction, ETriggerEvent::Triggered, this, &ThisClass::OnMoveTriggered);
	input->BindAction(inputConfig->moveAction, ETriggerEvent::Canceled, this, &ThisClass::OnMoveReleased);
	input->BindAction(inputConfig->pauseAction, ETriggerEvent::Started, this, &ThisClass::OnPause);
	input->BindAction(inputConfig->guideAction, ETriggerEvent::Started, this, &ThisClass::OnGuide);

	for (const FCDAbilityInput& abilityInput : inputConfig->abilityInputs)
	{
		input->BindAction(abilityInput.action, ETriggerEvent::Started, this, &ThisClass::OnAbilityInput, abilityInput.inputTag);
	}
}

void ACDPlayerController::PlayerTick(float deltaTime)
{
	Super::PlayerTick(deltaTime);
	UpdatePendingInteraction();
}

void ACDPlayerController::OnMoveStarted()
{
	movePressedTime = 0.f;
	if (UpdateMoveDestination() && CanMove())
	{
		// 누르는 즉시 반응: 짧은 클릭은 이 길찾기 이동으로 끝난다
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, moveDestination);
	}
}

void ACDPlayerController::OnMoveTriggered()
{
	const float previousTime = movePressedTime;
	movePressedTime += GetWorld()->GetDeltaSeconds();

	// 짧은 클릭 구간에서는 길찾기 이동을 방해하지 않는다
	if (movePressedTime < shortPressThreshold)
	{
		return;
	}

	// 길게 누르기로 막 전환된 순간: 길찾기 중단 후 커서 추적 이동으로
	if (previousTime < shortPressThreshold)
	{
		StopMovement();
	}

	APawn* controlledPawn = GetPawn();
	if (UpdateMoveDestination() && controlledPawn && CanMove())
	{
		controlledPawn->AddMovementInput((moveDestination - controlledPawn->GetActorLocation()).GetSafeNormal2D());
	}
}

void ACDPlayerController::OnMoveReleased()
{
	// 길게 누르다 뗐으면 마지막 커서 위치까지는 이어서 이동
	if (movePressedTime >= shortPressThreshold && CanMove())
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, moveDestination);
	}
	movePressedTime = 0.f;
}

bool ACDPlayerController::UpdateMoveDestination()
{
	FHitResult hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, true, hit))
	{
		return false;
	}
	moveDestination = hit.Location;
	pendingInteraction = Cast<ACDInteractable>(hit.GetActor());
	return true;
}

void ACDPlayerController::OnAbilityInput(FGameplayTag inputTag)
{
	if (const ACDCharacterBase* controlledCharacter = GetPawn<ACDCharacterBase>())
	{
		controlledCharacter->GetCDAbilitySystemComponent()->AbilityInputPressed(inputTag);
	}
}

void ACDPlayerController::OnPause()
{
	if (UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(this))
	{
		uiManager->TogglePauseMenu();
	}
}

void ACDPlayerController::OnGuide()
{
	if (UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(this))
	{
		uiManager->ToggleGuide();
	}
}

bool ACDPlayerController::CanMove() const
{
	static const TArray<FGameplayTag> blockTagList = { CDTags::State_Casting, CDTags::State_HitReact, CDTags::State_Dead };
	static const FGameplayTagContainer blockTags = FGameplayTagContainer::CreateFromArray(blockTagList);

	const ACDCharacterBase* controlledCharacter = GetPawn<ACDCharacterBase>();
	return controlledCharacter && !controlledCharacter->GetCDAbilitySystemComponent()->HasAnyMatchingGameplayTags(blockTags);
}

void ACDPlayerController::UpdatePendingInteraction()
{
	ACDInteractable* target = pendingInteraction.Get();
	const APawn* controlledPawn = GetPawn();
	if (!target || !controlledPawn)
	{
		return;
	}
	if (FVector::Dist2D(target->GetActorLocation(), controlledPawn->GetActorLocation()) <= target->GetInteractRange())
	{
		StopMovement();
		pendingInteraction = nullptr;
		target->Interact(this);
	}
}
