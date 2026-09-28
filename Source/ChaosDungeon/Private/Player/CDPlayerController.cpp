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
	input->BindAction(inputConfig->moveAction, ETriggerEvent::Completed, this, &ThisClass::OnMoveReleased);
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
	StopMovement();
	movePressedTime = 0.f;
	pendingInteraction = nullptr;
}

void ACDPlayerController::OnMoveTriggered()
{
	movePressedTime += GetWorld()->GetDeltaSeconds();

	FHitResult hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, true, hit))
	{
		return;
	}
	moveDestination = hit.Location;
	pendingInteraction = Cast<ACDInteractable>(hit.GetActor());

	APawn* controlledPawn = GetPawn();
	if (controlledPawn && CanMove())
	{
		controlledPawn->AddMovementInput((moveDestination - controlledPawn->GetActorLocation()).GetSafeNormal2D());
	}
}

void ACDPlayerController::OnMoveReleased()
{
	if (movePressedTime <= shortPressThreshold && CanMove())
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, moveDestination);
	}
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
