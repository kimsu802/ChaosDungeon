#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "CDSpeechBubbleComponent.generated.h"

class UCDSpeechBubbleWidget;

/**
 * 플레이어 머리 위 대사창. BP_Player 의 Mesh "head" 소켓에 붙인다.
 * - Screen Space WidgetComponent → 카메라 거리와 상관없이 같은 크기, 매 프레임 머리를 따라감
 * - 스킬 시전 대사: UCDPlayerSkillAbility 가 시전 확정 시 ShowSpeech, EndAbility 에서 HideSpeech
 * - ShowSpeech 가 돌려준 번호로만 내릴 수 있다 → 이전 스킬의 EndAbility 가 다음 스킬 대사를 내리지 않음
 */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class CHAOSDUNGEON_API UCDSpeechBubbleComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	/** Screen Space · Draw At Desired Size · 아래 가운데 피벗 */
	UCDSpeechBubbleComponent();

	/** 대사를 띄운다 (떠 있던 대사는 교체). 내릴 때 쓸 번호를 돌려준다 */
	UFUNCTION(BlueprintCallable, Category = "Speech")
	int32 ShowSpeech(const FText& text);

	/** 대사를 내린다. speechId 가 지금 떠 있는 대사의 번호일 때만 (INDEX_NONE = 무조건) */
	UFUNCTION(BlueprintCallable, Category = "Speech")
	void HideSpeech(int32 speechId = -1);

private:
	/** 위젯 (Widget Class = WBP_SpeechBubble) */
	UCDSpeechBubbleWidget* GetBubble() const;

private:
	/** 지금 떠 있는 대사 번호 (INDEX_NONE = 없음) */
	int32 currentSpeechId = INDEX_NONE;

	/** 마지막으로 발급한 번호 */
	int32 lastSpeechId = 0;
};
