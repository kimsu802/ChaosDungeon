#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CDOptionEntryBase.generated.h"

class UCDOptionData;
class UCommonTextBlock;
class UImage;

/**
 * 옵션 리스트 한 줄의 공통 베이스. 왼쪽 이름 + 오른쪽 조작부(자식 클래스).
 * - 리스트 뷰가 엔트리를 재사용하므로, 아이템이 바뀔 때마다 이전 데이터의 구독을 풀고 새로 구독한다
 * - 값 표시는 데이터의 onValueChanged → RefreshValue 한 곳에서만 갱신
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDOptionEntryBase : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// IUserObjectListEntry::NativeOnListItemObjectSet()
	virtual void NativeOnListItemObjectSet(UObject* listItemObject) override;

	// IUserListEntry::NativeOnEntryReleased()
	virtual void NativeOnEntryReleased() override;

	// UUserWidget::NativeOnMouseEnter()
	virtual void NativeOnMouseEnter(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent) override;

	// UUserWidget::NativeOnMouseLeave()
	virtual void NativeOnMouseLeave(const FPointerEvent& inMouseEvent) override;

	/** 새 데이터가 연결됨 (범위 · 선택지 등 1회 설정) */
	virtual void OnOptionDataSet(UCDOptionData* optionData);

	/** 값 표시 갱신 */
	virtual void RefreshValue();

	/** 연결된 데이터 */
	UCDOptionData* GetOptionData() const
	{
		return boundData.Get();
	}

private:
	/** 이전 데이터 구독 해제 */
	void UnbindData();

	/** 호버 하이라이트 */
	void SetHighlighted(bool bHighlighted);

protected:
	/** 항목 이름 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> nameText;

	/** 호버 시 켜지는 줄 하이라이트 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> hoverHighlight;

private:
	/** 연결된 데이터 */
	TWeakObjectPtr<UCDOptionData> boundData;

	/** onValueChanged 구독 핸들 */
	FDelegateHandle valueChangedHandle;
};
