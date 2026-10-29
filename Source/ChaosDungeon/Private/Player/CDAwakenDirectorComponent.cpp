#include "Player/CDAwakenDirectorComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Components/PostProcessComponent.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"
#include "Core/CDMessageSubsystem.h"
#include "Curves/RealCurve.h"
#include "Engine/CurveTable.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"

namespace CDAwaken
{
	/** CurveTable 행 이름의 뒷부분 (ECDAwakenParam 순서) */
	static const TCHAR* ParamNames[] =
	{
		TEXT("FovScale"), TEXT("ArmScale"), TEXT("Saturation"), TEXT("Contrast"), TEXT("Cool"), TEXT("Vignette"), TEXT("Fringe"),
		TEXT("Exposure"), TEXT("Bloom"), TEXT("MotionBlur"), TEXT("DirBlur"), TEXT("RadialBlur"), TEXT("Invert"), TEXT("Shake")
	};
	static_assert(UE_ARRAY_COUNT(ParamNames) == static_cast<int32>(ECDAwakenParam::Count), "ParamNames 와 ECDAwakenParam 개수가 다름");

	constexpr int32 ParamCount = static_cast<int32>(ECDAwakenParam::Count);

	FORCEINLINE int32 Index(ECDAwakenParam param)
	{
		return static_cast<int32>(param);
	}
}

UCDAwakenDirectorComponent::UCDAwakenDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCDAwakenDirectorComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* owner = GetOwner();
	camera = owner->FindComponentByClass<UCameraComponent>();
	cameraBoom = owner->FindComponentByClass<USpringArmComponent>();

	if (const UCameraComponent* ownerCamera = camera.Get())
	{
		defaultFieldOfView = ownerCamera->FieldOfView;
	}
	if (const USpringArmComponent* boom = cameraBoom.Get())
	{
		defaultArmLength = boom->TargetArmLength;
		defaultSocketOffset = boom->SocketOffset;
		bDefaultCameraLag = boom->bEnableCameraLag;
		defaultCameraLagSpeed = boom->CameraLagSpeed;
	}

	// 연출 전용 PP: 레벨 PPV 위에 가중치로 덮는다 (가중치 0 = 영향 없음)
	postProcess = NewObject<UPostProcessComponent>(owner, TEXT("AwakenPostProcess"));
	postProcess->bUnbound = true;
	postProcess->Priority = 10.f;
	postProcess->BlendWeight = 0.f;
	postProcess->SetupAttachment(owner->GetRootComponent());
	postProcess->RegisterComponent();

	FPostProcessSettings& settings = postProcess->Settings;
	settings.bOverride_ColorSaturation = true;
	settings.bOverride_ColorContrast = true;
	settings.bOverride_ColorGain = true;
	settings.bOverride_VignetteIntensity = true;
	settings.bOverride_SceneFringeIntensity = true;
	settings.bOverride_AutoExposureBias = true;
	settings.bOverride_BloomIntensity = true;
	settings.bOverride_MotionBlurAmount = true;
	if (postProcessMaterial)
	{
		settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.f, postProcessMaterial));
	}

	for (int32 i = 0; i < CDAwaken::ParamCount; ++i)
	{
		values[i] = GetNeutralValue(static_cast<ECDAwakenParam>(i));
	}
	CacheCurves();
}

void UCDAwakenDirectorComponent::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	// 연출 도중 사라지면(사망 · 레벨 이동) 세상 시간만은 반드시 되돌린다
	if (phase != ECDAwakenPhase::None)
	{
		UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
		phase = ECDAwakenPhase::None;
	}
	Super::EndPlay(endPlayReason);
}

void UCDAwakenDirectorComponent::CacheCurves()
{
	if (curveTable == nullptr)
	{
		return;
	}

	const UEnum* phaseEnum = StaticEnum<ECDAwakenPhase>();
	for (int32 phaseIndex = static_cast<int32>(ECDAwakenPhase::Charge); phaseIndex <= static_cast<int32>(ECDAwakenPhase::Recover); ++phaseIndex)
	{
		const FString phaseName = phaseEnum->GetNameStringByValue(phaseIndex);
		for (int32 paramIndex = 0; paramIndex < CDAwaken::ParamCount; ++paramIndex)
		{
			const FName rowName(*FString::Printf(TEXT("%s.%s"), *phaseName, CDAwaken::ParamNames[paramIndex]));
			curves[phaseIndex][paramIndex] = curveTable->FindCurve(rowName, TEXT("CDAwakenDirector"), false);
		}
	}
}

float UCDAwakenDirectorComponent::GetNeutralValue(ECDAwakenParam param)
{
	switch (param)
	{
	case ECDAwakenParam::FovScale:
	case ECDAwakenParam::ArmScale:
	case ECDAwakenParam::Saturation:
	case ECDAwakenParam::Contrast:
		return 1.f;
	case ECDAwakenParam::Vignette:
		return 0.4f;
	case ECDAwakenParam::Bloom:
		return 0.675f;
	case ECDAwakenParam::MotionBlur:
		return 0.5f;
	default:
		return 0.f;
	}
}

void UCDAwakenDirectorComponent::EnterPhase(ECDAwakenPhase newPhase)
{
	if (newPhase == ECDAwakenPhase::None)
	{
		FinishAll();
		return;
	}
	if (newPhase == ECDAwakenPhase::Recover)
	{
		StartRecover();
		return;
	}

	phase = newPhase;
	phaseTime = 0.f;
	bFreezeReleased = false;

	switch (phase)
	{
	case ECDAwakenPhase::Charge:
		bAbilityEnded = false;
		ApplyTimeDilation(chargeTimeDilation, true);
		break;

	case ECDAwakenPhase::Release:
		ApplyTimeDilation(1.f, false);
		// 돌진하는 캐릭터를 카메라가 뒤따라오게 → 순간 이동으로 화면 전체가 뭉개지는 것 방지
		if (USpringArmComponent* boom = cameraBoom.Get())
		{
			if (releaseCameraLagSpeed > 0.f)
			{
				boom->bEnableCameraLag = true;
				boom->CameraLagSpeed = releaseCameraLagSpeed;
			}
		}
		break;

	case ECDAwakenPhase::Freeze:
		// 플레이어까지 전부 멈춤 → 몽타주도 멈춰 다음 Hit 노티파이가 freezeDuration 만큼 늦게 온다
		ApplyTimeDilation(freezeTimeDilation, false);
		break;

	case ECDAwakenPhase::Impact:
		ApplyTimeDilation(1.f, false);
		if (impactShake)
		{
			const APawn* pawn = Cast<APawn>(GetOwner());
			if (APlayerController* playerController = pawn ? pawn->GetController<APlayerController>() : nullptr)
			{
				playerController->ClientStartCameraShake(impactShake);
			}
		}
		break;

	default:
		break;
	}

	UpdateScreenSpace();
	EvaluateCurves();
	ApplyValues((phase == ECDAwakenPhase::Charge) ? 0.f : 1.f);
	BroadcastPhase();
	SetComponentTickEnabled(true);
}

void UCDAwakenDirectorComponent::NotifyAbilityEnded(bool bWasCancelled)
{
	if ((phase == ECDAwakenPhase::None) || (phase == ECDAwakenPhase::Recover))
	{
		return;
	}

	bAbilityEnded = true;

	// 취소(피격 · 회피 등) 또는 Impact 전에 몽타주가 끝남 → 바로 복귀
	// Freeze 는 정지 구간이 끝난 뒤 Tick 에서 복귀, Impact 는 끝까지 재생
	if (bWasCancelled || (phase == ECDAwakenPhase::Charge) || (phase == ECDAwakenPhase::Release))
	{
		StartRecover();
	}
}

void UCDAwakenDirectorComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

	// deltaTime 은 Global × Owner 배율이 곱해진 값 → 연출은 실시간 기준
	const float realDelta = GetWorld()->DeltaRealTimeSeconds;
	AdvancePhase(realDelta);

	if (phase == ECDAwakenPhase::None)
	{
		return;
	}

	if (phase == ECDAwakenPhase::Recover)
	{
		const float alpha = FMath::InterpEaseInOut(0.f, 1.f, FMath::Clamp(phaseTime / FMath::Max(recoverDuration, KINDA_SMALL_NUMBER), 0.f, 1.f), 2.f);
		for (int32 i = 0; i < CDAwaken::ParamCount; ++i)
		{
			values[i] = FMath::Lerp(recoverFrom[i], GetNeutralValue(static_cast<ECDAwakenParam>(i)), alpha);
		}
		UpdateScreenSpace();
		ApplyValues(1.f - alpha);
		return;
	}

	EvaluateCurves();
	UpdateScreenSpace();

	float weight = 1.f;
	if (phase == ECDAwakenPhase::Charge)
	{
		weight = FMath::Clamp(phaseTime / FMath::Max(postProcessBlendInTime, KINDA_SMALL_NUMBER), 0.f, 1.f);
	}
	ApplyValues(weight);
}

void UCDAwakenDirectorComponent::AdvancePhase(float realDelta)
{
	phaseTime += realDelta;

	switch (phase)
	{
	case ECDAwakenPhase::Freeze:
		if ((bFreezeReleased == false) && (phaseTime >= freezeDuration))
		{
			// 정지 해제 → 몽타주 재개 → Hit 노티파이 → Impact
			bFreezeReleased = true;
			ApplyTimeDilation(1.f, false);
		}
		// Hit 가 오지 않음 (어빌리티 종료 · 노티파이 누락) → 복귀
		if (bFreezeReleased && (bAbilityEnded || (phaseTime >= freezeDuration + impactTimeout)))
		{
			StartRecover();
		}
		break;

	case ECDAwakenPhase::Impact:
		if (phaseTime >= impactDuration)
		{
			StartRecover();
		}
		break;

	case ECDAwakenPhase::Recover:
		if (phaseTime >= recoverDuration)
		{
			FinishAll();
		}
		break;

	default:
		break;
	}
}

void UCDAwakenDirectorComponent::EvaluateCurves()
{
	const int32 phaseIndex = static_cast<int32>(phase);
	for (int32 i = 0; i < CDAwaken::ParamCount; ++i)
	{
		const FRealCurve* curve = curves[phaseIndex][i];
		values[i] = curve ? curve->Eval(phaseTime) : GetNeutralValue(static_cast<ECDAwakenParam>(i));
	}
}

void UCDAwakenDirectorComponent::ApplyValues(float postProcessWeight)
{
	using namespace CDAwaken;

	if (UCameraComponent* ownerCamera = camera.Get())
	{
		ownerCamera->SetFieldOfView(defaultFieldOfView * values[Index(ECDAwakenParam::FovScale)]);
	}

	if (USpringArmComponent* boom = cameraBoom.Get())
	{
		boom->TargetArmLength = defaultArmLength * values[Index(ECDAwakenParam::ArmScale)];

		// 카메라 셰이크 에셋은 Global 시간으로 돌아서 기 모으기 중엔 느려진다 → 실시간으로 직접 흔든다
		const float shake = values[Index(ECDAwakenParam::Shake)];
		FVector offset = defaultSocketOffset;
		if (shake > 0.f)
		{
			offset += FVector(0.f, FMath::FRandRange(-shake, shake), FMath::FRandRange(-shake, shake));
		}
		boom->SocketOffset = offset;
	}

	if (postProcess)
	{
		FPostProcessSettings& settings = postProcess->Settings;
		const float saturation = values[Index(ECDAwakenParam::Saturation)];
		const float contrast = values[Index(ECDAwakenParam::Contrast)];
		const float cool = values[Index(ECDAwakenParam::Cool)];
		settings.ColorSaturation = FVector4(saturation, saturation, saturation, 1.f);
		settings.ColorContrast = FVector4(contrast, contrast, contrast, 1.f);
		settings.ColorGain = FMath::Lerp(FVector4(1.f, 1.f, 1.f, 1.f), FVector4(coolGain.R, coolGain.G, coolGain.B, 1.f), cool);
		settings.VignetteIntensity = values[Index(ECDAwakenParam::Vignette)];
		settings.SceneFringeIntensity = values[Index(ECDAwakenParam::Fringe)];
		settings.AutoExposureBias = values[Index(ECDAwakenParam::Exposure)];
		settings.BloomIntensity = values[Index(ECDAwakenParam::Bloom)];
		settings.MotionBlurAmount = values[Index(ECDAwakenParam::MotionBlur)];
		postProcess->BlendWeight = postProcessWeight;
	}

	if (parameterCollection)
	{
		// MPC_Awaken 파라미터 이름
		static const FName DirBlur(TEXT("DirBlur"));
		static const FName DirX(TEXT("DirX"));
		static const FName DirY(TEXT("DirY"));
		static const FName RadialBlur(TEXT("RadialBlur"));
		static const FName CenterU(TEXT("CenterU"));
		static const FName CenterV(TEXT("CenterV"));
		static const FName Invert(TEXT("Invert"));

		const float radians = FMath::DegreesToRadians(screenAngle);
		UKismetMaterialLibrary::SetScalarParameterValue(this, parameterCollection, DirBlur, values[Index(ECDAwakenParam::DirBlur)] * postProcessWeight);
		UKismetMaterialLibrary::SetScalarParameterValue(this, parameterCollection, DirX, FMath::Cos(radians));
		UKismetMaterialLibrary::SetScalarParameterValue(this, parameterCollection, DirY, FMath::Sin(radians));
		UKismetMaterialLibrary::SetScalarParameterValue(this, parameterCollection, RadialBlur, values[Index(ECDAwakenParam::RadialBlur)] * postProcessWeight);
		UKismetMaterialLibrary::SetScalarParameterValue(this, parameterCollection, CenterU, screenCenter.X);
		UKismetMaterialLibrary::SetScalarParameterValue(this, parameterCollection, CenterV, screenCenter.Y);
		UKismetMaterialLibrary::SetScalarParameterValue(this, parameterCollection, Invert, values[Index(ECDAwakenParam::Invert)] * postProcessWeight);
	}
}

void UCDAwakenDirectorComponent::ApplyTimeDilation(float globalDilation, bool bCompensateOwner)
{
	UGameplayStatics::SetGlobalTimeDilation(this, globalDilation);

	// 컴포넌트(메시 애니메이션 · 이동 · SpringArm)는 Owner 배율을 따른다
	GetOwner()->CustomTimeDilation = bCompensateOwner ? (1.f / FMath::Max(globalDilation, KINDA_SMALL_NUMBER)) : 1.f;
}

void UCDAwakenDirectorComponent::StartRecover()
{
	if ((phase == ECDAwakenPhase::None) || (phase == ECDAwakenPhase::Recover))
	{
		return;
	}

	FMemory::Memcpy(recoverFrom, values, sizeof(values));
	phase = ECDAwakenPhase::Recover;
	phaseTime = 0.f;
	ApplyTimeDilation(1.f, false);
	BroadcastPhase();
	SetComponentTickEnabled(true);
}

void UCDAwakenDirectorComponent::FinishAll()
{
	for (int32 i = 0; i < CDAwaken::ParamCount; ++i)
	{
		values[i] = GetNeutralValue(static_cast<ECDAwakenParam>(i));
	}
	ApplyValues(0.f);

	if (USpringArmComponent* boom = cameraBoom.Get())
	{
		boom->bEnableCameraLag = bDefaultCameraLag;
		boom->CameraLagSpeed = defaultCameraLagSpeed;
	}

	ApplyTimeDilation(1.f, false);

	const bool bWasActive = (phase != ECDAwakenPhase::None);
	phase = ECDAwakenPhase::None;
	phaseTime = 0.f;
	if (bWasActive)
	{
		BroadcastPhase();
	}
	SetComponentTickEnabled(false);
}

void UCDAwakenDirectorComponent::UpdateScreenSpace()
{
	const APawn* pawn = Cast<APawn>(GetOwner());
	const APlayerController* playerController = pawn ? pawn->GetController<APlayerController>() : nullptr;
	if (playerController == nullptr)
	{
		return;
	}

	const FVector location = pawn->GetActorLocation();
	FVector2D screenFrom;
	FVector2D screenTo;
	if ((playerController->ProjectWorldLocationToScreen(location, screenFrom) == false)
		|| (playerController->ProjectWorldLocationToScreen(location + (pawn->GetActorForwardVector() * 200.f), screenTo) == false))
	{
		return;
	}

	const FVector2D direction = (screenTo - screenFrom).GetSafeNormal();
	if (direction.IsNearlyZero() == false)
	{
		// 화면 Y 는 아래가 + → UMG RenderTransformAngle(시계 방향 +)과 같은 기준
		screenAngle = FMath::RadiansToDegrees(FMath::Atan2(direction.Y, direction.X));
	}

	int32 viewportWidth = 0;
	int32 viewportHeight = 0;
	playerController->GetViewportSize(viewportWidth, viewportHeight);
	if ((viewportWidth > 0) && (viewportHeight > 0))
	{
		screenCenter = FVector2D(screenFrom.X / viewportWidth, screenFrom.Y / viewportHeight);
	}
}

void UCDAwakenDirectorComponent::BroadcastPhase() const
{
	FCDAwakenPhaseMessage message;
	message.phase = phase;
	message.screenAngle = screenAngle;
	message.instigator = GetOwner();
	UCDMessageSubsystem::Get(this).Broadcast(CDTags::Msg_Awaken_Phase, message);
}
