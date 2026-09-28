#include "Core/CDHubGameMode.h"
#include "Core/CDRunSubsystem.h"
#include "Player/CDPlayerController.h"
#include "UI/CDUIManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

ACDHubGameMode::ACDHubGameMode()
{
	PlayerControllerClass = ACDPlayerController::StaticClass();
}

void ACDHubGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (GetGameInstance()->GetSubsystem<UCDRunSubsystem>()->HasPlayedRun())
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
		uiManager->ShowTitle();
	}
}

void ACDHubGameMode::EnterHub(bool bSkipBlend)
{
	const float blendTime = bSkipBlend ? 0.f : cameraBlendTime;
	if (APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		playerController->SetViewTargetWithBlend(playerController->GetPawn(), blendTime, VTBlend_Cubic);
	}
}
