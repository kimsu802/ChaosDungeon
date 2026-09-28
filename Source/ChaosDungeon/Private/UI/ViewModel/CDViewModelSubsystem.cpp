#include "UI/ViewModel/CDViewModelSubsystem.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "MVVMGameSubsystem.h"
#include "Types/MVVMViewModelCollection.h"
#include "UI/ViewModel/CDHealthVM.h"
#include "UI/ViewModel/CDRunResultVM.h"
#include "UI/ViewModel/CDSkillSlotVM.h"
#include "UI/ViewModel/CDStageVM.h"

template <typename TViewModel>
TViewModel* UCDViewModelSubsystem::CreateGlobalViewModel(FName name)
{
	TViewModel* viewModel = NewObject<TViewModel>(this);

	FMVVMViewModelContext context;
	context.ContextClass = TViewModel::StaticClass();
	context.ContextName = name;

	UMVVMGameSubsystem* mvvm = GetLocalPlayer()->GetGameInstance()->GetSubsystem<UMVVMGameSubsystem>();
	mvvm->GetViewModelCollection()->AddViewModelInstance(context, viewModel);
	return viewModel;
}

void UCDViewModelSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	playerHealth = CreateGlobalViewModel<UCDHealthVM>(TEXT("PlayerHealth"));
	bossHealth = CreateGlobalViewModel<UCDHealthVM>(TEXT("BossHealth"));
	stage = CreateGlobalViewModel<UCDStageVM>(TEXT("Stage"));
	runResult = CreateGlobalViewModel<UCDRunResultVM>(TEXT("RunResult"));

	const TArray<FGameplayTag> slotTags = { CDTags::Input_Skill_1, CDTags::Input_Skill_2, CDTags::Input_Skill_3, CDTags::Input_Skill_4, CDTags::Input_Dodge };
	for (const FGameplayTag& slotTag : slotTags)
	{
		skillSlots.Add(slotTag, NewObject<UCDSkillSlotVM>(this));
	}

	UCDMessageSubsystem& messages = GetMessages();
	stage->StartListening(messages);
	runResult->StartListening(messages);
	bossHandle = messages.Listen(CDTags::Msg_Combat_BossAppeared, this, &ThisClass::HandleBossAppeared);
}

void UCDViewModelSubsystem::Deinitialize()
{
	UCDMessageSubsystem& messages = GetMessages();
	stage->StopListening(messages);
	runResult->StopListening(messages);
	messages.Unlisten(bossHandle);
	Super::Deinitialize();
}

void UCDViewModelSubsystem::BindPlayer(APlayerController* playerController)
{
	playerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePawnChanged);
	// 이미 빙의된 경우
	HandlePawnChanged(nullptr, playerController->GetPawn());
}

void UCDViewModelSubsystem::HandlePawnChanged(APawn* oldPawn, APawn* newPawn)
{
	UAbilitySystemComponent* abilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(newPawn);
	playerHealth->BindTo(abilitySystem);

	UCDAbilitySystemComponent* cdAbilitySystem = Cast<UCDAbilitySystemComponent>(abilitySystem);
	for (const TPair<FGameplayTag, TObjectPtr<UCDSkillSlotVM>>& slot : skillSlots)
	{
		slot.Value->BindTo(cdAbilitySystem, slot.Key);
	}
}

void UCDViewModelSubsystem::HandleBossAppeared(const FCDActorMessage& message)
{
	bossHealth->BindTo(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(message.actor));
}

UCDMessageSubsystem& UCDViewModelSubsystem::GetMessages() const
{
	return *GetLocalPlayer()->GetGameInstance()->GetSubsystem<UCDMessageSubsystem>();
}
