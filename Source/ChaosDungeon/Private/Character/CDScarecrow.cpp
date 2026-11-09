#include "Character/CDScarecrow.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AbilitySystem/CDAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDTypes.h"
#include "Data/CDMonsterData.h"
#include "Player/CDSpeechBubbleComponent.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "CDScarecrow"

ACDScarecrow::ACDScarecrow()
{
	// 움직이거나 공격하지 않도록 AI 컨트롤러를 붙이지 않음
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;

	// 머리 위 대사창: 허수아비 메시에 head 소켓이 없을 수 있어 캡슐 기준 (높이는 BP 에서 조절)
	speechBubble = CreateDefaultSubobject<UCDSpeechBubbleComponent>(TEXT("SpeechBubble"));
	speechBubble->SetupAttachment(GetCapsuleComponent());
	speechBubble->SetRelativeLocation(FVector(0.f, 0.f, 140.f));

	idleLines.Add(LOCTEXT("Idle_HitMe", "날 때려보세요 !"));
	idleLines.Add(LOCTEXT("Idle_Bored", "흐흠~ 지루하다"));
	hitLines.Add(LOCTEXT("Hit_Hurt", "조금 아플지도? 후후"));
}

void ACDScarecrow::BeginPlay()
{
	Super::BeginPlay();

	// 스폰 매니저 대신 스스로 데이터 적용 (ActivateFromPool 은 바닥 위치를 받아 캡슐 절반만큼 올린다)
	if (scarecrowData != nullptr)
	{
		const FVector groundLocation = GetActorLocation() - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		ActivateFromPool(scarecrowData, FCDDifficultySettings(), groundLocation);
	}

	if (bImmuneToStagger)
	{
		abilitySystem->AddLooseGameplayTag(CDTags::State_Immune_Stagger);
	}

	if (bImmuneToKnockback)
	{
		abilitySystem->AddLooseGameplayTag(CDTags::State_Immune_Knockback);
	}

	ScheduleIdleLine();
}

void ACDScarecrow::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(endPlayReason);
}

void ACDScarecrow::HandleDamageTaken(AActor* damageInstigator, float damage, bool bCritical)
{
	// 데미지 숫자 · 기록 메시지는 그대로
	Super::HandleDamageTaken(damageInstigator, damage, bCritical);

	// 연타 중엔 체력 회복을 미룸 → 멈추면 healthResetDelay 뒤 가득
	GetWorldTimerManager().SetTimer(healthResetTimer, this, &ThisClass::RefillHealth, healthResetDelay, false);

	// 맞는 동안엔 혼잣말 대신 피격 대사 (쿨다운)
	ScheduleIdleLine();

	const float now = GetWorld()->GetTimeSeconds();
	if ((now - lastHitLineTime) < hitLineCooldown)
	{
		return;
	}

	if (const FText* line = PickLine(hitLines, lastHitIndex))
	{
		lastHitLineTime = now;
		SayLine(*line);
	}
}

void ACDScarecrow::HandleDeath()
{
	// 죽지 않는다: 사망 처리(풀 반환 · 드랍 · 사망 메시지) 대신 바로 체력 회복
	Revive(0.f);
}

void ACDScarecrow::RefillHealth()
{
	abilitySystem->SetNumericAttributeBase(UCDAttributeSet::GetHealthAttribute(), attributes->GetMaxHealth());
}

void ACDScarecrow::ScheduleIdleLine()
{
	const float delay = FMath::FRandRange(idleIntervalMin, FMath::Max(idleIntervalMin, idleIntervalMax));
	GetWorldTimerManager().SetTimer(idleLineTimer, this, &ThisClass::HandleIdleLine, delay, false);
}

void ACDScarecrow::HandleIdleLine()
{
	if (const FText* line = PickLine(idleLines, lastIdleIndex))
	{
		SayLine(*line);
	}

	ScheduleIdleLine();
}

void ACDScarecrow::SayLine(const FText& text)
{
	if (!CanSpeak())
	{
		return;
	}

	const int32 speechId = speechBubble->ShowSpeech(text);
	if (speechId == INDEX_NONE)
	{
		return;
	}

	// 이 대사가 아직 떠 있을 때만 내림 (그 사이 다른 대사로 바뀌었으면 컴포넌트가 무시)
	TWeakObjectPtr<UCDSpeechBubbleComponent> weakBubble = speechBubble.Get();
	GetWorldTimerManager().SetTimer(hideLineTimer, FTimerDelegate::CreateWeakLambda(this, [weakBubble, speechId]()
		{
			if (weakBubble.IsValid())
			{
				weakBubble->HideSpeech(speechId);
			}
		}), lineDuration, false);
}

bool ACDScarecrow::CanSpeak() const
{
	// 카메라 블렌드(EnterHub) 중에는 아직 타이틀 카메라가 뷰 타겟 → 블렌드가 끝나야 true
	const APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	const APawn* playerPawn = playerController ? playerController->GetPawn() : nullptr;
	return (playerPawn != nullptr) && (playerController->GetViewTarget() == playerPawn);
}

const FText* ACDScarecrow::PickLine(const TArray<FText>& lines, int32& lastIndex)
{
	if (lines.IsEmpty())
	{
		return nullptr;
	}

	int32 index = FMath::RandRange(0, lines.Num() - 1);

	// 같은 대사가 연속으로 나오지 않게 한 칸 밀기
	if ((lines.Num() > 1) && (index == lastIndex))
	{
		index = (index + 1) % lines.Num();
	}

	lastIndex = index;
	return &lines[index];
}

#undef LOCTEXT_NAMESPACE
