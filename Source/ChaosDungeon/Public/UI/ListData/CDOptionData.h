// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CDOptionData.generated.h"

/**
 * 옵션 항목 1개의 데이터 (리스트 뷰의 아이템).
 * - 값은 직접 들고 있지 않고 getter / setter 로 실제 설정(UCDGameUserSettings, 키 매핑)을 읽고 쓴다
 * - 엔트리 위젯은 onValueChanged 를 구독해 표시만 갱신 (값이 바뀌는 곳이 엔트리 밖이어도 맞게 보임)
 * - 종류별 자식: Scalar(슬라이더) / Discrete(좌우 선택) / KeyBinding(키 입력)
 * - 생성 · 연결은 UCDOptionRegistry 가 담당
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDOptionData : public UObject
{
	GENERATED_BODY()

public:
	/** 표시 이름 · 설명 설정 (레지스트리가 생성 직후 호출) */
	void InitInfo(const FText& inDisplayName, const FText& inDescription);

	/** 표시 이름 */
	FORCEINLINE const FText& GetDisplayName() const
	{
		return displayName;
	}

	/** 설명 (옵션 화면 오른쪽 설명 칸) */
	FORCEINLINE const FText& GetDescription() const
	{
		return description;
	}

	/** 기본값으로 되돌리기 */
	virtual void ResetToDefault();

	/** 값이 바뀌었음을 알림 (다른 항목 때문에 바뀐 경우에도 호출: "전체 품질 → 개별 항목") */
	void NotifyValueChanged();

public:
	/** 값 변경 알림 (엔트리 갱신, "전체 품질 → 개별 항목" 연동용) */
	FSimpleMulticastDelegate onValueChanged;

protected:
	/** 표시 이름 */
	FText displayName;

	/** 설명 */
	FText description;
};
