#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Core/CDMessageSubsystem.h"
#include "Core/CDTypes.h"
#include "CDFloatingTextSubsystem.generated.h"

class UCDFloatingTextWidget;
struct FCDDamageMessage;
struct FCDWorldTextMessage;

/**
 * 데미지 숫자 / 버프 텍스트 (위젯 풀링)
 * 초당 수십 개가 뜨는 고빈도 표시라 MVVM 대신 위젯을 직접 다룬다.
 * 구독: Combat.Damage, UI.FloatingText → 게임 코드는 이 클래스를 모른다.
 *
 * DefaultGame.ini
 *   [/Script/ChaosDungeon.CDFloatingTextSubsystem]
 *   widgetClass=/Game/UI/WBP_FloatingText.WBP_FloatingText_C
 */
UCLASS(Config = Game)
class CHAOSDUNGEON_API UCDFloatingTextSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// UWorldSubsystem::OnWorldBeginPlay()
	virtual void OnWorldBeginPlay(UWorld& inWorld) override;

	// USubsystem::Deinitialize()
	virtual void Deinitialize() override;

	/** 텍스트 1개 표시 */
	void Show(const FVector& worldLocation, const FText& text, ECDFloatingTextStyle style);

	/** 옵션: 데미지 숫자 표시 */
	FORCEINLINE void SetDamageNumbersEnabled(bool bEnabled)
	{
		bDamageNumbersEnabled = bEnabled;
	}

private:
	/** Combat.Damage → 데미지 숫자 */
	void HandleDamage(const FCDDamageMessage& message);

	/** UI.FloatingText → 버프 텍스트 */
	void HandleWorldText(const FCDWorldTextMessage& message);

	/** 쉬고 있는 위젯을 찾거나 새로 생성 */
	UCDFloatingTextWidget* Acquire();

private:
	/** 떠오르는 텍스트 위젯 클래스 (ini) */
	UPROPERTY(Config)
	TSoftClassPtr<UCDFloatingTextWidget> widgetClass;

	/** 위젯 풀 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCDFloatingTextWidget>> pool;

	/** 메시지 구독 핸들 */
	TArray<FCDListenerHandle> handles;

	/** 데미지 숫자 표시 여부 */
	bool bDamageNumbersEnabled = true;
};
