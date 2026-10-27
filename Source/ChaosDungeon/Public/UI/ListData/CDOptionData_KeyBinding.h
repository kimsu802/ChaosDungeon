#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "UI/ListData/CDOptionData.h"
#include "CDOptionData_KeyBinding.generated.h"

class APlayerController;

/**
 * 스킬 슬롯 키 항목. 읽기 / 쓰기는 UCDKeyBindingLibrary 그대로 사용 (다른 슬롯과 겹치면 맞바꿈).
 * 맞바꿈이 일어나면 다른 키 항목도 바뀌므로, 레지스트리가 onRemapped 로 같은 탭의 키 항목을 모두 갱신한다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDOptionData_KeyBinding : public UCDOptionData
{
	GENERATED_BODY()

public:
	/** 플레이어 · 슬롯 태그 연결 */
	void InitKeyBinding(APlayerController* inPlayerController, FGameplayTag inSlotTag);

	/** 현재 키 */
	FKey GetKey() const;

	/** 키 변경. 막힌 키(Esc, 마우스 좌/우, F1)거나 실패하면 false */
	bool SetKey(const FKey& newKey);

	/** 이 키를 쓸 수 있는가 */
	static bool IsKeyAllowed(const FKey& key);

	// UCDOptionData::ResetToDefault()
	virtual void ResetToDefault() override;

public:
	/** 키가 바뀐 뒤 호출 (레지스트리가 같은 탭의 키 항목 전체 갱신을 연결) */
	TFunction<void()> onRemapped;

private:
	/** 키를 바꿀 플레이어 */
	TWeakObjectPtr<APlayerController> playerController;

	/** 슬롯 입력 태그 (Input.Skill.1 등) */
	FGameplayTag slotTag;
};
