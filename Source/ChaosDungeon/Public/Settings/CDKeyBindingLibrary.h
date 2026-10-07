#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "CDKeyBindingLibrary.generated.h"

class APlayerController;
class UCDInputConfig;
class UEnhancedInputUserSettings;

/**
 * 스킬 슬롯 키 매핑 (옵션 화면의 InputKeySelector 에서 호출)
 * - 바꾸는 것은 "키 → 입력 액션" 매핑뿐. 입력 액션 → 슬롯 태그 → 스킬 연결은 그대로라 게임 코드는 수정할 필요가 없다.
 * - 다른 슬롯이 이미 쓰는 키를 고르면 두 슬롯의 키를 맞바꾼다.
 * - 변경 즉시 적용 + 저장. 슬롯 VM 은 OnSettingsChanged 로 키 표시를 갱신한다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDKeyBindingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 슬롯의 현재 키 (매핑이 없으면 Invalid) */
	UFUNCTION(BlueprintPure, Category = "Input|KeyBinding")
	static FKey GetSlotKey(const APlayerController* playerController, UPARAM(meta = (Categories = "Input")) FGameplayTag slotTag);

	/** 슬롯의 키 변경. 성공하면 true */
	UFUNCTION(BlueprintCallable, Category = "Input|KeyBinding")
	static bool RemapSlotKey(const APlayerController* playerController, UPARAM(meta = (Categories = "Input")) FGameplayTag slotTag, FKey newKey);

	/** 매핑 이름의 첫 번째 키 슬롯에 현재 지정된 키 (없으면 Invalid). 슬롯 VM 의 키 표시에도 사용 */
	static FKey FindMappedKey(const UEnhancedInputUserSettings* userSettings, FName mappingName);

private:
	/** 플레이어의 Enhanced Input 사용자 설정 */
	static UEnhancedInputUserSettings* GetUserSettings(const APlayerController* playerController);

	/** 플레이어 컨트롤러의 입력 설정 */
	static const UCDInputConfig* GetInputConfig(const APlayerController* playerController);

	/** 매핑 이름의 첫 번째 키 슬롯을 key 로 지정 */
	static bool MapKey(UEnhancedInputUserSettings* userSettings, FName mappingName, const FKey& key);

	/** 변경 사항 적용 + 디스크 저장 */
	static void ApplyAndSave(UEnhancedInputUserSettings* userSettings);
};
