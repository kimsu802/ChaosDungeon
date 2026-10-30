#include "Core/CDHubGameMode.h"
#include "Core/CDRunSubsystem.h"
#include "Core/CDPlayerClassSubsystem.h"
#include "Data/CDCharacterClassData.h"
#include "GameFramework/Pawn.h"
#include "Player/CDPlayerController.h"
#include "UI/CDUIManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Core/CDMessages.h"
#include "Core/CDGameplayTags.h"
#include "Data/CDStageData.h"

ACDHubGameMode::ACDHubGameMode()
{
	PlayerControllerClass = ACDPlayerController::StaticClass();
}

void ACDHubGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 던전(결과 화면 "돌아가기")에서 돌아온 경우: 타이틀을 건너뛰고 바로 허브 HUD
	// 단, 일시정지 메뉴의 "메인 메뉴"로 돌아온 경우는 타이틀을 띄운다
	UCDRunSubsystem* run = GetGameInstance()->GetSubsystem<UCDRunSubsystem>();
	const bool bReturnToTitle = run->ConsumeReturnToTitle();
	if (run->HasPlayedRun() && (bReturnToTitle == false))
	{
		EnterHub(true);
		return;
	}

	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	TArray<AActor*> cameras;
	UGameplayStatics::GetAllActorsWithTag(this, titleCameraTag, cameras);
	if (playerController && cameras.Num() > 0)
	{
		playerController->SetViewTarget(cameras[0]);
	}

	// 타이틀 화면의 '시작' 버튼 → 화면 닫기 + EnterHub
	if (UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(playerController))
	{	
		UWorld* world = playerController->GetWorld();
		if (!world)
		{
			return;
		}

		uiManager->ShowTitle();
	}
}

void ACDHubGameMode::EnterHub(bool bSkipBlend)
{
	const float blendTime = bSkipBlend ? 0.f : cameraBlendTime;
	if (APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		playerController->SetViewTargetWithBlend(playerController->GetPawn(), blendTime, VTBlend_Cubic);

		if (UCDUIManagerSubsystem* uiManager = UCDUIManagerSubsystem::Get(playerController))
		{
			UWorld* world = playerController->GetWorld();
			if (!world)
			{
				return;
			}

			uiManager->ShowGameHud();
		}
	}


}

void ACDHubGameMode::RespawnPlayerAsClass(APlayerController* playerController, UCDCharacterClassData* newClass)
{
	if ((playerController == nullptr) || (newClass == nullptr))
	{
		return;
	}

	UCDPlayerClassSubsystem* classSubsystem = GetGameInstance()->GetSubsystem<UCDPlayerClassSubsystem>();
	if (classSubsystem->GetSelectedClass() == newClass)
	{
		return;
	}

	// 새 캐릭터는 빙의될 때(PossessedBy) 여기서 직업을 읽는다
	classSubsystem->SelectClass(newClass);

	APawn* oldPawn = playerController->GetPawn();
	AActor* oldViewTarget = playerController->GetViewTarget();

	if (oldPawn == nullptr)
	{
		RestartPlayer(playerController);
	}
	else
	{
		const FTransform spawnTransform = oldPawn->GetActorTransform();
		playerController->UnPossess();
		oldPawn->Destroy();

		// DefaultPawnClass(BP_CDPlayer) 를 같은 자리에 스폰 → 빙의
		RestartPlayerAtTransform(playerController, spawnTransform);
	}

	// 빙의하면 뷰 타겟이 새 폰으로 바뀐다 → 타이틀 카메라를 보고 있었다면 블렌드 없이 되돌림
	if ((oldViewTarget != nullptr) && (oldViewTarget != oldPawn))
	{
		playerController->SetViewTarget(oldViewTarget);
	}
}
