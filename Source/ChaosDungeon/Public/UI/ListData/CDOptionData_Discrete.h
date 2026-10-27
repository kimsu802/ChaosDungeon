#pragma once

#include "CoreMinimal.h"
#include "UI/ListData/CDOptionData.h"
#include "CDOptionData_Discrete.generated.h"

/**
 * 정해진 선택지 중 하나를 고르는 항목 (화면 모드, 품질, 수직 동기화 등). 엔트리는 "< 높음 >".
 * getter 가 선택지에 없는 값(INDEX_NONE)을 돌려주면 customText ("사용자 지정") 로 표시.
 */
UCLASS()
class CHAOSDUNGEON_API UCDOptionData_Discrete : public UCDOptionData
{
	GENERATED_BODY()

public:
	/** 선택지 · 기본값 · 실제 설정을 읽고 쓰는 함수 연결 */
	void InitDiscrete(TArray<FText>&& inOptions, TFunction<int32()>&& inGetter, TFunction<void(int32)>&& inSetter, int32 inDefaultIndex);

	/** 현재 선택 번호 (선택지에 없으면 INDEX_NONE) */
	int32 GetIndex() const;

	/** 선택 변경 */
	void SetIndex(int32 newIndex);

	/** 다음 선택지 (끝이면 처음으로) */
	void SelectNext();

	/** 이전 선택지 (처음이면 끝으로) */
	void SelectPrevious();

	/** 선택지 개수 */
	int32 GetOptionCount() const
	{
		return options.Num();
	}

	/** 표시용 글자 */
	FText GetValueText() const;

	/** 선택지에 없는 값일 때 표시할 글자 */
	void SetCustomText(const FText& inCustomText)
	{
		customText = inCustomText;
	}

	// UCDOptionData::ResetToDefault()
	virtual void ResetToDefault() override;

private:
	/** 선택지 */
	TArray<FText> options;

	/** 실제 설정 읽기 */
	TFunction<int32()> getter;

	/** 실제 설정 쓰기 */
	TFunction<void(int32)> setter;

	/** 기본 선택 번호 */
	int32 defaultIndex = 0;

	/** 선택지에 없는 값일 때 표시 */
	FText customText;
};
