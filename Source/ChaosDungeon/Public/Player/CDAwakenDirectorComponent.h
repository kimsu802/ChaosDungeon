#pragma once

#include "Components/ActorComponent.h"
#include "Core/CDTypes.h"
#include "CDAwakenDirectorComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UPostProcessComponent;
class UCurveTable;
class UMaterialInterface;
class UMaterialParameterCollection;
class UCameraShakeBase;
struct FRealCurve;

/** 각성 연출에서 커브로 조절하는 값 (CurveTable 행 이름의 뒷부분) */
enum class ECDAwakenParam : uint8
{
	FovScale,		// 기본 FOV 배율
	ArmScale,		// 기본 SpringArm 길이 배율
	Saturation,		// ColorSaturation
	Contrast,		// ColorContrast
	Cool,			// 차가운 틴트 비율 (0 = 없음, 1 = coolGain)
	Vignette,		// VignetteIntensity
	Fringe,			// SceneFringeIntensity (색수차)
	Exposure,		// AutoExposureBias
	Bloom,			// BloomIntensity
	MotionBlur,		// MotionBlurAmount
	DirBlur,		// MPC: 방향 블러 세기
	RadialBlur,		// MPC: 방사 블러 세기
	Invert,			// MPC: 임팩트 프레임 (0 / 1)
	Shake,			// SocketOffset 미세 흔들림 (uu)
	Count
};

/**
 * 각성기 연출 감독 (플레이어에 부착). 카메라 · 포스트프로세스 · TimeDilation 을 한 곳에서 바꾸고 한 곳에서 되돌린다.
 *
 * 단계: Charge(기 모으기) → Release(돌진) → Freeze(정적) → Impact(지연 베기) → Recover(복귀) → None
 *  - 단계 진입은 UCDPlayerSkillAbility 가 몽타주 이벤트에 맞춰 EnterPhase 로 알려준다
 *  - Impact / Recover 는 시간이 지나면 스스로 넘어간다 (어빌리티가 먼저 끝나도 연출은 끝까지 재생)
 *  - 어빌리티가 취소되면 즉시 시간을 되돌리고 Recover 로
 *
 * 값은 CurveTable(CT_AwakenDirection) 의 "단계.파라미터" 행을 단계 시작 후 경과 시간(실시간)으로 평가한다.
 *  - 행이 없으면 기본값(효과 없음)
 *  - TimeDilation 영향을 받지 않도록 실시간(UWorld::DeltaRealTimeSeconds)으로 진행
 *
 * 단계가 바뀔 때마다 Msg.Awaken.Phase 를 발행 → UI(UCDAwakenOverlayWidget)가 레터박스 · 플래시 · 집중선을 맡는다.
 */
UCLASS(ClassGroup = (CD), meta = (BlueprintSpawnableComponent))
class CHAOSDUNGEON_API UCDAwakenDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 틱 설정 (연출 중에만 틱) */
	UCDAwakenDirectorComponent();

	// UActorComponent::TickComponent()
	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

	/** 단계 진입 */
	void EnterPhase(ECDAwakenPhase newPhase);

	/** 어빌리티 종료 알림. 취소면 즉시 복귀, 정상 종료면 남은 연출을 마저 재생 */
	void NotifyAbilityEnded(bool bWasCancelled);

	/** 현재 단계 */
	FORCEINLINE ECDAwakenPhase GetPhase() const
	{
		return phase;
	}

protected:
	// UActorComponent::BeginPlay()
	virtual void BeginPlay() override;

	// UActorComponent::EndPlay()
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

private:
	/** 단계 시간을 진행시키고 자동 전환 처리 */
	void AdvancePhase(float realDelta);

	/** 현재 단계 커브를 평가해 values 갱신 */
	void EvaluateCurves();

	/** values 를 카메라 · PP · MPC 에 반영 */
	void ApplyValues(float postProcessWeight);

	/** Global TimeDilation 설정. bCompensateOwner 면 플레이어는 정상 속도 유지 */
	void ApplyTimeDilation(float globalDilation, bool bCompensateOwner);

	/** 복귀 시작: 현재 값을 기억하고 recoverDuration 동안 기본값으로 */
	void StartRecover();

	/** 연출 완전 종료: 모든 값을 기본값으로 */
	void FinishAll();

	/** 플레이어 정면 방향의 화면 각도 · 화면 위치 갱신 */
	void UpdateScreenSpace();

	/** Msg.Awaken.Phase 발행 */
	void BroadcastPhase() const;

	/** 파라미터 기본값 (효과 없음) */
	static float GetNeutralValue(ECDAwakenParam param);

	/** 커브 테이블에서 단계 · 파라미터 커브 캐시 */
	void CacheCurves();

protected:
	/** 단계별 커브 (행 이름: Charge.Saturation, Release.FovScale ...) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Data")
	TObjectPtr<UCurveTable> curveTable;

	/** 블러 · 임팩트 프레임용 MPC (DirBlur, DirX, DirY, RadialBlur, CenterU, CenterV, Invert) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Data")
	TObjectPtr<UMaterialParameterCollection> parameterCollection;

	/** M_PP_Awaken (Blendable) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Data")
	TObjectPtr<UMaterialInterface> postProcessMaterial;

	/** Impact 진입 시 재생할 카메라 셰이크 (시간이 1.0 으로 돌아온 뒤라 정상 속도) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Camera")
	TSubclassOf<UCameraShakeBase> impactShake;

	/** Release 이후 카메라가 뒤따라오는 속도 (0 = Lag 사용 안 함) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Camera")
	float releaseCameraLagSpeed = 10.f;

	/** Cool = 1 일 때 ColorGain */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Camera")
	FLinearColor coolGain = FLinearColor(0.9f, 0.97f, 1.1f, 1.f);

	/** 기 모으기 동안 세상 시간 배율 (플레이어는 역수로 보정) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Time", meta = (ClampMin = 0.01, ClampMax = 1))
	float chargeTimeDilation = 0.2f;

	/** 정적 동안 세상 시간 배율 (플레이어 포함 전부 멈춤 → 몽타주도 멈춰 Hit 노티파이가 늦게 온다) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Time", meta = (ClampMin = 0.0001, ClampMax = 1))
	float freezeTimeDilation = 0.02f;

	/** 정적 유지 시간 (실시간 초) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Time")
	float freezeDuration = 0.3f;

	/** 정적이 풀린 뒤 Impact(Hit 노티파이)를 기다리는 최대 시간. 넘으면 복귀 */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Time")
	float impactTimeout = 1.f;

	/** Impact 연출 길이 (끝나면 Recover) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Time")
	float impactDuration = 0.6f;

	/** 복귀 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Time")
	float recoverDuration = 0.4f;

	/** Charge 진입 시 PP 가중치가 0 → 1 로 오르는 시간 (레벨 PPV 값과 달라도 튀지 않게) */
	UPROPERTY(EditDefaultsOnly, Category = "Awaken|Time")
	float postProcessBlendInTime = 0.15f;

private:
	/** 연출 전용 PP (Unbound, BeginPlay 에서 생성) */
	UPROPERTY(Transient)
	TObjectPtr<UPostProcessComponent> postProcess;

	/** 플레이어 카메라 */
	TWeakObjectPtr<UCameraComponent> camera;

	/** 플레이어 카메라 암 */
	TWeakObjectPtr<USpringArmComponent> cameraBoom;

	/** 단계 × 파라미터 커브 (없으면 null) */
	const FRealCurve* curves[static_cast<int32>(ECDAwakenPhase::Recover) + 1][static_cast<int32>(ECDAwakenParam::Count)] = {};

	/** 현재 값 */
	float values[static_cast<int32>(ECDAwakenParam::Count)] = {};

	/** Recover 시작 시점의 값 */
	float recoverFrom[static_cast<int32>(ECDAwakenParam::Count)] = {};

	/** 현재 단계 */
	ECDAwakenPhase phase = ECDAwakenPhase::None;

	/** 단계 시작 후 경과 시간 (실시간) */
	float phaseTime = 0.f;

	/** Freeze 의 정지 구간이 끝났는가 */
	bool bFreezeReleased = false;

	/** 어빌리티가 이미 끝났는가 */
	bool bAbilityEnded = false;

	/** 기본 FOV */
	float defaultFieldOfView = 90.f;

	/** 기본 암 길이 */
	float defaultArmLength = 2000.f;

	/** 기본 SocketOffset */
	FVector defaultSocketOffset = FVector::ZeroVector;

	/** 기본 카메라 Lag 사용 여부 */
	bool bDefaultCameraLag = false;

	/** 기본 카메라 Lag 속도 */
	float defaultCameraLagSpeed = 10.f;

	/** 플레이어 정면 방향의 화면 각도 (도, 화면 오른쪽 = 0, 시계 방향 +) */
	float screenAngle = 0.f;

	/** 플레이어의 화면 위치 (UV, 방사 블러 중심) */
	FVector2D screenCenter = FVector2D(0.5f, 0.5f);
};
